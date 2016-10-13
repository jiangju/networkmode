/*
 * DL376_1.h
 *
 *  Created on: 2016��7��14��
 *      Author: j
 */

#ifndef ST376_1_DL376_1_H_
#define ST376_1_DL376_1_H_
#include "DL376_1_DataType.h"
#define	TEM_BUFF_LEN	2048	//376.1��ʱ���ݻ�����󳤶�
#define DL3761MSA		55		//Ĭ��MSAֵ

typedef struct
{
	unsigned short 	Len;				//������Ч���ݳ���
	unsigned char 	Data[TEM_BUFF_LEN];	//���ݻ���
}tp3761Buffer;							//376.1��ʱ���ݻ���

#ifdef _DL3761_C_
unsigned char _SendNum;
#endif

void DL376_1_LinkFrame(tp3761Buffer *inBuffer, tpFrame376_1 *frame3761);
int DL3761_Protocol_LinkPack(tpFrame376_1 *snframe3761, tp3761Buffer *outbuffer);
int ProtoAnaly_Effic376_1Frame(tp3761Buffer *buf);
int ProtoAnaly_Get376_1BufFromCycBuf(unsigned char *inbuf, unsigned short maxlen,\
		unsigned short *r, unsigned short *w, tp3761Buffer *outbuf);
void DL3761_Process_Response(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761);
#ifndef _DL3761_C_
extern unsigned char _SendNum;
#endif

#endif /* ST376_1_DL376_1_H_ */
