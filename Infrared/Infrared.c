/*
 * Infrared.c
 *
 *  Created on: 2016��6��24��
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

static int _fd_infrared;				//�����豸������
static pthread_mutex_t	_infrared_lock;	//������
//static pthread_cond_t infrared_wait;	//���ڽ��յȴ�����
static InfraredRvBuffer _InfaraedBuffer;//������ջ���
/*
 * ��������:��������߳�
 * */
void *PthreadInfraredRv(void *arg)
{
	//int fd;
	int len = 0;
	unsigned short i = 0;
	unsigned char usartBuffer[INFRARED_RD_DATA_LEN] = {0};
	int l = 0;

	//���뿴�Ź�
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	//���ÿ��Ź�
	set_watch_dog(wdt_id, 6);

	//����Ĵ����豸�ļ�������
	fcntl(_fd_infrared, F_SETFL, FNDELAY);	//���ڲ�����
	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��

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
 * ��������:�����߳�
 * */
void *Infrared(void *data)
{
	//���뿴�Ź�
	int wdt_id = *(int *)data;
	feed_watch_dog(wdt_id);	//ι��

	//���ô��ڲ�����
	//���ô��ڲ�����
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

	//�򿪴���1�豸�ļ�
	_fd_infrared = open("/dev/ttySP1", O_RDWR | O_NOCTTY | O_NDELAY);
	if(_fd_infrared < 0)
	{
		printf("open ttySP1 err\n");
		return NULL;
	}

	//������������߳�
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, PthreadInfraredRv, NULL))
	{
		//�����߳�ʧ�ܣ�ʹ���Ź���λ
		while(1);
	}

	//�����߳���
	pthread_mutex_init(&_infrared_lock, NULL);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��

		usleep(1);
		pthread_mutex_lock(&_infrared_lock);

		//�����н���ʱ���ŷ��أ��޽���ʱ����
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
		//�ж��Ƿ���Ҫ����
		if(0x66 == _RebootInfrared)
		{
			while(1);
		}
	}
	pthread_exit(NULL);
}
