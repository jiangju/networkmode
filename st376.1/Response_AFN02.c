/*
 * Response.c
 *
 *  Created on: 2016年7月20日
 *      Author: j
 */
#include "DL376_1_DataType.h"
#include "DL376_1.h"
#include "Response_AFN02.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SeekAmm.h"
#include "StandBook.h"
#include "TopRoute.h"
/*
 * 函数功能:响应登录 心跳
 * 参数:	rvframe3761 待解析的376.1数据
 * 		snframe3761	解析后填充的发送376.1
 * 返回值:无
 * */
void ResponseAFN02_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	/**********************************登录后检测是否需要搜表***************************/
	unsigned char ter[4] = {0};
	struct seek_amm_task task;
	if(rvframe3761->Frame376_1App.AppBuf[2] == 0x01 && rvframe3761->Frame376_1App.AppBuf[3] == 0x00)
	{
		ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
		ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
		ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
		ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

		if(0 != check_hld_top_node_ter(ter))	//非充值终端
		{
			if(0 == StatTerAmmNum(ter))	//终端下有无电表
			{
				if(-1 == find_seek_amm_task(ter, &task))	//搜表队列中无该终端
				{
					struct seek_amm_task *temp = (struct seek_amm_task *)malloc(sizeof(struct seek_amm_task));
					if(temp != NULL)
					{
						temp->flag = 0xFF;
						temp->next = NULL;
						memcpy(temp->ter, ter, 4);
						temp->ticker_ticker = 3 * 60; //10 S
						add_seek_amm_task(temp);
					}
				}
			}
		}
	}

	/************************************************************************/
	if(NULL == snframe3761)
			return;
	int index = 0;
	/**********************************填充链路层*******************************/
	//起始符
	snframe3761->Frame376_1Link.BeginChar0 = 0x68;
	snframe3761->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	snframe3761->Frame376_1Link.EndChar = 0x16;
	//控制域
	snframe3761->Frame376_1Link.CtlField.DIR = 0;	//下行
	snframe3761->Frame376_1Link.CtlField.PRM = 0;	//从动站
	snframe3761->Frame376_1Link.CtlField.FCV = 0;	//FCB位无效
	snframe3761->Frame376_1Link.CtlField.FCB = 0;
	//snframe3761->Frame376_1Link.CtlField.ACD  = 0;
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

	//数据标识
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x01;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}

