/*
 * DL3762_AFN11.c
 *
 *  Created on: 2016年6月30日
 *      Author: j
 */
#include "DL3762_AFN11.h"
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
 * 函数功能:添加从节点
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	StandNode node;
	int index = 2;	//输入数据索引
	unsigned char num = 0;
	int ret = 0;
	unsigned char flag = 0;	//错误标志
	int fd;
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	unsigned short outIndex = 0;
	int i = 3;

	//得到待添加的从节点个数
	num = rvframe3762->Frame376_2App.AppData.Buffer[index++];

	if(rvframe3762->Frame376_2App.AppData.Len < (num * 7 + 3))
	{
		ret = -1;
		flag = 0;	//格式错误
		goto Afn0;
	}
	else
	{
		while(num--)
		{
			memset(&node, 0xFF, sizeof(StandNode));
			//电表地址
			memcpy(node.Amm, (rvframe3762->Frame376_2App.AppData.Buffer + index), AMM_ADDR_LEN);
			index += AMM_ADDR_LEN;
			//规约类型
			node.type = rvframe3762->Frame376_2App.AppData.Buffer[index++];
			//获取存储序号
			node.num = GetNearestSurplus();
			//抄表标志(_RunPara.CyFlag为抄表成功标志，当电表的抄表标志等于它时，抄表成功，每次重启抄表时，_RunPara.CyFlag - 1)
			//将抄表标志设置为与_Runpara.CyFlag不相等
			pthread_mutex_lock(&_RunPara.mutex);
			node.cyFlag = _RunPara.CyFlag + 1;
			pthread_mutex_unlock(&_RunPara.mutex);
			//添加/修改內存中的台账节点
			AddNodeStand(&node);
			//从内存中获取该节点写入文件中
			ret = SeekAmmAddr(node.Amm, AMM_ADDR_LEN);
			if(-1 != ret)
			{
//				printf("add node stand\n");
				GetStandNode(ret, &node);
				ret = AlterNodeStandFile(&node);
			}
		}

		pthread_mutex_lock(&_RunPara.mutex);
		if(0x01 != _RunPara.StandFlag)
		{
			_RunPara.StandFlag = 0x1;
			//打开文件
			while(i--)
			{
				fd = open(CONFIG_FILE, O_RDWR | O_CREAT, 0666);
				if(fd >=0 )
					break;
			}

			if(fd < 0)
			{
				pthread_mutex_unlock(&_RunPara.mutex);
				goto Afn0;
			}

			tpIsStand stand;
			int len;
			len = offsetof(tpConfiguration, StandFlag);
			memcpy(&stand.flag, &_RunPara.StandFlag, 1);
			len = offsetof(tpIsStand, CS);
			stand.CS = Func_CS((void*)&stand, len);
			len = offsetof(tpConfiguration, StandFlag);
			WriteFile(fd, len, (void*)&stand, sizeof(tpCyFlag));
			close(fd);
		}
		pthread_mutex_unlock(&_RunPara.mutex);
	}

Afn0:		//应答
	if(0 > ret)
	{
		Fn = 2;		//否认
		FNtoDT(Fn, DT);

		//数据标识
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//错误状态字
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = flag;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
	else
	{
		Fn = 1;		//确认
		FNtoDT(Fn, DT);

		//数据标识
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//数据内容
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}

	return;
}

/*
 * 函数功能:删除从节点
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	AmmAttribute Amm;
	unsigned short inIndex = 2;	//输入数据索引
	unsigned short outIndex = 0;	//输出数据索引
	unsigned char num = 0;	//删除的数量
	int	ret = -1;
	unsigned char flag = 0;
	int i = 3;
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	//获得待删除的数量
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//判断删除帧的长度是否真确	数据标识+从节点数量+每个从节点的数量
	if(rvframe3762->Frame376_2App.AppData.Len < (2 + 1 + AMM_ADDR_LEN * num))
	{
		ret = -1;
		flag = 0;
	}
	else
	{
		for(i = 0; i < num; i++)
		{
			memset(&Amm, 0, sizeof(AmmAttribute));
			memcpy(Amm.Amm, (rvframe3762->Frame376_2App.AppData.Buffer+inIndex), AMM_ADDR_LEN);
			inIndex += AMM_ADDR_LEN;
			DeleNodeStand(Amm.Amm);
		}
		ret = 0;
	}

	//返回帧
	if(0 > ret)
	{
		Fn = 2;		//否认
		FNtoDT(Fn, DT);

		//数据标识
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//错误状态字
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = flag;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
	else
	{
		Fn = 1;		//确认
		FNtoDT(Fn, DT);

		//数据标识
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//数据内容
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
}

/*
 * 函数功能:设置从节点固定中继路径
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * 函数功能:设置路由工作模式
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_04(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * 函数功能:激活从节点主动注册
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_05(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * 函数功能:终止从节点主动注册
 * 参数	rvframe3762	待执行的376.2
 * 		snframe3762	待返回的376.2
 * 返回值:无
 * */
void AFN11_06(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * 函数功能:不支持的AFN的响应
 * */
void AFN11_Other(tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 0;	//输出数据索引
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};

	Fn = 1;		//确认
	FNtoDT(Fn, DT);

	//数据标识
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

	//数据内容
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:路由设置
 * 参数: rvframe3762 待解析的376.2数据
 * 		snframe3762	解析后填充的发送376.2
 * 返回值:无
 * */
void DL3762_AFN11_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
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
	snframe3762->Frame376_2App.AppData.AFN = AFN_AFFI;	//确认帧
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}

	temp = snframe3762->Frame376_2App.AppData.Len;
	printf("AFN11  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN11_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN11_02(rvframe3762, snframe3762);
			break;
//		case 3:
//			AFN11_03(rvframe3762, snframe3762);
//			break;
//		case 4:
//			AFN11_04(rvframe3762, snframe3762);
//			break;
//		case 5:
//			AFN11_05(rvframe3762, snframe3762);
//			break;
//		case 6:
//			AFN11_06(rvframe3762, snframe3762);
//			break;
		default :
			AFN11_Other(snframe3762);
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}

