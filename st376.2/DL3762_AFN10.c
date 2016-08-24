/*
 * DL3762_AFN10.c
 *
 *  Created on: 2016年7月4日
 *      Author: j
 */
#include "DL3762_AFN10.h"
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "SysPara.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "StandBook.h"

/*
 * 函数功能:查询从节点数量
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	unsigned char temp = 0;

	//从节点数量
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//最大支持从节点数量
	temp = AMM_MAX_NUM % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = AMM_MAX_NUM / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:查询从节点信息
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char temp = 0;
	unsigned short serialNum = 0;
	unsigned char count0 = 0;
	unsigned char count1 = 0;
	unsigned short inIndex = 2;
	unsigned short outIndex = 2;
	int	i = 0;

	//判断接受数据的长度是否合理	数据标识+从节点起始序号+从节点数量
	if(rvframe3762->Frame376_2App.AppData.Len < (2 + 2 + 1))
	{
//		printf("AFN10 len 70 erro\n");
		return;
	}

	//获取起始序号
	temp = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	serialNum = temp + 256 * rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	if(0 == serialNum)
	{
		serialNum = 1;
	}

	//获取数量
	count0 = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//填充发送内容
	//从节点总数量
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//本次应答总数量	集中器下发的起始序号1为最小
	if(_StandNodeNum < serialNum)
	{
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++]  = 0;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
		return;
	}

	if((_StandNodeNum - serialNum + 1) < count0)
	{
		count1 = _StandNodeNum - serialNum + 1;
	}
	else
	{
		count1 = count0;
	}
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = count1;

	//从节点信息
	for(i = 0; i < count1; i++)
	{
		//从节点地址
		memcpy((snframe3762->Frame376_2App.AppData.Buffer + outIndex),
				_SortNode[serialNum - 1 + i]->Amm, AMM_ADDR_LEN);
		outIndex += AMM_ADDR_LEN;

		//从节点信息
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		temp = (_SortNode[serialNum - 1 + i]->type << 3)  & 0x38;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp | 0x4;
	}

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:指定从节点的上一级中继路由信息
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;

	//提供路由的从节点总数量n
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:路由运行状态
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_04(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	int	i = 0;
	unsigned char temp = 0;
	unsigned short num = 0;
	//运行状态字(路由学习完成)
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x01;

	//从节点数量
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//已抄收节点数量
	for(i = 0; i < _StandNodeNum; i++)
	{
		if(_SortNode[i]->cyFlag == _RunPara.CyFlag)
		{
			num++;
		}
	}
	temp = num % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = num / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//中继吵到从节点数量
	temp = num % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = num / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//工作开关
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x01;

	//通信速率
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x02;

	//第一项中继级别
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//第二项中继级别
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//第三项中继级别
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//第一项工作步骤
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;
	//第二项工作步骤
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;
	//第三项工作步骤
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:未抄读成功的从节点信息
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_05(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short sernum = 0;	//起始序号
	unsigned char  num = 0;		//数量
	unsigned short outIndex = 2;
	unsigned short tempout = 0;
	unsigned short inIndex = 2;
	unsigned short temp = 0;
	unsigned short tempnum = 0;
	int i = 0;

	//获得起始序号
	sernum = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	sernum = sernum + 256 * rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//获取数量
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	if(sernum < 0 || num <= 0)
		return;
	if(sernum == 0)
		sernum = 1;

	//从节点数量
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//本次应答数量
	outIndex++;
	tempout = outIndex;	//备份本次应答数量索引

	for(i = 0; i < num && ((sernum + i) < _StandNodeNum); i++)
	{
		//抄表不成功
		if(_SortNode[sernum - 1 + i]->cyFlag != _RunPara.CyFlag)
		{
			tempnum++;
			//从节点地址
			memcpy(snframe3762->Frame376_2App.AppData.Buffer + outIndex, _SortNode[sernum - 1 + i], AMM_ADDR_LEN);
			outIndex += outIndex;
			//从节点信息
			snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
			temp = _SortNode[sernum - 1 + i]->type;
			temp &= 0x7;
			temp = temp << 3;
			temp = temp || 0x01;
			snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
		}
	}

	snframe3762->Frame376_2App.AppData.Buffer[tempout] = tempnum;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:主动注册的从节点信息
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN10_06(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	int	i = 0;
	unsigned char temp = 0;

	//从节点数量
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//本次应答的从节点数量
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:路由查询
 * 参数: rvframe3762 待解析的376.2数据
 * 		snframe3762	解析后填充的发送376.2
 * 返回值:无
 * */
void DL3762_AFN10_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;
	unsigned short temp = 0;
	if(NULL == snframe3762)
	{
		return;
	}

	memset(snframe3762, 0, sizeof(tpFrame376_2));

	/**************************************链路层***************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	if(0 == rvframe3762->Frame376_2Link.CtlField.PRM ||
			1 == rvframe3762->Frame376_2Link.CtlField.DIR)
	{
		return;
	}
	snframe3762->Frame376_2Link.CtlField.PRM = 0;
	snframe3762->Frame376_2Link.CtlField.DIR = 1;

	snframe3762->Frame376_2Link.CtlField.CommMode = rvframe3762->Frame376_2Link.CtlField.CommMode;
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;

	/****************************************应用层******************************************/
	//无地址域
	memset(&snframe3762->Frame376_2App.R, 0, sizeof(tpR));
	snframe3762->Frame376_2App.R.MessageSerialNumber = rvframe3762->Frame376_2App.R.MessageSerialNumber;
	snframe3762->Frame376_2Link.Len += R_LEN;
	snframe3762->Frame376_2App.AppData.AFN = AFN_ROUTEQUERY;	//
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	snframe3762->Frame376_2App.AppData.Buffer[0] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[1] = DT[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}
	snframe3762->Frame376_2App.AppData.Len = 2;
	temp = snframe3762->Frame376_2App.AppData.Len;
//	printf("AFN10  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN10_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN10_02(rvframe3762, snframe3762);
			break;
		case 3:
			AFN10_03(rvframe3762, snframe3762);
			break;
		case 4:
			AFN10_04(rvframe3762, snframe3762);
			break;
		case 5:
			AFN10_05(rvframe3762, snframe3762);
			break;
		case 6:
			AFN10_06(rvframe3762, snframe3762);
			break;
		default :
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}

