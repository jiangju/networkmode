/*
 * DL3761_AFN0E.c
 *
 *  Created on: 2016年8月16日
 *      Author: j
 */

#include "DL376_1_DataType.h"
#include "DL376_1.h"
#include "Response_AFN0E.h"
#include <stdio.h>
#include <string.h>

/*
 * 函数功能:响应事件上报
 * */
void ResponseAFN0E_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
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
