/*
 * Usart0.c
 *
 *  Created on: 2016��7��14��
 *      Author: j
 */
#define _USART0_C_
#include "Usart0.h"
#undef _USART0_C_

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <HLDUsart.h>
#include <limits.h>
#include <pthread.h>
#include "DL376_2.h"
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "SysPara.h"
#include "log3762.h"
#include "HLDWatchDog.h"

pthread_mutex_t mylock;
//static  pthread_cond_t usart_wait;			//���ڽ��յȴ�����
static	Usart0RvBuffer UsartBuffer;

/*
 * ��������:���մ������ݴ�����ջ�����
 */
void *PthreadUsart0Rv(void *data)
{
	int fd;
	int len = 0;
	unsigned short i = 0;
	unsigned char usartBuffer[USART0_RD_DATA_LEN] = {0};	//��ȡ���ڻ���
	int l = 0;
	//�����豸�ļ�������
	fd = *((int *)data);

	//���뿴�Ź�
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
//		printf("usart0 apply dog error\n");
		system("reboot");
	}
	//���ÿ��Ź�
	set_watch_dog(wdt_id, 6);

	while(1)
	{
		usleep(10);

		feed_watch_dog(wdt_id);	//ι��

		memset(usartBuffer, 0, USART0_RD_DATA_LEN * sizeof(unsigned char));
		printf("");		//ȥ����ӡ���ھͻ᲻���գ�����ԭ��δ��
		len = read(fd, usartBuffer, USART0_RD_DATA_LEN);
		if(len == -1)
		{
			continue;
		}

		if(UsartBuffer.WriteIndex >= UsartBuffer.ReadIndex)  //д>=��
		{
			l = USART0_RV_DATA_LEN - UsartBuffer.WriteIndex + UsartBuffer.ReadIndex;
		}
		else if(UsartBuffer.WriteIndex < UsartBuffer.ReadIndex) //д<��
		{
			l = UsartBuffer.ReadIndex - UsartBuffer.WriteIndex;
		}

		if(l < len)
			continue;

		//printf("len %d\n",len);
		//�����յ������ݴ������BUFFER
		pthread_mutex_lock(&mylock);
		for(i = 0; i < len; i++)
		{
			//printf(" %x",usartBuffer[i]);
			if(USART0_RV_DATA_LEN == UsartBuffer.WriteIndex)
			{
				UsartBuffer.WriteIndex = 0;
			}
			UsartBuffer.DataBuffer[UsartBuffer.WriteIndex] = usartBuffer[i];
			UsartBuffer.WriteIndex++;
		}
		//printf("\n");
		pthread_mutex_unlock(&mylock);
//		pthread_cond_broadcast(&(usart_wait));
	}
	close(fd);
	pthread_exit(NULL);
}

/*
 * ��������:������������̫��ģ��ͨ�ŵ��߳�
 */
void *Usart0(void *data)
{
	//���뿴�Ź�
	int wdt_id = *(int *)data;
//	printf("Usart0   %d\n",wdt_id);
	feed_watch_dog(wdt_id);	//ι��

	tp3762Buffer tpbuffer;
	tpFrame376_2 rvframe3762;
	tpFrame376_2 snframe3762;
	int i = 0;
	//int fd;

	memset(&tpbuffer, 0, sizeof(tp3762Buffer));
	memset(&rvframe3762, 0, sizeof(tpFrame376_2));
	memset(&snframe3762, 0 ,sizeof(tpFrame376_2));

	char ret = 0;

	//���ô��ڲ�����
	UsartAttribute usart;
	usart.BaudRate = 9600;
	usart.DataLen = 8;
	usart.FlowCtl = 0;
	usart.isParity = 2;
	usart.isStop = 1;
	if(0 != SetUsart(0, &usart))
	{
//		printf("usart set erro\n");
	}

	//�򿪴���1�豸�ļ�
	Usart0Fd = open("/dev/ttySP0", O_RDWR | O_NOCTTY);
	if(Usart0Fd < 0)
	{
		printf("open ttySP0 err\n");
		pthread_exit(NULL);
	}
	fcntl(Usart0Fd, F_SETFL, FNDELAY);	//���ڲ�����
	printf("open usart0 ok \n");
	//68 3d 00 81 00 00 00 00 00 02 03 02 01 f1 00 60 00 00 00 5a ff 00 fa
	//00 e2 00 28 22 22 53 06 00 81 51 f8 07 03 00 16 07 27 09 13 43
	//54 33 52 03 12 15 88 62 b0 04 58 02 64 00 32 dc 16
	//����2��ȴ������ȶ�
	sleep(3);
	//�������ڽ����߳�
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, PthreadUsart0Rv, (void*)&Usart0Fd))
	{
		perror("PthreadUsart0Rv  pthread create");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	//�����߳���
	pthread_mutex_init(&mylock, NULL);
	pthread_mutex_init(&writelock, NULL);	//��ֹ���߳�ͬʱ�Դ���д

	unsigned char testbuff[61] = {0x68, 0x3d, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x02, 0x01, 0xf1, 0x00, 0x60, 0x00, 0x00, 0x00, 0x5a, 0xff, 0x00, 0xfa, \
		0x00, 0xe2, 0x00, 0x28, 0x22, 0x22, 0x53, 0x06, 0x00, 0x81, 0x51, 0xf8, 0x07, 0x03, 0x00, 0x16, 0x07, 0x27, 0x09, 0x13, 0x43, \
		0x54, 0x33, 0x52, 0x03, 0x12, 0x15, 0x88, 0x62, 0xb0, 0x04, 0x58, 0x02, 0x64, 0x00, 0x32, 0xdc, 0x16
		};
	pthread_mutex_lock(&mylock);
	UsartSend(Usart0Fd, testbuff, 61);
	pthread_mutex_unlock(&mylock);
	sleep(1);

	//��������ʱ��
	unsigned char testbuff1[15] = {0x68, 0x0F, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x02, 0x00, 0xDA, 0x16};
	pthread_mutex_lock(&mylock);
	UsartSend(Usart0Fd, testbuff1, 15);
	pthread_mutex_unlock(&mylock);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��

		usleep(1);
		pthread_mutex_lock(&mylock);

		//�����н���ʱ���ŷ��أ��޽���ʱ����
		ret = ProtoAnaly_Get376_2BufFromCycBuf(&UsartBuffer, &tpbuffer);

		pthread_mutex_unlock(&mylock);
		if(0 == ret)
		{
			printf("ret376.2\n");
			DL376_2_LinkFrame(&tpbuffer, &rvframe3762);
			for(i = 0; i < tpbuffer.Len; i++)
			{
				printf(" %02x",tpbuffer.Data[i]);
			}
			printf("\n");
			log_3762_task_add(&_log_3762_task, tpbuffer.Data, tpbuffer.Len, 0x00);
		}
		else
		{
			continue;
		}

		if(true == rvframe3762.IsHaving && true != snframe3762.IsHaving)
		{
			DL3762_Process_Request(&rvframe3762, &snframe3762);
		}

		if(true == snframe3762.IsHaving)
		{
			if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
			{
				pthread_mutex_lock(&writelock);
				UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
				log_3762_task_add(&_log_3762_task, tpbuffer.Data, tpbuffer.Len, 0x01);
				pthread_mutex_unlock(&writelock);
			}
			memset(&snframe3762, 0, sizeof(tpFrame376_2));
		}

		//�ж��Ƿ���ִ�н��������Ҫ������ָ��
		if(0x66 == _RebootUsart0)
		{
			sleep(3);
			system("reboot");
		}

	}

	pthread_join(pt, NULL);
	pthread_exit(NULL);
}

