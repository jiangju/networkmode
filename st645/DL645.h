/*
 * DL645.h
 *
 *  Created on: 2016年7月24日
 *      Author: j
 */

#ifndef ST645_DL645_H_
#define ST645_DL645_H_
#include "SysPara.h"
#define DL645_DATALENGTH	255		//645最大长度

typedef struct
{
	unsigned char Address[AMM_ADDR_LEN];	//地址域
	unsigned char CtlField;					//控制域
	unsigned char Length;					//数据域长度
	unsigned char Datas[DL645_DATALENGTH];	//数据域BUF
}tpFrame645;
int ProtoAnaly645BufFromCycBuf(unsigned char *buf, int len, tpFrame645 * buf645);
#endif /* ST645_DL645_H_ */
