/*
 * DL3762_AFN06.c
 *
 *  Created on: 2016年7月25日
 *      Author: j
 */
#include "DL3762_AFN06.h"
#include "StandBook.h"
#include <string.h>
#include <stdio.h>
/*
 * 函数功能:构造上报抄读数据报文
 * 参数:	index	电表在台账中的序号
 * 		type	通信类型
 * 		inbuf	输入缓存
 * 		len		数据长度
 * 		snframe3762 输出数据
 * */
void Create3762AFN06_02(unsigned short index,unsigned char type,
		unsigned char *inbuf, unsigned char len, tpFrame376_2 *snframe3762)
{
	unsigned short outindex = 0;
	memset(snframe3762, 0, sizeof(tpFrame376_2));
	StandNode node;
	/************************************链路层*************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	snframe3762->Frame376_2Link.CtlField.PRM = 0;	//从动站

	snframe3762->Frame376_2Link.CtlField.DIR = 1;	//上行
	snframe3762->Frame376_2Link.CtlField.CommMode = 1;//通信1
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;
	/*************************************应用层*************************************/
	//信息域
	snframe3762->Frame376_2App.R.PredictAnswerLen = 0x11;
	snframe3762->Frame376_2App.R.CommModeIdentifying = 0x01;	//通信模块表示
	snframe3762->Frame376_2Link.Len += R_LEN;
	//地址域
	GetStandNode(index, &node);
	memcpy(snframe3762->Frame376_2App.Addr.SourceAddr, node.Amm, AMM_ADDR_LEN);	//源地址
	memcpy(snframe3762->Frame376_2App.Addr.DestinationAddr, _RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);//目的地址
	snframe3762->Frame376_2Link.Len += (6 + 6);
	//功能码
	snframe3762->Frame376_2App.AppData.AFN = AFN_ACCORD;	//上报
	snframe3762->Frame376_2Link.Len += AFN_LEN;
	//数据标识
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x00;
	//从节点序号
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = (unsigned char)((index+1) % 256);
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = (unsigned char)((index+1) / 256);
	//通信协议类型
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = type;
	//当前报文通信上行时长
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x01;
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x00;
	//报文长度
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = len;
	//报文内容
	memcpy((snframe3762->Frame376_2App.AppData.Buffer + outindex), inbuf, len);
	outindex += len;
	snframe3762->Frame376_2App.AppData.Len += outindex;
	snframe3762->Frame376_2Link.Len += outindex;
	snframe3762->IsHaving = true;
}

