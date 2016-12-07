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
//#include <time.h>
#include <sys/time.h>
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
	StandNode node;
	GetStandNode(_Collect.taskc.index, &node);
	//判断抄读标志
	switch (rvframe3762->Frame376_2App.AppData.Buffer[index])
	{
		case 0x00:
			if(1 == CompareUcharArray(node.Amm, rvframe3762->Frame376_2App.Addr.DestinationAddr, AMM_ADDR_LEN))
			{
			//	printf("****************************************************************************\n");
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 1;
				_Collect.taskc.count = 0;
				_Collect.taskc.index++;
				_Collect.taskc.index %= GetStandNodeNum();
			}
			break;
		case 0x01:
			if(1 == CompareUcharArray(node.Amm, rvframe3762->Frame376_2App.Addr.DestinationAddr, AMM_ADDR_LEN))
			{
				//更新抄表成功标志
				pthread_mutex_lock(&_RunPara.mutex);
				node.cyFlag = _RunPara.CyFlag;
				pthread_mutex_unlock(&_RunPara.mutex);
				UpdateStandNode(_Collect.taskc.index, &node);
				AlterNodeStandFile(&node);
//				printf("index ok %d\n",_Collect.taskc.index);
				pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 0;
				_Collect.taskc.count = 0;
				_Collect.taskc.index++;
				_Collect.taskc.index %= GetStandNodeNum();
				pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);

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
				if(1 == CompareUcharArray(node.Amm, buf.Address, AMM_ADDR_LEN))
				{
					pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
					_Collect.taskc.isok = 1;
					_Collect.taskc.timer = 0;
					_Collect.taskc.count = 0;
					_Collect.taskc.len = len;
					memcpy(_Collect.taskc.buf, rvframe3762->Frame376_2App.AppData.Buffer + index, len);
					memcpy(_Collect.taskc.dadt, buf.Datas, 4);
					pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
				}
			}
			break;
	}
}

/*
 * 函数功能:路由请求集中器时钟 响应
 * */
void AFN14_02(tpFrame376_2 *rvframe3762)
{
//	unsigned char new_time[6] = {0};	//秒	分 时 日 月 年

	unsigned short index = 2;
	struct tm serTime; //接收时间结构体
	struct timeval tv;
	time_t timep;
	//秒
	serTime.tm_sec = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10);
	index += 1;
	printf(" %02d",serTime.tm_sec);

	//分
	serTime.tm_min = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10);
	index += 1;
	printf(": %02d",serTime.tm_min);

	//时
	serTime.tm_hour = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10);
	index += 1;
	printf(": %02d",serTime.tm_hour);

	//日
	serTime.tm_mday = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10);
	index += 1;
	printf(" %02d",serTime.tm_mday);

	//月
	serTime.tm_mon = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10) - 1;
	index += 1;
	printf("-%02d",serTime.tm_mon);

	//年
	serTime.tm_year = (rvframe3762->Frame376_2App.AppData.Buffer[index] & 0x0f) \
			+ (((rvframe3762->Frame376_2App.AppData.Buffer[index] & 0xf0) >> 4) * 10) + 2000 - 1900;
	index += 1;
	printf("-%02d\n",serTime.tm_year);

	timep = mktime(&serTime);

	tv.tv_sec = timep;
	tv.tv_usec = 0;

	if(settimeofday (&tv, (struct timezone *) 0) < 0)
	{
		printf("Set system datatime error!/n");
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
			AFN14_02(rvframe3762);
			break;
		case 3:
			break;
		default :
			break;
	}
}
