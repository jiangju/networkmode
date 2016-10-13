/*
 * Usart0.h
 *
 *  Created on: 2016��7��14��
 *      Author: j
 */

#ifndef USART0_USART0_H_
#define USART0_USART0_H_
#include <pthread.h>
#define	USART0_RV_DATA_LEN	2048//�������ݽ��ջ��泤��
#define USART0_RD_DATA_LEN	500	//����1һ�ζ�ȡ���ݳ���

typedef struct
{
	unsigned short 	ReadIndex;						//������
	unsigned short 	WriteIndex;						//д����
	unsigned char  	DataBuffer[USART0_RV_DATA_LEN];	//���ݻ���
}Usart0RvBuffer;									//���ڽ��ջ���

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
