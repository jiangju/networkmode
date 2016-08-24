/*
 * Collect.h
 *
 *  Created on: 2016年7月21日
 *      Author: j
 */

#ifndef COLLECT_COLLECT_H_
#define COLLECT_COLLECT_H_
#include "SysPara.h"
#include "StandBook.h"
#include <pthread.h>
#define TASK_MAX_LEN	300		//采集任务缓存最大长度
#define TASK_MAX_NUM	100		//任务缓存中最大任务数量

#define NET_COLLECT_TIMER 6	//网络抄表等待回复时间
#define AFN14_01_TIMER	  15	//请求抄读数据内容等待回复时间

#define EXE_COLLECT1	  6		//广播下发任务后，需延时6秒(真实事件为3-4秒)

struct task_a
{
	unsigned char amm[AMM_ADDR_LEN];	//表号
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char buf[TASK_MAX_LEN];	//数据
	int			  len;					//数据长度
	unsigned char ter[TER_ADDR_LEN];	//请求该任务的终端
	struct task_a *next;				//下一个任务
};

typedef struct
{
	int num;	//任务数量
	pthread_mutex_t taska_mutex;	//任务a锁
	struct task_a *next;	//下个任务
}CollcetTaskA;	//充值终端任务

struct task_b
{
	unsigned char amm[NODE_ADDR_LEN];	//表号
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char buf[TASK_MAX_LEN];	//数据
	int 		  len;					//数据长度
	unsigned char type;					//协议类型
	struct task_b *next;				//下一个任务
};

typedef struct
{
	int num;	//任务数量
	pthread_mutex_t taskb_mutex;	//任务B锁
	struct task_b *next;	//下一个任务
}CollcetTaskB;	//实时抄表任务

typedef struct
{
	int				index;				//台账电表
	int 			timer;				//请求任务倒计时
	int             count;				//请求任务失败次数
	int				isok;				//请求任务是否成功
	unsigned char 	dadt[DADT_LEN];		//数据标识
	unsigned char 	buf[TASK_MAX_LEN];	//任务数据
	int				len;				//任务数据长度
	int				key;//开关
}CollcetTaskC;	//周期抄表任务

typedef struct
{
	int	index;	//台账索引
	int key;	//开关
}CollcetTaskD;

struct task_e
{
	unsigned char amm[NODE_ADDR_LEN];	//表号
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char buf[TASK_MAX_LEN];	//数据
	int 		  len;					//数据长度
	unsigned char type;					//协议类型
	struct task_e *next;				//下一个任务
};

typedef struct
{
	int num;	//任务数量
	pthread_mutex_t taske_mutex;	//任务E锁
	struct task_e *next;	//下一个任务
}CollcetTaskE;	//实时透明转发任务  376.2透明转发

typedef struct
{
	CollcetTaskA taska;
	CollcetTaskB taskb;
	CollcetTaskC taskc;
	CollcetTaskD taskd;
	CollcetTaskE taske;
	unsigned char runingtask;			//正在执行的任务类型 'a' 'b' 'c' 'd' 'e'
	unsigned char amm[AMM_ADDR_LEN];	//表号
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char timer;				//抄表超时判断
	unsigned char a_isend;				//该数据标识抄收是否结束
	unsigned char b_isend;				//该数据标识抄收是否结束
	unsigned char c_isend;				//该数据标识抄收是否结束
	unsigned char d_isend;				//该数据标识抄收是否结束
	unsigned char e_isend;				//该数据标识抄收是否结束
	unsigned char conut;				//抄表失败次数
//	int			  s;					//充值终端套接字
//	int  		  flag;					//该套接字是否有效
}Collcet;

#ifdef	_COOLECT_C_
Collcet _Collect;	//抄表器
#endif

void CollectorInit(void);
void TaskcReset(void);
void *Collector(void *arg);
void CollectTaskB(void);
void CollectTaskC(void);
void CollectTaskE(void);
int AddTaskA(struct task_a *a);
void DeleNearTaskA(void);
int AddTashB(struct task_b *b);
void DeleNearTaskB(void);
int AddTashE(struct task_e *e);
void DeleNearTaskE(void);
int ExecuteCollect0(unsigned char *amm, unsigned char *inbuf, int len);
int ExecuteCollect1(unsigned char *amm, unsigned char *inbuf, int len);
#ifndef _COOLECT_C_
extern Collcet _Collect;	//抄表器
#endif

#endif /* COLLECT_COLLECT_H_ */
