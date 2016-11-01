/*
 * DL645.h
 *
 *  Created on: 2016��7��24��
 *      Author: j
 */

#ifndef ST645_DL645_H_
#define ST645_DL645_H_
#include "SysPara.h"
#define DL645_DATALENGTH	255		//645��󳤶�

typedef struct
{
	unsigned char Address[AMM_ADDR_LEN];	//��ַ��
	unsigned char CtlField;					//������
	unsigned char Length;					//�����򳤶�
	unsigned char Datas[DL645_DATALENGTH];	//������BUF
}tpFrame645;

typedef struct
{
	unsigned char buf[DL645_DATALENGTH];
	unsigned char len;
}Buff645;

int ProtoAnaly645BufFromCycBuf(unsigned char *buf, int len, tpFrame645 * buf645);
int Create645From(tpFrame645 * buf645, Buff645 *outbuf);
#endif /* ST645_DL645_H_ */
