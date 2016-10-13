/*
 * Usart0.h
 *
 *  Created on: 2016年7月14日
 *      Author: j
 */

#ifndef USART0_USART0_H_
#define USART0_USART0_H_
#include <pthread.h>
#define	USART0_RV_DATA_LEN	2048//串口数据接收缓存长度
#define USART0_RD_DATA_LEN	500	//串口1一次读取数据长度

typedef struct
{
	unsigned short 	ReadIndex;						//读索引
	unsigned short 	WriteIndex;						//写索引
	unsigned char  	DataBuffer[USART0_RV_DATA_LEN];	//数据缓存
}Usart0RvBuffer;									//串口接收缓存

#ifdef _USART0_C_
int Usart0Fd;
pthread_mutex_t writelock;

#endif

#ifndef _USART0_C_

extern int Usart0Fd;
extern pthread_mutex_t writelock;

#endif

void *Usart0(void *data);

#endif /* USART0_USART0_H_ */
