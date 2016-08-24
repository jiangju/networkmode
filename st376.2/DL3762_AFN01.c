/*
 * DL3762_AFN01.c
 *
 *  Created on: 2016年7月26日
 *      Author: j
 */
#include "DL3762_AFN01.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "StandBook.h"
#include "string.h"
#include "stdlib.h"
#include "CommLib.h"

//硬件复位
void AFN01_01(tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	unsigned short outIndex = 0;
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

	_RebootUsart0 = 0x66;
}

//参数初始化
void AFN01_02(tpFrame376_2 *snframe3762)
{
	int i = 3;
	int fd;
	int ret = 0;
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	unsigned short outIndex = 0;
	while(i--)
	{
		fd = open(STAND_BOOK_FILE, O_RDWR);
		if(fd >= 0)
			break;
	}
	if(fd < 0)
	{
		ret = -1;
	}
	else
	{
		AmmAttribute amm;
		memset(&amm, 0xFF, sizeof(AmmAttribute));
		for(i = 0; i < AMM_MAX_NUM; i++)
		{
			write(fd, &amm, sizeof(AmmAttribute));
		}
		close(fd);

		memset(_Surplus, 0, AMM_MAX_NUM);

		for(i = 0; i < _StandNodeNum; i++)
		{
			if(_SortNode[i] != NULL)
				free(_SortNode[i]);
			_SortNode[i] = NULL;
		}
		_StandNodeNum = 0;
		ret = 0;
	}

	if(ret == -1)
	{
		Fn = 2;		//否认
		FNtoDT(Fn, DT);

		//数据标识
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//错误状态字
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xEE;
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

//数据区初始化
void AFN01_03(tpFrame376_2 *snframe3762)
{
	AFN01_02(snframe3762);
}

//初始化
void DL3762_AFN01_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;
	unsigned short temp = 0;
	if(NULL == snframe3762)
	{
		return;
	}

	memset(snframe3762, 0, sizeof(tpFrame376_2));

	/**********************************先填充链路层*****************************************/
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

	/**************************************应用层************************************************/
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
//	printf("AFN1  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN01_01(snframe3762);
			break;
		case 2:
			AFN01_02(snframe3762);
			break;
		case 3:
			AFN01_03(snframe3762);
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
