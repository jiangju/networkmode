/*
 * HLDWatchDog.h
 *
 *  Created on: 2016年8月18日
 *      Author: j
 */

#ifndef COMMONALITY_HLDWATCHDOG_H_
#define COMMONALITY_HLDWATCHDOG_H_
#include <stdio.h>
#include <pthread.h>
#define WDT_HARD	"/dev/watchdog"	//物理看门狗(大看门狗)

#define WDT_MAX		20				//小看门狗个数

typedef struct
{
	int wdt_id[WDT_MAX];		//存放 0 1， 0 表示看门狗未申请  1表示看门狗已经申请， 索引号为看门狗id
	int	wdt_time[WDT_MAX];		//看门狗计时器
	int wdt_cont[WDT_MAX];		//最大计算，当wdt_time > wdt_cont时复位系统
}HLDWdt;		//华立达看门狗

void watch_dog_init(void);
int apply_watch_dog(void);
int set_watch_dog(int id, int max);
int feed_watch_dog(int id);
int close_watch_dog(int id);
void *HLDWatchDog(void *arg);

#endif /* COMMONALITY_HLDWATCHDOG_H_ */
