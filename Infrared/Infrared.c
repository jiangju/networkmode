/*
 * Infrared.c
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <HLDUsart.h>
#include <termios.h>
#include <limits.h>
#include <pthread.h>
#include "Infrared.h"
#include "DL376_1.h"
#include <stdlib.h>
#include "SysPara.h"
#include <HLDWatchDog.h>

static int _fd_infrared;				//红外设备描述符
static pthread_mutex_t	_infrared_lock;	//红外锁
//static pthread_cond_t infrared_wait;	//串口接收等待处理
static InfraredRvBuffer _InfaraedBuffer;//红外接收缓存
/*
 * 函数功能:红外接收线程
 * */
void *PthreadInfraredRv(void *arg)
{
	//int fd;
	int len = 0;
	unsigned short i = 0;
	unsigned char usartBuffer[INFRARED_RD_DATA_LEN] = {0};
	int l = 0;

	//申请看门狗
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	//设置看门狗
	set_watch_dog(wdt_id, 6);

	//红外的串口设备文件描述符
	fcntl(_fd_infrared, F_SETFL, FNDELAY);	//串口不阻塞
	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗

		usleep(1);
		memset(usartBuffer, 0, INFRARED_RD_DATA_LEN);
		printf("");
		len = read(_fd_infrared, usartBuffer, INFRARED_RD_DATA_LEN);
		if(len == -1)
		{
			continue;
		}

		if(_InfaraedBuffer.WriteIndex >= _InfaraedBuffer.ReadIndex)
		{
			l = INFRARED_RV_DATA_LEN - _InfaraedBuffer.WriteIndex + _InfaraedBuffer.ReadIndex;
		}
		else
		{
			l = _InfaraedBuffer.ReadIndex - _InfaraedBuffer.WriteIndex;
		}

		if(l < len)
		{
			continue;
		}

		pthread_mutex_lock(&_infrared_lock);
		for(i = 0; i < len; i++)
		{
			if(INFRARED_RD_DATA_LEN == _InfaraedBuffer.WriteIndex)
			{
				_InfaraedBuffer.WriteIndex = 0;
			}
			_InfaraedBuffer.DataBuffer[_InfaraedBuffer.WriteIndex] = usartBuffer[i];
			_InfaraedBuffer.WriteIndex++;
		}
		pthread_mutex_unlock(&_infrared_lock);
//		pthread_cond_broadcast(&(infrared_wait));
	}
	close(_fd_infrared);
	pthread_exit(NULL);
}

/*
 * 函数功能:红外线程
 * */
void *Infrared(void *data)
{
	//申请看门狗
	int wdt_id = *(int *)data;
	feed_watch_dog(wdt_id);	//喂狗

	//设置串口波特率
	//设置串口波特率
	UsartAttribute usart;
	usart.BaudRate = 1200;
	usart.DataLen = 8;
	usart.FlowCtl = 0;
	usart.isParity = 2;
	usart.isStop = 1;
	if(0 != SetUsart(1, &usart))
	{
		printf("usart set erro\n");
	}

	char ret = 0;
	tp3761Buffer outbuf;
	tpFrame376_1 rvframe3761;
	tpFrame376_1 snframe3761;

	//打开串口1设备文件
	_fd_infrared = open("/dev/ttySP1", O_RDWR | O_NOCTTY | O_NDELAY);
	if(_fd_infrared < 0)
	{
		printf("open ttySP1 err\n");
		return NULL;
	}

	//创建红外接收线程
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, PthreadInfraredRv, NULL))
	{
		//创建线程失败，使看门狗复位
		while(1);
	}

	//创建线程锁
	pthread_mutex_init(&_infrared_lock, NULL);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗

		usleep(1);
		pthread_mutex_lock(&_infrared_lock);

		//红外有接收时，才返回，无接收时阻塞
//		pthread_cond_wait(&(infrared_wait), &(_infrared_lock));

		ret = ProtoAnaly_Get376_1BufFromCycBuf(_InfaraedBuffer.DataBuffer, INFRARED_RV_DATA_LEN, \
				&_InfaraedBuffer.ReadIndex, &_InfaraedBuffer.WriteIndex, &outbuf);
		pthread_mutex_unlock(&_infrared_lock);
		if(0 == ret)
		{
			DL376_1_LinkFrame(&outbuf, &rvframe3761);
		}
		else
		{
			continue;
		}

		if(true == rvframe3761.IsHaving)
		{
			DL3761_Process_Response(&rvframe3761, &snframe3761);
		}

		if(true == snframe3761.IsHaving)
		{
			if(0 == DL3761_Protocol_LinkPack(&snframe3761, &outbuf))
			{
				UsartSend(_fd_infrared, outbuf.Data, outbuf.Len);
			}
		}
		//判断是否需要重启
		if(0x66 == _RebootInfrared)
		{
			while(1);
		}
	}
	pthread_exit(NULL);
}
