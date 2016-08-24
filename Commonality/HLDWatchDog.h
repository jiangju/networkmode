/*
 * HLDWatchDog.h
 *
 *  Created on: 2016��8��18��
 *      Author: j
 */

#ifndef COMMONALITY_HLDWATCHDOG_H_
#define COMMONALITY_HLDWATCHDOG_H_
#include <stdio.h>
#include <pthread.h>
#define WDT_HARD	"/dev/watchdog"	//�����Ź�(���Ź�)

#define WDT_MAX		20				//С���Ź�����

typedef struct
{
	int wdt_id[WDT_MAX];		//��� 0 1�� 0 ��ʾ���Ź�δ����  1��ʾ���Ź��Ѿ����룬 ������Ϊ���Ź�id
	int	wdt_time[WDT_MAX];		//���Ź���ʱ��
	int wdt_cont[WDT_MAX];		//�����㣬��wdt_time > wdt_contʱ��λϵͳ
}HLDWdt;		//�����￴�Ź�

void watch_dog_init(void);
int apply_watch_dog(void);
int set_watch_dog(int id, int max);
int feed_watch_dog(int id);
int close_watch_dog(int id);
void *HLDWatchDog(void *arg);

#endif /* COMMONALITY_HLDWATCHDOG_H_ */
