/*
 * HLDWatchDog.c
 *
 *  Created on: 2016��8��18��
 *      Author: j
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <pthread.h>
#include "HLDWatchDog.h"
#include <string.h>
//#include <un>

static pthread_mutex_t wdt_mu;		//���Ź���
static HLDWdt _HLDWdt;

/*
 * ��������:��ʼ�����Ź�
 * */
void watch_dog_init(void)
{
	memset(&_HLDWdt, 0x00, sizeof(HLDWdt));
	pthread_mutex_init(&wdt_mu, NULL);
}

/*
 * ��������:���뿴�Ź�
 * ����ֵ: >= 0���Ź�id -1 ����
 * */
int apply_watch_dog(void)
{
	int i = 0;
	pthread_mutex_lock(&wdt_mu);
	for(i = 0; i < WDT_MAX; i++)
	{
		if(0 ==_HLDWdt.wdt_id[i])	//�ж��Ƿ�����
		{
			_HLDWdt.wdt_id[i] = 1;
			pthread_mutex_unlock(&wdt_mu);
			return i;
		}
	}
	pthread_mutex_unlock(&wdt_mu);
	return -1;
}

/*
 * ��������:���ÿ��Ź�
 * ����	id	���Ź�id
 * 		max	���ʱʱ��
 * ����ֵ 0�ɹ� -1 ʧ��
 * */
int set_watch_dog(int id, int max)
{
	if(id < 0 || id >= WDT_MAX || max <= 0)
	{
		return -1;
	}

	pthread_mutex_lock(&wdt_mu);
	//_HLDWdt.wdt_id[id] = 1;
	_HLDWdt.wdt_time[id] = 0;
	_HLDWdt.wdt_cont[id] = max;
	pthread_mutex_unlock(&wdt_mu);
	return 0;
}

/*
 * ��������:ι��
 * ����: id	���Ź�id
 * ����ֵ: 0�ɹ� -1ʧ��
 * */
int feed_watch_dog(int id)
{
	if(id < 0 || id >= WDT_MAX)
	{
		return -1;
	}

	pthread_mutex_lock(&wdt_mu);
	_HLDWdt.wdt_time[id] = 0;
	pthread_mutex_unlock(&wdt_mu);
	return 0;
}

/*
 * ��������:�رտ��Ź�
 * ����:id 	���Ź�id
 * ����ֵ 0 �ɹ� -1 ʧ��
 * */
int close_watch_dog(int id)
{
	if(id < 0 || id >= WDT_MAX)
		return -1;

	pthread_mutex_lock(&wdt_mu);
	_HLDWdt.wdt_id[id] = 0;
	pthread_mutex_unlock(&wdt_mu);
	return 0;
}

/*
 * ��������:���Ź��߳�
 * */
void *HLDWatchDog(void *arg)
{
	int fd;
	int timeout = 10;
	int i = 0;
	int res = 0;

	//�������Ź���ػ����￴�Ź��߳�
	fd = open(WDT_HARD, O_WRONLY);
	if(fd < 0)
	{
		perror("open watch dog:");
		pthread_exit(NULL);
	}
	//���������Ź���ʱʱ��
	ioctl(fd, WDIOC_SETTIMEOUT, &timeout);

	//���С����
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&wdt_mu);
		for(i = 0; i < WDT_MAX; i++)
		{
			if(1 == _HLDWdt.wdt_id[i])
			{
				//����Ч�Ŀ��Ź����г�ʱ�ж�
				if(_HLDWdt.wdt_time[i] >= _HLDWdt.wdt_cont[i] && _HLDWdt.wdt_cont[i] > 0)
				{
					res = 1;
					break;
				}
				_HLDWdt.wdt_time[i]++;
			}
		}
		pthread_mutex_unlock(&wdt_mu);
		if(res == 1)
		{
			printf("*********************************************TIME OUT i = %d\n",i);
			break;
		}
		write(fd, "\0", 1);	//ι������
	}
	close(fd);
	pthread_exit(NULL);
}

