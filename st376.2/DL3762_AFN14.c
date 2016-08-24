/*
 * DL3762_AFN14.c
 *
 *  Created on: 2016年7月22日
 *      Author: j
 */
#include "DL3762_AFN14.h"
#include <string.h>
#include <stdio.h>
#include "StandBook.h"
#include "Collect.h"
#include "DL645.h"
#include "CommLib.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "SysPara.h"
#include "stddef.h"
/*
 * 函数功能:构造路由请求抄读内容
 * 参数:	amm		电表号
 * 		type	规约类型
 * 		snfrane376.2	构造后的数据
 * 返回值 wu
 * */
void CreatAFN14_01up(unsigned char *amm, tpFrame376_2 *snframe3762)
{
	unsigned short index = 0;
	int ret = 0;
	memset(snframe3762, 0, sizeof(tpFrame376_2));
	/**************************************链路层***************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;

	snframe3762->Frame376_2Link.CtlField.PRM = 1;
	snframe3762->Frame376_2Link.CtlField.DIR = 1;

	snframe3762->Frame376_2Link.CtlField.CommMode = 1;
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;

	/****************************************应用层******************************************/
	//无地址域
	memset(&snframe3762->Frame376_2App.R, 0, sizeof(tpR));
	snframe3762->Frame376_2Link.Len += R_LEN;
	snframe3762->Frame376_2App.AppData.AFN = AFN_ROUTERCV;	//AFN14
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x01;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;

	//通信相位
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x01;

	//从节点地址
	memcpy(snframe3762->Frame376_2App.AppData.Buffer + index, amm, AMM_ADDR_LEN);
	index += AMM_ADDR_LEN;

	//从节点序号
	ret = SeekAmmAddr(amm, AMM_ADDR_LEN);
	ret += 1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = (unsigned char)((unsigned short)ret & 0x00FF);
	snframe3762->Frame376_2App.AppData.Buffer[index++] = (unsigned char)((unsigned short)ret >> 8);

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
	snframe3762->IsHaving = true;
}

/*
 * 函数功能:
 * */
void AFN14_01(tpFrame376_2 *rvframe3762)
{
	unsigned short index = 2;
	int len = 0;
	tpFrame645 buf;
	int fd;
	int i = 0;
	//判断抄读标志
	switch (rvframe3762->Frame376_2App.AppData.Buffer[index])
	{
		case 0x00:
			if(1 == CompareUcharArray(_SortNode[_Collect.taskc.index]->Amm, rvframe3762->Frame376_2App.Addr.DestinationAddr, AMM_ADDR_LEN))
			{
			//	printf("****************************************************************************\n");
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 1;
				_Collect.taskc.count = 0;
				_Collect.taskc.index++;
				_Collect.taskc.index %= _StandNodeNum;
			}
			break;
		case 0x01:
			if(1 == CompareUcharArray(_SortNode[_Collect.taskc.index]->Amm, rvframe3762->Frame376_2App.Addr.DestinationAddr, AMM_ADDR_LEN))
			{
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 0;
				_Collect.taskc.count = 0;
				//更新抄表成功标志
				_SortNode[_Collect.taskc.index]->cyFlag = _RunPara.CyFlag;
				while(i--)
				{
					fd = open(STAND_BOOK_FILE, O_RDWR, 0666);
					if(fd >= 0)
						break;
				}
				if(fd >= 0)
					AlterNodeStandFile(fd, _SortNode[_Collect.taskc.index]);
//				printf("index ok %d\n",_Collect.taskc.index);
				_Collect.taskc.index++;
				_Collect.taskc.index %= _StandNodeNum;
			}
			break;
		case 0x02:
			//获取数据长度
			index += 2;
			len = rvframe3762->Frame376_2App.AppData.Buffer[index];
			index++;
			//获取表地址
			if(0 == ProtoAnaly645BufFromCycBuf((rvframe3762->Frame376_2App.AppData.Buffer + index), len, &buf))
			{
				if(1 == CompareUcharArray(_SortNode[_Collect.taskc.index]->Amm, buf.Address, AMM_ADDR_LEN))
				{
					_Collect.taskc.isok = 1;
					_Collect.taskc.timer = 0;
					_Collect.taskc.count = 0;
					_Collect.taskc.len = len;
					memcpy(_Collect.taskc.buf, rvframe3762->Frame376_2App.AppData.Buffer + index, len);
					memcpy(_Collect.taskc.dadt, buf.Datas, 4);
				}
			}
			break;
	}
}

void DL3762_AFN14_Analy(tpFrame376_2 *rvframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}

	printf("AFN14  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN14_01(rvframe3762);
			break;
		case 2:
			break;
		case 3:
			break;
		default :
			break;
	}
}
