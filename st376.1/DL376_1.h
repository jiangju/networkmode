/*
 * DL376_1.h
 *
 *  Created on: 2016年7月14日
 *      Author: j
 */

#ifndef ST376_1_DL376_1_H_
#define ST376_1_DL376_1_H_
#include "DL376_1_DataType.h"
#define	TEM_BUFF_LEN	2048	//376.1临时数据缓存最大长度
#define DL3761MSA		55		//默认MSA值

typedef struct
{
	unsigned short 	Len;				//缓存有效数据长度
	unsigned char 	Data[TEM_BUFF_LEN];	//数据缓存
	unsigned char 	src;				//数据源   0x11 红外  0x22三网合一   0x33充值   0x44 串口
}tp3761Buffer;							//376.1临时数据缓存

#ifdef _DL3761_C_
unsigned char _SendNum;
#endif

void DL376_1_LinkFrame(tp3761Buffer *inBuffer, tpFrame376_1 *frame3761);
int DL3761_Protocol_LinkPack(tpFrame376_1 *snframe3761, tp3761Buffer *outbuffer);
int ProtoAnaly_Effic376_1Frame(tp3761Buffer *buf);
int ProtoAnaly_Get376_1BufFromCycBuf(unsigned char *inbuf, unsigned short maxlen,\
		unsigned short *r, unsigned short *w, unsigned char src, tp3761Buffer *outbuf);
void DL3761_Process_Response(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761);
void Creat_DL3761_No(tpFrame376_1 *inbuf, tpFrame376_1 *outbuf, char flag);
#ifndef _DL3761_C_
extern unsigned char _SendNum;
#endif

#endif /* ST376_1_DL376_1_H_ */
