/*
 * DL3761_AFN5.c
 *
 *  Created on: 2016年10月24日
 *      Author: j
 */

#include "DL376_1.h"
#include "DL3761_AFN05.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CommLib.h"
#include <stdlib.h>
#include <stddef.h>
#include "SysPara.h"
#include "Route.h"
#include "SeekAmm.h"
#include "StandBook.h"
#include <sys/time.h>
#include "Collect.h"

/*
 * 函数功能:充值终端拔卡
 * */
void DL3761_AFN05_97(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int in_index = 4;
	int out_index = 0;
	unsigned char amm[AMM_ADDR_LEN] = {0};

	//获取表地址
	memcpy(amm, (rvframe3761->Frame376_1App.AppBuf + in_index), AMM_ADDR_LEN);

	if(0 == DeleTaskA(amm, 1))
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
	}
	else
	{
		printf("amm: %02x %02x %02x %02x %02x %02x\n",amm[0],amm[1],amm[2],amm[3],amm[4],amm[5]);
		//数据标识
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}

void DL3761_AFN05_99(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int in_index = 4;
	int out_index = 0;
	int res = 0;
	int i = 0;
	unsigned char ter[TER_ADDR_LEN] = {0};
	//获取终端个数
	unsigned short num = 0;
	num = rvframe3761->Frame376_1App.AppBuf[in_index + 1] * 256 +rvframe3761->Frame376_1App.AppBuf[in_index];
	in_index += 2;
	if(num <= 0)
	{
		res = -1;
	}
	else
	{
		struct seek_amm_task *task;
		struct seek_amm_task temp;
		for(i = 0; i < num; i++)
		{
			memcpy(ter, rvframe3761->Frame376_1App.AppBuf + in_index, TER_ADDR_LEN);
			in_index += TER_ADDR_LEN;
			if(-1 != find_seek_amm_task(ter, &temp))
				continue;
			task = (struct seek_amm_task *)malloc(sizeof(struct seek_amm_task));
			if(NULL == task)
				continue;
			task->next = NULL;
			task->flag = 0xFF;
			task->ticker_ticker = 10;
			memcpy(task->ter, ter, TER_ADDR_LEN);
			in_index += TER_ADDR_LEN;
			add_seek_amm_task(task);
		}
		res = 0;
	}

	if(res != 0)
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
	}
	else
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0x00;
	}
	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}

/*..
 * 函数功能:控制命令
 * */
void DL3761_AFN05_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	if(NULL == rvframe3761 || NULL == snframe3761)
		return;
	/***********************************填充链路层**************************/
	//起始符
	snframe3761->Frame376_1Link.BeginChar0 = 0x68;
	snframe3761->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	snframe3761->Frame376_1Link.EndChar = 0x16;
	//控制域
	snframe3761->Frame376_1Link.CtlField.DIR = 1;	//上行
	snframe3761->Frame376_1Link.CtlField.PRM = 0;	//从动站
	snframe3761->Frame376_1Link.CtlField.FCV = 0;	//FCB位无效
	snframe3761->Frame376_1Link.CtlField.FCB = 0;
	snframe3761->Frame376_1Link.CtlField.FUNC_CODE = 0;	//功能码
	//地址域
	memcpy(snframe3761->Frame376_1Link.AddrField.WardCode,\
			rvframe3761->Frame376_1Link.AddrField.WardCode, 2);
	memcpy(snframe3761->Frame376_1Link.AddrField.Addr, \
			rvframe3761->Frame376_1Link.AddrField.Addr, 2);
	snframe3761->Frame376_1Link.AddrField.MSA = 0;

	/*************************************应用层********************************/
	//功能码
	snframe3761->Frame376_1App.AFN = AFN3761_AFFI;
	//请求确认标志
	snframe3761->Frame376_1App.SEQ.CON = 0;	//不需要确认
	//帧类型
	snframe3761->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//帧序号
	snframe3761->Frame376_1App.SEQ.PSEQ_RSEQ = rvframe3761->Frame376_1App.SEQ.PSEQ_RSEQ;
	//时间标签
	snframe3761->Frame376_1App.SEQ.TPV = 0;	//不要时标
	//附加域 时标
	snframe3761->Frame376_1App.AUX.AUXTP.flag = 0;	//无效
	snframe3761->Frame376_1App.AUX.AUXEC.flag = 0;

	//解析接受帧
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);
//	printf("AFN 05  %d\n",Fn);
	switch(Fn)
	{
		case 99:
			DL3761_AFN05_99(rvframe3761, snframe3761);
			break;
		case 97:
			DL3761_AFN05_97(rvframe3761, snframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
