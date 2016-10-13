/*
 * Usart0.c
 *
 *  Created on: 2016年7月14日
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
//static  pthread_cond_t usart_wait;			//串口接收等待处理
static	Usart0RvBuffer UsartBuffer;

/*
 * 函数功能:接收串口数据存入接收缓存中
 */
void *PthreadUsart0Rv(void *data)
{
	int fd;
	int len = 0;
	unsigned short i = 0;
	unsigned char usartBuffer[USART0_RD_DATA_LEN] = {0};	//读取串口缓存
	int l = 0;
	//串口设备文件描述符
	fd = *((int *)data);

	//申请看门狗
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
//		printf("usart0 apply dog error\n");
		system("reboot");
	}
	//设置看门狗
	set_watch_dog(wdt_id, 6);

	while(1)
	{
		usleep(10);

		feed_watch_dog(wdt_id);	//喂狗

		memset(usartBuffer, 0, USART0_RD_DATA_LEN * sizeof(unsigned char));
		printf("");		//去掉打印串口就会不接收，具体原因未明
		len = read(fd, usartBuffer, USART0_RD_DATA_LEN);
		if(len == -1)
		{
			continue;
		}

		if(UsartBuffer.WriteIndex >= UsartBuffer.ReadIndex)  //写>=读
		{
			l = USART0_RV_DATA_LEN - UsartBuffer.WriteIndex + UsartBuffer.ReadIndex;
		}
		else if(UsartBuffer.WriteIndex < UsartBuffer.ReadIndex) //写<读
		{
			l = UsartBuffer.ReadIndex - UsartBuffer.WriteIndex;
		}

		if(l < len)
			continue;

		//printf("len %d\n",len);
		//将接收到的数据存入接收BUFFER
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
 * 函数功能:处理集中器与以太网模块通信的线程
 */
void *Usart0(void *data)
{
	//申请看门狗
	int wdt_id = *(int *)data;
//	printf("Usart0   %d\n",wdt_id);
	feed_watch_dog(wdt_id);	//喂狗

	tp3762Buffer tpbuffer;
	tpFrame376_2 rvframe3762;
	tpFrame376_2 snframe3762;
	int i = 0;
	//int fd;

	memset(&tpbuffer, 0, sizeof(tp3762Buffer));
	memset(&rvframe3762, 0, sizeof(tpFrame376_2));
	memset(&snframe3762, 0 ,sizeof(tpFrame376_2));

	char ret = 0;

	//设置串口波特率
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

	//打开串口1设备文件
	Usart0Fd = open("/dev/ttySP0", O_RDWR | O_NOCTTY);
	if(Usart0Fd < 0)
	{
		printf("open ttySP0 err\n");
		pthread_exit(NULL);
	}
	fcntl(Usart0Fd, F_SETFL, FNDELAY);	//串口不阻塞
	printf("open usart0 ok \n");
	//68 3d 00 81 00 00 00 00 00 02 03 02 01 f1 00 60 00 00 00 5a ff 00 fa
	//00 e2 00 28 22 22 53 06 00 81 51 f8 07 03 00 16 07 27 09 13 43
	//54 33 52 03 12 15 88 62 b0 04 58 02 64 00 32 dc 16
	//休眠2秒等待串口稳定
	sleep(3);
	//创建串口接收线程
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, PthreadUsart0Rv, (void*)&Usart0Fd))
	{
		perror("PthreadUsart0Rv  pthread create");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	//创建线程锁
	pthread_mutex_init(&mylock, NULL);
	pthread_mutex_init(&writelock, NULL);	//防止多线程同时对串口写

	unsigned char testbuff[61] = {0x68, 0x3d, 0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x02, 0x01, 0xf1, 0x00, 0x60, 0x00, 0x00, 0x00, 0x5a, 0xff, 0x00, 0xfa, \
		0x00, 0xe2, 0x00, 0x28, 0x22, 0x22, 0x53, 0x06, 0x00, 0x81, 0x51, 0xf8, 0x07, 0x03, 0x00, 0x16, 0x07, 0x27, 0x09, 0x13, 0x43, \
		0x54, 0x33, 0x52, 0x03, 0x12, 0x15, 0x88, 0x62, 0xb0, 0x04, 0x58, 0x02, 0x64, 0x00, 0x32, 0xdc, 0x16
		};
	pthread_mutex_lock(&mylock);
	UsartSend(Usart0Fd, testbuff, 61);
	pthread_mutex_unlock(&mylock);
	sleep(1);

	//请求集中器时钟
	unsigned char testbuff1[15] = {0x68, 0x0F, 0x00, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x02, 0x00, 0xDA, 0x16};
	pthread_mutex_lock(&mylock);
	UsartSend(Usart0Fd, testbuff1, 15);
	pthread_mutex_unlock(&mylock);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗

		usleep(1);
		pthread_mutex_lock(&mylock);

		//串口有接收时，才返回，无接收时阻塞
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

		//判断是否有执行结束相关需要重启的指令
		if(0x66 == _RebootUsart0)
		{
			sleep(3);
			system("reboot");
		}

	}

	pthread_join(pt, NULL);
	pthread_exit(NULL);
}

