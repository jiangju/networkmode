/*
 * HLDWatchDog.c
 *
 *  Created on: 2016年8月18日
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

static pthread_mutex_t wdt_mu;		//看门狗锁
static HLDWdt _HLDWdt;

/*
 * 函数功能:初始化看门狗
 * */
void watch_dog_init(void)
{
	memset(&_HLDWdt, 0x00, sizeof(HLDWdt));
	pthread_mutex_init(&wdt_mu, NULL);
}

/*
 * 函数功能:申请看门狗
 * 返回值: >= 0看门狗id -1 错误
 * */
int apply_watch_dog(void)
{
	int i = 0;
	pthread_mutex_lock(&wdt_mu);
	for(i = 0; i < WDT_MAX; i++)
	{
		if(0 ==_HLDWdt.wdt_id[i])	//判断是否申请
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
 * 函数功能:设置看门狗
 * 参数	id	看门狗id
 * 		max	最大超时时间
 * 返回值 0成功 -1 失败
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
 * 函数功能:喂狗
 * 参数: id	看门狗id
 * 返回值: 0成功 -1失败
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
 * 函数功能:关闭看门狗
 * 参数:id 	看门狗id
 * 返回值 0 成功 -1 失败
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
 * 函数功能:看门狗线程
 * */
void *HLDWatchDog(void *arg)
{
	int fd;
	int timeout = 10;
	int i = 0;
	int res = 0;

	//打开物理看门狗监控华立达看门狗线程
	fd = open(WDT_HARD, O_WRONLY);
	if(fd < 0)
	{
		perror("open watch dog:");
		pthread_exit(NULL);
	}
	//设置物理看门狗超时时间
	ioctl(fd, WDIOC_SETTIMEOUT, &timeout);

	//监控小看狗
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&wdt_mu);
		for(i = 0; i < WDT_MAX; i++)
		{
			if(1 == _HLDWdt.wdt_id[i])
			{
				//对有效的看门狗进行超时判断
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
		write(fd, "\0", 1);	//喂狗物理狗
	}
	close(fd);
	pthread_exit(NULL);
}

