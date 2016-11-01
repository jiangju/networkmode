/*
 * DL3761_AFN17.c
 *
 *  Created on: 2016年10月21日
 *      Author: j
 */
#include "DL376_1.h"
#include "DL3761_AFN17.h"
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

/*
 * 函数功能:搜表结果与台账匹配
 * 参数:		amm		电表
 * 			ter		终端
 * 返回值 0  -1
 * */
int seek_amm_synchroniza(unsigned char *amm, unsigned char *ter)
{
	if(NULL == amm || NULL == ter)
		return -1;
	StandNode node;
	int i = 3;
	int fd, ret;
	//打开文件
	while(i--)
	{
		fd = open(STAND_BOOK_FILE, O_RDWR);
		if(fd >=0 )
			break;
	}

	if(fd < 0)
		return -1;

	if(0x01 == _RunPara.StandFlag)
	{
		//查看是否有相同的电表
		ret = SeekAmmAddr(node.Amm, AMM_ADDR_LEN);
		if(ret >= 0)
		{
			GetStandNode(ret, &node);
			memcpy(node.Ter, ter, TER_ADDR_LEN);
			AddNodeStand(&node);
			AlterNodeStandFile(fd, &node);
		}
	}
	else
	{
		memset(&node, 0xFF, sizeof(StandNode));
		//电表地址
		memcpy(node.Amm, (amm), AMM_ADDR_LEN);
		//终端地址
		memcpy(node.Ter, ter, TER_ADDR_LEN);
		//规约类型
		node.type = 0x02;
		//获取存储序号
		node.num = GetNearestSurplus();
		//抄表标志(_RunPara.CyFlag为抄表成功标志，当电表的抄表标志等于它时，抄表成功，每次重启抄表时，_RunPara.CyFlag - 1)
		//将抄表标志设置为与_Runpara.CyFlag不相等
		node.cyFlag = _RunPara.CyFlag + 1;
		//添加/修改內存中的台账节点
		AddNodeStand(&node);
		AlterNodeStandFile(fd, &node);
	}
	close(fd);
	return 0;
}

/*
 * 函数功能:搜表任务主动上报处理
 * */
void DL3761_AFN17_01(tpFrame376_1 *rvframe3761)
{
	unsigned short in_index = 4;
	unsigned char num = 0;
	int i = 0;
	struct seek_amm_result *result;

	result = (struct seek_amm_result *)malloc(sizeof(struct seek_amm_result));
	if(NULL == result)
		return;
	memset(result, 0, sizeof(struct seek_amm_result));

	result->ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
	result->ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
	result->ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
	result->ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

	//更新主动台账
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];	//获取个数
	result->num = num;
	result->index = initiative_stand_min_not_using_index();
	if(result->index < 0)
		return;

	for(i = 0; i < num; i++)
	{
		memcpy((unsigned char *)(result->amm + i), (rvframe3761->Frame376_1App.AppBuf + in_index), AMM_ADDR_LEN);
		in_index += AMM_ADDR_LEN;
		//没有下发被动台账时，将内容同步到被动台账，有下发被动台账时，根据搜表结果更新被动台账路径
		seek_amm_synchroniza((unsigned char *)(result->amm + i), result->ter);
	}

	if(0 == add_seek_amm_result(result))
	{
		write_seek_amm_result(result);
	}
}

/*
 * 函数功能:扩展
 * */
void DL3761_AFN17_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
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

	//功能码
	snframe3761->Frame376_1App.AFN = AFN3761_EXTEND16;
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
	snframe3761->Frame376_1App.AppBuf[0] = rvframe3761->Frame376_1App.AppBuf[0];
	snframe3761->Frame376_1App.AppBuf[1] = rvframe3761->Frame376_1App.AppBuf[1];
	snframe3761->Frame376_1App.AppBuf[2] = rvframe3761->Frame376_1App.AppBuf[2];
	snframe3761->Frame376_1App.AppBuf[3] = rvframe3761->Frame376_1App.AppBuf[3];

	//解析接受帧
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);

	switch(Fn)
	{
		case 1:
			DL3761_AFN17_01(rvframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
