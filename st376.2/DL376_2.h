/*
 * DL376_2.h
 *
 *  Created on: 2016��6��21��
 *      Author: Administrator
 */

#ifndef ST376_2_DL376_2_H_
#define ST376_2_DL376_2_H_

#include "DL376_2_DataType.h"
#include "Usart0.h"
typedef struct
{
	unsigned short 	Len;						//������Ч���ݳ���
	unsigned char 	Data[USART0_RV_DATA_LEN];	//���ݻ���
}tp3762Buffer;									//376.2��ʱ���ݻ���

char ProtoAnaly_Get376_2BufFromCycBuf(Usart0RvBuffer *rvbuffer, tp3762Buffer *buffer);
void DL376_2_LinkFrame(tp3762Buffer *inBuffer, tpFrame376_2 *frame3762);
void DL3762_Process_Request(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762);
char DL3762_Protocol_LinkPack(tpFrame376_2 *snframe3762, tp3762Buffer *tpbuffer);

#endif /* ST376_2_DL376_2_H_ */
