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

#define NET_COLLECT_TIMER 4		//网络抄表等待回复时间
#define AFN14_01_TIMER	  6	//请求抄读数据内容等待回复时间

#define EXE_COLLECT1	  6		//广播下发任务后，需延时6秒(真实事件为3-4秒)

#define TASKA_THREAD_SIZE	60	//处理充值任务线程个数

struct task_a_content					//充值任务内容
{
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char top_ter[TER_ADDR_LEN];//充值终端  终端地址
	unsigned char buf[TASK_MAX_LEN];	//数据
	int 		  len;					//数据长度
	struct task_a_content *next;		//下一个任务内容
};

struct task_a_status	//任务状态
{
	unsigned char dadt[DADT_LEN];		//数据标识
	unsigned char top_ter[TER_ADDR_LEN];//充值终端  终端地址
	unsigned char timer;				//任务倒计时(执行任务 等待成功返回)
	unsigned char is_end;				//任务是否结束	0结束  1任务正在进行
};

#define TASKA_NODE_LIVE_TIMER	60 * 5		//节点生存倒计时 1min
#define TASKA_TIMEOUT			4		//充值任务 超时时间 4s

struct task_a_node						//充值任务的任务队列中的节点(以任务电表的终端地址为节点)
{
	unsigned char amm[AMM_ADDR_LEN];	//表号
	unsigned char ter[TER_ADDR_LEN];	//终端(电表对应的终端，终端地址来作为任务队列分支的节点)
	unsigned char is_allot;				//是否分配给线程  0为未分配  1为已分配
	unsigned int  timer;				//节点存在倒计时(为0时，删除该节点的分支)
	struct task_a_content *task_head;	//头任务
	struct task_a_content *task_tail;	//尾任务
	struct task_a_status task_status;	//任务执行状态
	struct task_a_node *next;			//下一个节点
};

typedef struct
{
	int task_num;					//任务数量
	int not_assigned_num;			//没有分配给线程的终端个数
	int key;						//允许执行任务开关  1允许  非1 不允许
	pthread_mutex_t taska_mutex;	//任务a锁
	pthread_cond_t need_assigned;	//有终端需要分配给线程  条件变量
	pthread_cond_t node_not_empty;	//节点不为空条件变量

	struct task_a_node *node_head;	//任务节点队列的第一个节点
	struct task_a_node *node_tail;	//任务节点队列的最后一个节点
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
	int				index;				//台账电表索引
	int 			timer;				//请求任务倒计时
	int             count;				//请求任务失败次数
	int				isok;				//请求任务是否成功
	unsigned char 	dadt[DADT_LEN];		//数据标识
	unsigned char 	buf[TASK_MAX_LEN];	//任务数据
	int				len;				//任务数据长度
	int				key;				//开关
	pthread_mutex_t taskc_mutex;		//锁
}CollcetTaskC;	//周期抄表任务

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
	unsigned char runingtask;
	unsigned char amm[AMM_ADDR_LEN];	//表号		 (充值任务的表号不在此判断)
	unsigned char dadt[DADT_LEN];		//数据标识        (充值任务的数据标识不在此判断)
	unsigned char timer;				//抄表超时判断 (充值任务的超时不在此判断)
	unsigned char isend;				//该数据标识抄收是否结束
	unsigned char conut;				//抄表失败次数 (充值终端的抄表失败次数不在此判断)
}CollcetStatus;

typedef struct
{
	CollcetTaskA taska;
	CollcetTaskB taskb;
	CollcetTaskC taskc;
	CollcetTaskE taske;
//	unsigned char runingtask;			//正在执行的任务类型 'a' 'b' 'c' 'd' 'e'
	pthread_mutex_t mutex;				//锁
//	unsigned char amm[AMM_ADDR_LEN];	//表号		 (充值任务的表号不在此判断)
//	unsigned char dadt[DADT_LEN];		//数据标识        (充值任务的数据标识不在此判断)
//	unsigned char timer;				//抄表超时判断 (充值任务的超时不在此判断)
//
//	unsigned char b_isend;				//该数据标识抄收是否结束
//	unsigned char c_isend;				//该数据标识抄收是否结束
//	unsigned char e_isend;				//该数据标识抄收是否结束
//	unsigned char conut;				//抄表失败次数 (充值终端的抄表失败次数不在此判断)
	CollcetStatus status;				//抄表器状态
}Collcet;

typedef struct
{
	pthread_mutex_t mutex;				//锁
	volatile int	timer;				//广播全局倒计时（只有倒计时结束后，才能继续对终端进行数据下发）
	volatile int	allow_num;			//广播许可次数
	volatile int 	flush_allow_timer;	//刷新许可倒计时(在该倒计时时间段内，最大广播次数为allow_num)
}BoradcastContorl;						//广播管理器(当执行完广播后需等待n秒后才能对终端进行数据下发)

#ifdef	_COOLECT_C_
Collcet _Collect;	//抄表器
BoradcastContorl _BoradContorl;
#endif
/*********************************Collector**************************************/
void CollectorInit(void);
void TaskcReset(void);
void *Collector(void *arg);
void GetCollectorStatus(CollcetStatus *status);
void SetCollectorStatus(CollcetStatus *status);
unsigned char GetCollectorStatusTimer(void);
void CollectorStatusTimerTicker(void);
void BoradcastContorlInit(void);
int GetBoradcastContorlTimer(void);
void SetBoradcastContorlTimer(int t);
void BoradcastContorlTimerTicker(void);
int GetBoradcastContorlAllowTimer(void);
void SetBoradcastContorlAllowTimer(int t);
void BoradcastContorlAllowTimerTicker(void);
int GetBoradcastContorlAllowNum(void);
void SetBoradcastContorlAllowNum(int num);

/************************************A***************************************/
void TaskACollectInit(void);
void *TaskATimer(void *arg);
void *CollectTaskA(void *arg);
struct task_a_node *SeekTaskANode0(unsigned char *ter);
struct task_a_node *SeekTaskANode1(unsigned char *amm);
int AddTaskA(unsigned char *amm, unsigned char *dadt, unsigned char *buf, int len, unsigned char *ter);
int DeleTaskA(unsigned char *amm, unsigned char flag);
int TaskaIsEmpty(void);
void SetCollectorStatus(CollcetStatus *status);
void SetTaskaKey(int key);
int GetTaskaKey(void);

/*************************************B*******************************************/
void CollectTaskB(void);
int AddTashB(struct task_b *b);
void DeleNearTaskB(void);
int GetTaskbNum(void);

/************************************C*************************************************/
void CollectTaskC(void);
int GetTaskcKey(void);
int GetRequestTimer(void);
void RequestTimerTicker(void);

/**************************************E*************************************************/
int GetTaskeNum(void);
int AddTashE(struct task_e *e);
void DeleNearTaskE(void);
void CollectTaskE(void);

int ExecuteCollect0(unsigned char *amm, unsigned char *inbuf, int len);
int ExecuteCollect1(unsigned char *amm, unsigned char *inbuf, int len);

#ifndef _COOLECT_C_
extern Collcet _Collect;	//抄表器
extern BoradcastContorl _BoradContorl;
#endif

#endif /* COLLECT_COLLECT_H_ */
