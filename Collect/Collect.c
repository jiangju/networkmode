/*
 * Collect.c
 *
 *  Created on: 2016年7月21日
 *      Author: j
 */
#define _COOLECT_C_
#include "Collect.h"
#undef	_COOLECT_C_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Route.h"
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include "DL3762_AFN14.h"
#include "DL376_2.h"
#include "DL3761_AFN10.h"
#include "DL376_1_DataType.h"
#include "DL376_1.h"
#include <unistd.h>
#include <HLDUsart.h>
#include <pthread.h>
#include "CommLib.h"
#include "HLDWatchDog.h"
#include "DL3762_AFN13.h"
#include "DL645.h"
#include "SeekAmm.h"
#include "NetWork1.h"
#include "hld_ac.h"
/*
 * 函数功能:初始化抄表器
 * */
void CollectorInit(void)
{
	memset(&_Collect, 0, sizeof(Collcet));

	pthread_mutex_init(&_Collect.taska.taska_mutex, NULL);	//任务A锁
	pthread_cond_init(&_Collect.taska.need_assigned, NULL); //条件变量
	pthread_cond_init(&_Collect.taska.node_not_empty, NULL);//条件表里

	_Collect.taska.task_num = 0;							//任务总数量
	_Collect.taska.not_assigned_num = 0;					//没有分派的节点个数
	_Collect.taska.key = 0;									//不允许充值任务执行
	_Collect.taska.node_head = NULL;
	_Collect.taska.node_tail = NULL;

	pthread_mutex_init(&_Collect.taskb.taskb_mutex, NULL);	//任务B锁
	pthread_mutex_init(&_Collect.taskc.taskc_mutex, NULL);	//任务C锁
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//任务E锁

	_Collect.status.isend = 1;								//当前抄收结束

	pthread_mutex_init(&_Collect.mutex, NULL);

	BoradcastContorlInit();
}

/*
 * 函数功能:设置抄表器状态
 * */
void SetCollectorStatus(CollcetStatus *status)
{
	pthread_mutex_lock(&_Collect.mutex);

	memcpy(&_Collect.status, status, sizeof(CollcetStatus));

	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * 函数功能:获取抄表器当前任务状态
 * 参数：	status		返回抄表器任务状态
 * */
void GetCollectorStatus(CollcetStatus *status)
{
	pthread_mutex_lock(&_Collect.mutex);
	memcpy(status, &_Collect.status, sizeof(CollcetStatus));
	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * 函数功能:初始化广播管理器
 * */
void BoradcastContorlInit(void)
{
	memset(&_BoradContorl, 0, sizeof(BoradcastContorl));
	pthread_mutex_init(&_BoradContorl.mutex, NULL);
}

/*
 * 函数功能:获得广播全局倒计时
 * */
int GetBoradcastContorlTimer(void)
{
	int ret = 0;
	pthread_mutex_lock(&_BoradContorl.mutex);
	ret = _BoradContorl.timer;
	pthread_mutex_unlock(&_BoradContorl.mutex);
	return ret;
}

/*
 * 函数功能:设置广播全局倒计时时间
 * */
void SetBoradcastContorlTimer(int t)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.timer = t;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * 函数功能:广播全局倒计时递减
 * */
void BoradcastContorlTimerTicker(void)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.timer--;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * 函数功能:获取刷新广播许可倒计时
 * */
int GetBoradcastContorlAllowTimer(void)
{
	int ret = 0;
	pthread_mutex_lock(&_BoradContorl.mutex);
	ret = _BoradContorl.flush_allow_timer;
	pthread_mutex_unlock(&_BoradContorl.mutex);
	return ret;
}

/*
 * 函数功能:设置刷新广播许可倒计时
 * */
void SetBoradcastContorlAllowTimer(int t)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.flush_allow_timer = t;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * 函数功能:刷新广播许可倒计时递减
 * */
void BoradcastContorlAllowTimerTicker(void)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.flush_allow_timer--;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * 函数功能:获取广播许可次数，如果许可次数大于0，则减一
 * 返回值: 许可次数
 * */
int GetBoradcastContorlAllowNum(void)
{
	int ret = 0;
	pthread_mutex_lock(&_BoradContorl.mutex);
	ret = _BoradContorl.allow_num--;
	pthread_mutex_unlock(&_BoradContorl.mutex);
	return ret;
}

/*
 * 函数功能:设置广播许可次数
 * 参数:		num 	设置的次数
 * */
void SetBoradcastContorlAllowNum(int num)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.allow_num = num;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * 函数功能:获取抄表器任务倒计时
 * */
unsigned char GetCollectorStatusTimer(void)
{
	unsigned char ret = 0;
	pthread_mutex_lock(&_Collect.mutex);
	ret = _Collect.status.timer;
	pthread_mutex_unlock(&_Collect.mutex);
	return ret;
}

/*
 * 函数功能:抄表任务倒计时递减
 * */
void CollectorStatusTimerTicker(void)
{
	pthread_mutex_lock(&_Collect.mutex);
	_Collect.status.timer--;
	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * 函数功能:定时函数
 * */
void CountDown(int signl)
{
	if(_Collect.status.timer > 0)
	{
		_Collect.status.timer--;
	}
	if(_Collect.taskc.timer > 0)
	{
		_Collect.taskc.timer--;
	}
	if(_BoradContorl.timer > 0)
	{
		_BoradContorl.timer--;
	}
	if(_BoradContorl.flush_allow_timer > 0)
	{
		_BoradContorl.flush_allow_timer--;
	}
}

/*
 * 函数功能:抄表器线程
 * */
void *Collector(void *arg)
{
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//喂狗
//	close_watch_dog(wdt_id);

	printf("COLLECTOR WDT  ID  %d\n", wdt_id);

	struct itimerval tick;

	signal(SIGALRM, CountDown);
	memset(&tick, 0, sizeof(tick));

	//Timeout to run first time
	tick.it_value.tv_sec = 1;
	tick.it_value.tv_usec = 0;

	//After first, the Interval time for clock
	tick.it_interval.tv_sec = 1;
	tick.it_interval.tv_usec = 0;

	if(setitimer(ITIMER_REAL, &tick, NULL) < 0)
	{
		perror("Time Down erro:");
	}
	//初始化充值任务执行器
	TaskACollectInit();
	//创建 维护 充值任务队列线程
	pthread_t pt;
	pt = pthread_create(&pt, NULL, TaskATimer, NULL);
	if(pt < 0)
	{
		pthread_exit(NULL);
	}

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		usleep(2);
		//执行充值终端任务
		if(0 == TaskaIsEmpty())
		{
			SetTaskaKey(1);	//允许
			goto bbb;	//跳转至  周期抄表任务，防止终止太久周期抄表任务，集中器复位以太网模块
						//周期抄表任务，需判断，是否有充值，有充值任务时，只进行任务请求，不执行
		}
		SetTaskaKey(0);	//不允许

		//执行实时透传任务
		if(GetTaskeNum() > 0)
		{
			CollectTaskE();
			continue;
		}

		//执行实时抄表任务
		if(GetTaskbNum() > 0)
		{
			CollectTaskB();
			continue;
		}
bbb:
		//执行周期抄表任务
		if(GetTaskcKey() == 1)
		{
			CollectTaskC();
			continue;
		}

	}
	pthread_exit(NULL);
}

/*
 *函数功能:执行任务方式0
 *参数:	amm		表号
 *		inbuf	待发送数据
 *		len		待发送数据长度
 *返回值: -1 失败 0 成功
 * */
int ExecuteCollect0(unsigned char *amm, unsigned char *inbuf, int len)
{
	printf("*******ExecuteCollect0******\n");
	int ret = 0;
	tpFrame376_1 outbuf;
	tp3761Buffer snbuf;
	TerSocket *p;
	StandNode node;

	if(0 != _hld_ac.get_status())	//未授权时，不进行数据抄收
	{
		return 0;
	}

	ret = SeekAmmAddr(amm, AMM_ADDR_LEN);
	if(ret < 0)
	{
		return -1;
	}

	if(0 > GetStandNode(ret, &node))
	{
		return -1;
	}

	//构造透明转发帧结构
	Create3761AFN10_01(node.Ter, inbuf, len, 1, &outbuf);
	//将3761格式数据转换为可发送二进制数据
	DL3761_Protocol_LinkPack(&outbuf, &snbuf);
	//差找对应的套接字

	pthread_mutex_lock(&(route_mutex));
	p = AccordTerSeek(node.Ter);
	if(p != NULL)
	{
		pthread_mutex_lock(&(p->write_mutex));

		while(1)
		{
			if(0 == judge_seek_amm_task(p->Ter)) //如果该终端正在搜表，则不进行数据下发
				break;
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
				if(errno == SIGPIPE)	//等待回收
				{
					p->ticker = 0;
				}
				break;
			}
			snbuf.Len -= ret;
			if(0 == snbuf.Len)
			{
				break;
			}
		}
		pthread_mutex_unlock(&(p->write_mutex));
	}
	else
	{
		pthread_mutex_unlock(&(route_mutex));
		return -1;
	}
	pthread_mutex_unlock(&(route_mutex));
	return 0;
}

/*
 * 函数功能:执行任务方式1
 * 参数: amm		电表地址
 * 		inbuf	待发送的数据
 * 		len		待发送的数据长度
 * 	返回值: -1 失败 0 成功
 * */
int ExecuteCollect1(unsigned char *amm, unsigned char *inbuf, int len)
{
	printf("*******ExecuteCollect1******\n");
	if(NULL == _FristNode)
	{
		return -1;
	}

	if(0 != _hld_ac.get_status())	//未授权时，不进行数据抄收
	{
		return 0;
	}

	pthread_mutex_lock(&(route_mutex));
	TerSocket *p = _FristNode;
	tpFrame376_1 outbuf;
	tp3761Buffer snbuf;
	int ret = 0;
	while(NULL != p)
	{
		//构造透明转发帧结构
		Create3761AFN10_01(p->Ter, inbuf, len, 1, &outbuf);
		//将3761格式数据转换为可发送二进制数据
		DL3761_Protocol_LinkPack(&outbuf, &snbuf);

		pthread_mutex_lock(&(p->write_mutex));
		while(1)
		{
			if(0 == judge_seek_amm_task(p->Ter))	//如果该终端正在搜表，则不进行数据下发
				break;
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
				if(errno == SIGPIPE)	//等待回收
				{
					p->ticker = 0;
				}
				break;
			}
			snbuf.Len -= ret;
			if(0 == snbuf.Len)
			{
				break;
			}
		}
		pthread_mutex_unlock(&(p->write_mutex));
		p = p->next;
	}
	pthread_mutex_unlock(&(route_mutex));
//	sleep(EXE_COLLECT1);	//广播后，为了防止三网合一终端因阻塞，丢失数据，需要休眠
	return 0;
}

/*******************************************任务a的接口*********************************************/

/*
 * 函数功能:充值终端任务执行器初始化
 * 参数:
 * 返回值:
 * */
void TaskACollectInit(void)
{
	_Collect.taska.task_num = 0;							//任务总数量
	_Collect.taska.not_assigned_num = 0;					//没有分陪的节点个数
	_Collect.taska.key = 0;									//不允许充值任务执行
	_Collect.taska.node_head = NULL;
	_Collect.taska.node_tail = NULL;

	int i= 0;
	pthread_t pt[TASKA_THREAD_SIZE];
	for(i = 0; i < TASKA_THREAD_SIZE; i++)
	{
		pthread_create(&pt[i], NULL, CollectTaskA, NULL);
	}
}

/*
 * 函数功能:充值任务队列是否为空
 * 返回值: 0 不空 -1 空
 * */
int TaskaIsEmpty(void)
{
	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	if(_Collect.taska.node_head != NULL)
	{
		pthread_mutex_unlock(&_Collect.taska.taska_mutex);
		return 0;
	}
	else
	{
		pthread_mutex_unlock(&_Collect.taska.taska_mutex);
		return -1;
	}
}

/*
 * 函数功能:设置执行充值任务任务开关
 * */
void SetTaskaKey(int key)
{
	pthread_mutex_lock(&_Collect.taska.taska_mutex);

	_Collect.taska.key = key;

	pthread_mutex_unlock(&_Collect.taska.taska_mutex);
}

/*
 * 函数功能:获得充值任务开关状态
 * 返回值:开关状态 1开  其他关
 * */
int GetTaskaKey(void)
{
	int ret = 0;
	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	ret = _Collect.taska.key;
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);
	return ret;
}

/*
 * 函数功能:充值终端任务倒计时线程
 * */
void *TaskATimer(void *arg)
{
	struct task_a_node *node_p;
	struct task_a_node *node_temp;

	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&_Collect.taska.taska_mutex);

		while(_Collect.taska.node_head == NULL)
		{
			pthread_cond_wait(&_Collect.taska.node_not_empty, &_Collect.taska.taska_mutex);
		}

		node_p = _Collect.taska.node_head;
		while(NULL != node_p)
		{
			//更新节点下的任务执行倒计时
			if((node_p->task_status.is_end == 1) && node_p->task_status.timer > 0)
			{
				node_p->task_status.timer--;
			}

			//更新节点存在倒计时
			if(node_p->timer > 0)
			{
				node_p->timer--;
				node_p = node_p->next;
			}
			else
			{
				//删除节点 及任务
				node_temp = node_p;
				node_p = node_p->next;
				pthread_mutex_unlock(&_Collect.taska.taska_mutex);
				DeleTaskA(node_temp->amm, 1);
				pthread_mutex_lock(&_Collect.taska.taska_mutex);
			}
		}

		pthread_mutex_unlock(&_Collect.taska.taska_mutex);
	}
	pthread_exit(NULL);
}

/*
 * 函数功能:充值终端
 * */
void *CollectTaskA(void *arg)
{
	tpFrame645 f645;
	Buff645 b645;
	tpFrame376_1 snframe3761;
	tp3761Buffer ttpbuffer;
	struct topup_node *pp;
	int ret = 0;
	struct task_a_node *temp_node;
	unsigned char default_mark[TER_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char p_mark[TER_ADDR_LEN]; //线程标志  标志线程当前分配的终端 pthread mark
	memset(p_mark, 0xFF, TER_ADDR_LEN);
	while(1)
	{
		usleep(10);
		ret = CompareUcharArray(p_mark, default_mark, TER_ADDR_LEN);
		pthread_mutex_lock(&_Collect.taska.taska_mutex);
		while(ret == 1 && _Collect.taska.not_assigned_num == 0)
		{
			pthread_cond_wait(&_Collect.taska.need_assigned, &_Collect.taska.taska_mutex);
		}

		if(_Collect.taska.key != 1 && ret == 1)	//等待已久从队列中取出的其他等级任务执行结束，再执行充值任务
		{								//防止有其他等级的任务刚刚广播结束，这边又继续发送数据导致出错
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			continue;
		}


		if(ret != 1)	//有分配的终端  执行终端下的任务
		{
			//获取节点
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);	//调用的SeekTaskANode0函数里有加锁  解锁
			temp_node = SeekTaskANode0(p_mark);
			pthread_mutex_lock(&_Collect.taska.taska_mutex);
			if(NULL == temp_node)	//无该节点
			{
				printf("not found taska node\n");
				memset(p_mark, 0xFF, TER_ADDR_LEN);
			}
			else	//有该节点
			{
				if(temp_node->task_status.is_end == 0)	//任务结束  可以执行下一任务
				{
					//执行任务
					if(NULL != temp_node->task_head)	//有任务则执行 无任务则不执行
					{
						while(GetBoradcastContorlTimer() > 0)
						{
							sleep(1);
						}
						ret = ExecuteCollect0(temp_node->amm, temp_node->task_head->buf, temp_node->task_head->len);
						if(-1 == ret)	//任务执行失败 结束任务
						{

							if(GetBoradcastContorlAllowTimer() <= 0)
							{
								SetBoradcastContorlAllowTimer(60);	//设置刷新广播许可倒计时时间
								SetBoradcastContorlAllowNum(3);	//设置广播许可次数
							}
							if(GetBoradcastContorlAllowNum() > 0)
							{
								while(GetBoradcastContorlTimer() > 0)
								{
									sleep(1);
								}
								ret = ExecuteCollect1(temp_node->amm, temp_node->task_head->buf,temp_node->task_head->len);
								SetBoradcastContorlTimer(3);
							}
						}

						if(-1 == ret)
						{
							//返回找不到路径给充值终端
							memcpy(f645.Address, temp_node->amm, AMM_ADDR_LEN);
							f645.CtlField = 0xD1;
							f645.Datas[0] = 0xFE + 0x33;
							f645.Length = 1;
							if(0 == Create645From(&f645, &b645))
							{
								Create3761AFN10_01(temp_node->task_head->top_ter, b645.buf, b645.len, 0, &snframe3761);
								//将3761格式数据转换为可发送二进制数据
								DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
								pthread_mutex_lock(&(topup_node_mutex));
								pp = AccordTerFind(temp_node->task_head->top_ter);
								if(NULL != pp)
								{
									pthread_mutex_lock(&(pp->write_mutex));
									while(1)
									{
										ret = write(pp->s, ttpbuffer.Data, ttpbuffer.Len);
										if(ret < 0)
										{
											break;
										}
										ttpbuffer.Len -= ret;
										if(0 == ttpbuffer.Len)
											break;
									}
									pthread_mutex_unlock(&(pp->write_mutex));
								}
								pthread_mutex_unlock(&(topup_node_mutex));
							}
							pthread_mutex_unlock(&_Collect.taska.taska_mutex);
							DeleTaskA(temp_node->amm, 0);
							pthread_mutex_lock(&_Collect.taska.taska_mutex);
							temp_node->task_status.is_end = 0;
						}
						else
						{
							memcpy(temp_node->task_status.dadt, temp_node->task_head->dadt, DADT_LEN);
							memcpy(temp_node->task_status.top_ter, temp_node->task_head->top_ter, TER_ADDR_LEN);
							temp_node->task_status.is_end = 1;
							temp_node->task_status.timer = TASKA_TIMEOUT;
						}
					}
				}
				else	//任务未结束  判断是否超时  超时结束任务
				{
					if(temp_node->task_status.timer > 0)
					{
						//未超时
					}
					else	//超时
					{
						//删除任务
						pthread_mutex_unlock(&_Collect.taska.taska_mutex);
						DeleTaskA(temp_node->amm, 0);
						pthread_mutex_lock(&_Collect.taska.taska_mutex);
						temp_node->task_status.is_end = 0;
					}
				}
			}
		}
		else	//没有分配终端 分配终端
		{
			temp_node = _Collect.taska.node_head;
			while(temp_node != NULL)
			{
				if(0 == temp_node->is_allot)
				{
					memcpy(p_mark, temp_node->ter, TER_ADDR_LEN);
					temp_node->is_allot = 1;
					_Collect.taska.not_assigned_num--;
					break;
				}
				temp_node = temp_node->next;
			}
		}
		pthread_mutex_unlock(&_Collect.taska.taska_mutex);
	}

	pthread_exit(NULL);
}

/*
 * 函数功能:根据终端地址查看任务队列中的是否有该分支
 * 参数:	ter		终端地址
 * 返回值: 存在 分支的节点地址  不存在 NULL
 * */
struct task_a_node *SeekTaskANode0(unsigned char *ter)
{
	struct task_a_node *temp = NULL;

	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	temp = _Collect.taska.node_head;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(temp->ter, ter, TER_ADDR_LEN))
		{
			break;
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);

	return temp;
}

/*
 * 函数功能:根据电表地址查看任务队列中是否有该分支
 * 参数:	amm		电表地址
 * 返回值:	存在  分支节点地址		不存在  NULL
 * */
struct task_a_node *SeekTaskANode1(unsigned char *amm)
{
	struct task_a_node *temp = NULL;

	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	temp = _Collect.taska.node_head;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(temp->amm, amm, AMM_ADDR_LEN))
		{
			break;
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);

	return temp;
}

/*
 * 函数功能:添加任务A
 * 参数:	amm		电表号
 * 		dadt	数据标识
 * 		buf		数据内容
 * 		len		数据长度
 * 		ter		充值终端地址
 * 返回值 0 成功 -1无该表  -2 被独占  -3
 * */
int AddTaskA(unsigned char *amm, unsigned char *dadt, unsigned char *buf, int len, unsigned char *ter)
{
	int ret= 0;
	struct task_a_node * p;
	struct task_a_content *new_task;
	//获取任务所要下发的终端地址
	StandNode node;
	ret = SeekAmmAddr(amm, AMM_ADDR_LEN);
	if(ret < 0)
	{
		return -1;
	}

	if(0 > GetStandNode(ret, &node))
	{
		return -1;
	}

	//判断是否需要添加新的分任务支
	//查看任务队列中有没有相同的终端地址
	p = SeekTaskANode0(node.Ter);
	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	if(NULL == p)	//添加新节点
	{
		printf("add task a new node\n");
		//申请节点  填充任务节点
		struct task_a_node *new_node = (struct task_a_node *)malloc(sizeof(struct task_a_node));
		if(NULL == new_node)
		{
			perror("malloc task a new node:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -3;
		}
		memset(new_node, 0, sizeof(struct task_a_node));
		//终端地址
		memcpy(new_node->ter, node.Ter, TER_ADDR_LEN);
		//电表地址
		memcpy(new_node->amm, amm, AMM_ADDR_LEN);
		//未分配线程处理
		new_node->is_allot = 0;
		//节点存在倒计时 1min
		new_node->timer = TASKA_NODE_LIVE_TIMER;
		//任务执行状态
		memset(&new_node->task_status, 0, sizeof(struct task_a_status));
		//头任务
		new_node->task_head = (struct task_a_content *)malloc(sizeof(struct task_a_content));
		if(NULL == new_node->task_head)
		{
			perror("malloc task new node content:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -3;
		}
		memset(new_node->task_head, 0, sizeof(struct task_a_content));
		memcpy(new_node->task_head->top_ter, ter, TER_ADDR_LEN);
		memcpy(new_node->task_head->buf, buf, len);
		memcpy(new_node->task_head->dadt, dadt, DADT_LEN);
		new_node->task_head->len = len;
		new_node->task_head->next = NULL;

		//尾任务
		new_node->task_tail = new_node->task_head;
		//下一个节点
		new_node->next = NULL;

		//将节点加入任务队列
		if(NULL == _Collect.taska.node_head)	//第一次添加节点
		{
			_Collect.taska.node_head = new_node;
			_Collect.taska.node_tail = _Collect.taska.node_head;
			pthread_cond_broadcast(&(_Collect.taska.node_not_empty));
		}
		else	//非第一次添加节点
		{
			_Collect.taska.node_tail->next = new_node;
			_Collect.taska.node_tail = _Collect.taska.node_tail->next;
		}
	}
	else			//在以有的节点上添加任务
	{
		printf("add task a new task\n");

		//判断终端是否被其他电表独占
		if(1 != CompareUcharArray(p->amm, amm, AMM_ADDR_LEN))
		{
			//终端已经被独占
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -2;
		}

		//添加新任务
		new_task = (struct task_a_content *)malloc(sizeof(struct task_a_content));
		if(NULL == new_task)
		{
			perror("malloc new task a:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -3;
		}
		memset(new_task, 0, sizeof(struct task_a_content));
		memcpy(new_task->top_ter, ter, TER_ADDR_LEN);
		memcpy(new_task->buf, buf, len);
		new_task->len = len;
		memcpy(new_task->dadt, dadt, DADT_LEN);
		new_task->next = NULL;

		//
		if(NULL == p->task_head)
		{
			p->task_head = new_task;
			p->task_tail = p->task_head;
		}
		else
		{
			p->task_tail->next = new_task;
			p->task_tail = p->task_tail->next;
		}
	}

	//任务总数量
	_Collect.taska.task_num++;
	if(p == NULL)
	{
		//未分配的终端数量
		_Collect.taska.not_assigned_num++;
		if(1 == _Collect.taska.not_assigned_num)
		{
			pthread_cond_broadcast(&(_Collect.taska.need_assigned));
		}
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);
	return 0;
}

/*
 * 函数功能:根据电表删除充值任务
 * 参数:	amm		电表
 * 		flag	删除方式  0  删除电表最老的任务
 * 						1  删除电表的所有任务
 * 返回值: 0成功  -1 失败
 * */
int DeleTaskA(unsigned char *amm, unsigned char flag)
{
	struct task_a_node * p = NULL;
	struct task_a_content *temp = NULL;
	p = SeekTaskANode1(amm);
	if(NULL == p)
	{
		return -1;
	}

	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	switch(flag)
	{
		case 0:		//删除对应电表最老的任务
			if(p->task_head != NULL)	//有任务
			{
				//删除任务
				temp = p->task_head;
				p->task_head = p->task_head->next;
				if(NULL == p->task_head)
					p->task_tail = NULL;
				free(temp);
				temp = NULL;
				_Collect.taska.task_num--;
				memset(&(p->task_status), 0, sizeof(struct task_a_status));
//				printf("dele task a \n");
			}
			break;
		case 1:		//删除对应电表的全部任务,且删除节点
//			printf("dele task a node\n");
			//删除节点上的任务
			while(p->task_head != NULL)
			{
				temp = p->task_head;
				p->task_head = p->task_head->next;
				free(temp);
				temp = NULL;
				_Collect.taska.task_num--;
			}

			//删除节点
			struct task_a_node *last_p;	//待删除节点的上一个节点
			if(p == _Collect.taska.node_head)	//删除的节点为第一个节点
			{
				if(0 == p->is_allot)	//还未分配
				{
					_Collect.taska.not_assigned_num--;
				}
				_Collect.taska.node_head = p->next;
				free(p);
				if(NULL == _Collect.taska.node_head)
				{
					_Collect.taska.node_tail = NULL;
				}
			}
			else		//删除的节点为非第一个节点
			{
				//获取待删除节点的上一个节点
				last_p = _Collect.taska.node_head;
				while(last_p->next != NULL)
				{
					if(last_p->next == p)
					{
						break;
					}
					last_p = last_p->next;
				}
				last_p->next = p->next;
				if(p == _Collect.taska.node_tail)
				{
					_Collect.taska.node_tail = last_p;
				}
				if(0 == p->is_allot)	//还未分配
				{
					_Collect.taska.not_assigned_num--;
				}
				free(p);
			}
//			if(_Collect.taska.node_head == NULL)
//				printf("task node is empty\n");

			break;
		default:
			break;
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);

	return 0;
}

/******************************************************任务B*************************************************/
/*
 * 函数功能:获取任务B数量
 * 返回值: >=0数量  <0错误
 * */
int GetTaskbNum(void)
{
	int num = 0;
	pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
	num = _Collect.taskb.num;
	pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
	return num;
}

/*
 * 函数功能:实时采集
 * */
void CollectTaskB(void)
{
	unsigned char buf[300] = {0};
	int len = 0;
	tpFrame376_2 snframe3762;
	tp3762Buffer tpbuffer;
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);
	//本次任务是否结束
	if(c_status.isend == 0)	//未结束
	{
		if(c_status.timer == 0)	//抄表超时
		{
			if(c_status.conut == 0)	//第一次超时
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'b';
				memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				//执行任务
				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);

				if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
				{
					while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
					{
						sleep(1);
					}

					//抄表状态  在抄表任务下发之前更新，防止在更新抄表状态时，数据已经收到
					c_status.conut = 3;
					c_status.timer = NET_COLLECT_TIMER + 2;
					SetCollectorStatus(&c_status);

					ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
					SetBoradcastContorlTimer(3);
				}
			}
			else if(c_status.conut == 1)	//第二次超时
			{
				c_status.conut++;
				c_status.runingtask = 'b';
				memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}
				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
				SetBoradcastContorlTimer(3);
			}
			else //结束当前任务  执行下一次任务
			{
				len = 0;
				Create3762AFN13_01(c_status.amm, _Collect.taskb.next->type, buf, len, &snframe3762);
				if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
				{
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
					pthread_mutex_unlock(&writelock);

					c_status.isend = 1;
					memset(c_status.amm, 0, AMM_ADDR_LEN);
					memset(c_status.dadt, 0, DADT_LEN);
					c_status.conut = 0;
					c_status.runingtask = 0;
					SetCollectorStatus(&c_status);

//					pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
					DeleNearTaskB();
//					pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
				}
			}
		}
	}
	else						//结束
	{
		if(GetTaskbNum() > 0)
		{
			c_status.isend = 0;
			c_status.conut = 0;
			c_status.runingtask = 'b';
			memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
			//执行任务
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);
			if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
			{
				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				c_status.conut = 3;
				SetCollectorStatus(&c_status);

				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
				SetBoradcastContorlTimer(3);
			}
		}
	}
}

/*
 * 函数功能:添加任务B
 * 参数:	b	任务数据
 * 返回值 0 成功 -1失败
 * */
int AddTashB(struct task_b *b)
{
	struct task_b *newp = NULL;
	struct task_b *p = NULL;
	struct task_b *temp = NULL;
	CollcetStatus c_status;
	GetCollectorStatus(&c_status);

	if(NULL == b)
	{
		return -1;
	}
	pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
	if(NULL == _Collect.taskb.next)	//第一次添加
	{
		newp = (struct task_b *)malloc(sizeof(struct task_b));
		if(NULL == newp)
		{
			pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
			return -1;
		}
		memcpy(newp, b, sizeof(struct task_b));
		_Collect.taskb.next = newp;
		_Collect.taskb.num = 1;
		pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);

		if(c_status.runingtask == 'c')
		{
			memset(&c_status, 0, sizeof(CollcetStatus));
			c_status.isend = 1;
			SetCollectorStatus(&c_status);
		}
		return 0;
	}
	if(TASK_MAX_NUM == _Collect.taskb.num)
	{
		pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
		return -1;
	}

	//遍历到任务尾
	p = _Collect.taskb.next;
	while(p != NULL)
	{
		temp = p;
		p = p->next;
	}

	newp = (struct task_b *)malloc(sizeof(struct task_b));
	if(NULL == newp)
	{
		pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
		return -1;
	}
	memcpy(newp, b, sizeof(struct task_b));
	temp->next = newp;
	_Collect.taskb.num += 1;
	pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
	if(c_status.runingtask == 'c')
	{
		memset(&c_status, 0, sizeof(CollcetStatus));
		c_status.isend = 1;
		SetCollectorStatus(&c_status);
	}
	return 0;
}

/*
 * 函数功能:删除当前任务b
 * 注:删除最近的任务
 * */
void DeleNearTaskB(void)
{
	struct task_b *b_p;
	pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
	if(_Collect.taskb.num <= 0)
	{
		pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
		return;
	}
	//提取下一个任务，删除当前任务
	_Collect.taskb.num--;
	b_p = _Collect.taskb.next;
	_Collect.taskb.next = b_p->next;
	//删除上一个任务
	if(b_p != NULL)
	{
		free(b_p);
		b_p = NULL;
	}
	pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
}


/******************************************************任务C***********************************************/
/*
 * 函数功能:执行周期抄表重新抄表任务
 * */
void TaskcReset(void)
{
	pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
	_Collect.taskc.key = 1;
	_Collect.taskc.count = 3;
	_Collect.taskc.isok = 0;
	_Collect.taskc.index = 0;
	_Collect.taskc.timer = 0;
	memset(_Collect.taskc.buf, 0 ,TASK_MAX_LEN);
	memset(_Collect.taskc.dadt, 0, DADT_LEN);
	pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
}

/*
 * 函数功能:获取周期抄表开关状态
 * 返回值: 开关状态
 * */
int GetTaskcKey(void)
{
	int key = 0;
	pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
	key = _Collect.taskc.key;
	pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
	return key;
}

/*
 * 函数功能:获取请求任务倒计时
 * 返回值:倒计时时间
 * */
int GetRequestTimer(void)
{
	int ret = 0;
	pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
	ret = _Collect.taskc.timer;
	pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
	return ret;
}

/*
 * 函数功能:请求任务倒计时递减
 * */
void RequestTimerTicker(void)
{
	pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
	_Collect.taskc.timer--;
	pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
}

/*
 * 函数功能:执行任务C 周期采集任务
 * */
void CollectTaskC(void)
{
	tpFrame376_2 snframe3762;
	tp3762Buffer buffer;
	StandNode node;
	GetStandNode(_Collect.taskc.index, &node);
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);

	//本次抄收的数据标识是否结束
	if(c_status.isend == 0)	//没有结束
	{
		if(c_status.timer == 0)	//抄表超时
		{

			if(c_status.conut == 0)	//第一次超时 采用执行任务方式1
			{
				c_status.isend = 0;
//				_Collect.flag = 0;
				c_status.conut++;
				c_status.runingtask = 'c';
				memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

				//执行任务
				while(GetBoradcastContorlTimer() > 0)
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);
				//执行任务
				if(0 == GetTaskaKey())
				{
					if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
					{
						while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
						{
							sleep(1);
						}
						c_status.timer = NET_COLLECT_TIMER + 2;
						c_status.conut = 3;
						SetCollectorStatus(&c_status);

						ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
						SetBoradcastContorlTimer(3);
					}
				}
			}
			else if(c_status.conut == 1)	//第二次失败 采用执行任务方式2
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'c';
				memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);

				//执行任务
				if(0 == GetTaskaKey())
				{
					ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
					SetBoradcastContorlTimer(3);
				}
			}
			else	//结束当前任务  执行下一次任务
			{
				c_status.isend = 1;
				c_status.timer = 0;
				c_status.conut = 0;
				memset(c_status.amm, 0, AMM_ADDR_LEN);
				memset(c_status.dadt, 0, DADT_LEN);
				SetCollectorStatus(&c_status);

				pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
				_Collect.taskc.count = 0;
				_Collect.taskc.index++;
				_Collect.taskc.index %= GetStandNodeNum();
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 0;
				_Collect.taskc.len = 0;
				memset(_Collect.taskc.buf, 0, TASK_MAX_LEN);
				memset(_Collect.taskc.dadt, 0, DADT_LEN);
				pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
			}
		}
	}
	else	//执行一下个任务
	{
		pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
		if(_Collect.taskc.isok == 1)	//请求任务成功	执行任务
		{
			c_status.isend = 0;
			c_status.conut = 0;
			c_status.runingtask = 'c';
			memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

			//执行任务
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);

			if(0 == GetTaskaKey())	//充值任务执行时，不进行周期任务，但可以请求
			{
				if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
					{
						sleep(1);
					}
					c_status.timer = NET_COLLECT_TIMER + 2;
					c_status.conut = 3;
					SetCollectorStatus(&c_status);

					ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
					SetBoradcastContorlTimer(3);
				}
			}

			_Collect.taskc.isok = 0;
		}
		else if(_Collect.taskc.timer == 0)	//请求任务超时
		{
			if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
			{
				pthread_mutex_lock(&_RunPara.mutex);
				if(node.cyFlag == _RunPara.CyFlag)
				{
					pthread_mutex_unlock(&_RunPara.mutex);
					_Collect.taskc.index++;
					_Collect.taskc.index %= GetStandNodeNum();
					pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
					return;
				}
				pthread_mutex_unlock(&_RunPara.mutex);
			}
			if(_Collect.taskc.count < 2)
			{
				_Collect.taskc.count++;
				if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
				{
					CreatAFN14_01up(node.Amm, &snframe3762);
					DL3762_Protocol_LinkPack(&snframe3762, &buffer);

					//发送
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, buffer.Data, buffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.taskc.timer = AFN14_01_TIMER;
					_Collect.taskc.isok = 0;
				}
			}
			else if(_Collect.taskc.count == 2)	//请求任务失败2次 则切换下一只表请求任务
			{
				_Collect.taskc.index++;
				_Collect.taskc.index %= GetStandNodeNum();
				if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
				{
					CreatAFN14_01up(node.Amm, &snframe3762);
					DL3762_Protocol_LinkPack(&snframe3762, &buffer);

					//发送
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, buffer.Data, buffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.taskc.timer = AFN14_01_TIMER;
					_Collect.taskc.count = 1;
					_Collect.taskc.isok = 0;
				}
			}
			else if(_Collect.taskc.count >= 3)	//第一次请求任务
			{
				//构造发送帧
				_Collect.taskc.index = 0;
				if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
				{
					CreatAFN14_01up(node.Amm, &snframe3762);
					DL3762_Protocol_LinkPack(&snframe3762, &buffer);

					//发送
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, buffer.Data, buffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.taskc.timer = AFN14_01_TIMER;
					_Collect.taskc.count = 1;
					_Collect.taskc.isok = 0;
				}
			}
		}
		pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
	}
}

/*************************************************任务E***************************************************/

/*
 * 函数功能:获取透明转发任务个数
 * 返回值:>=0 数量 <0错误
 * */
int GetTaskeNum(void)
{
	int num = 0;
	pthread_mutex_lock(&_Collect.taske.taske_mutex);
	num = _Collect.taske.num;
	pthread_mutex_unlock(&_Collect.taske.taske_mutex);
	return num;
}

/*
 * 函数功能:透明转发
 * */
void CollectTaskE(void)
{
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);
	//本次任务是否结束
	if(c_status.isend == 0)	//未结束
	{
		if(c_status.timer == 0)	//抄表超时
		{
			if(c_status.conut == 0)	//第一次超时
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'e';
				memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
				//执行任务
				while(GetBoradcastContorlTimer() > 0)
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);
				if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
				{
					while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
					{
						sleep(1);
					}
					c_status.timer = NET_COLLECT_TIMER + 2;
					c_status.conut = 3;	//执行完广播后  如果超时 直接结束任务
					SetCollectorStatus(&c_status);

					ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf,  _Collect.taske.next->len);
					SetBoradcastContorlTimer(3);
				}
			}
			else if(c_status.conut == 1)	//第二次超时
			{
				c_status.conut++;
				c_status.runingtask = 'e';
				memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf,  _Collect.taske.next->len);
				SetBoradcastContorlTimer(3);
			}
			else //结束当前任务  执行下一次任务
			{
				c_status.isend = 1;
				c_status.timer = 0;
//				_Collect.flag = 0;
				c_status.conut = 0;
				memset(c_status.amm, 0, AMM_ADDR_LEN);
				memset(c_status.dadt, 0, DADT_LEN);
				c_status.runingtask = 0;
				SetCollectorStatus(&c_status);
				pthread_mutex_lock(&_Collect.taske.taske_mutex);
				DeleNearTaskE();
				pthread_mutex_unlock(&_Collect.taske.taske_mutex);
			}
		}
	}
	else						//结束
	{
		if(GetTaskeNum() > 0)
		{
			c_status.isend = 0;
//			_Collect.flag = 0;
			c_status.conut = 0;
			c_status.runingtask = 'e';
			memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
			//执行任务
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);
			if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
			{
				while(GetBoradcastContorlTimer() > 0)	//广播抄表后需延时一段时间，进行通信
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				c_status.conut = 3;
				SetCollectorStatus(&c_status);

				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf,  _Collect.taske.next->len);
				SetBoradcastContorlTimer(3);
			}
		}
	}
}

/*
 * 函数功能:添加任务E
 * 参数:	e	任务数据
 * 返回值 0 成功 -1失败
 * */
int AddTashE(struct task_e *e)
{
	struct task_e *newp = NULL;
	struct task_e *p = NULL;
	struct task_e *temp = NULL;
	CollcetStatus c_status;
	GetCollectorStatus(&c_status);

	if(NULL == e)
	{
		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
		return -1;
	}
	pthread_mutex_lock(&_Collect.taske.taske_mutex);
	if(NULL == _Collect.taske.next)	//第一次添加
	{
		newp = (struct task_e *)malloc(sizeof(struct task_e));
		if(NULL == newp)
		{
			pthread_mutex_unlock(&_Collect.taske.taske_mutex);
			return -1;
		}
		memcpy(newp, e, sizeof(struct task_e));
		_Collect.taske.next = newp;
		_Collect.taske.num = 1;

		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
		if(c_status.runingtask == 'c' || c_status.runingtask == 'b')
		{
			memset(&c_status, 0, sizeof(CollcetStatus));
			c_status.isend = 1;
			SetCollectorStatus(&c_status);
		}
		return 0;
	}
	if(TASK_MAX_NUM == _Collect.taske.num)
	{
		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
		return -1;
	}
	//遍历到任务尾
	p = _Collect.taske.next;
	while(p != NULL)
	{
		temp = p;
		p = p->next;
	}

	newp = (struct task_e *)malloc(sizeof(struct task_e));
	if(NULL == newp)
	{
		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
		return -1;
	}
	memcpy(newp, e, sizeof(struct task_e));
	temp->next = newp;
	_Collect.taske.num += 1;

	pthread_mutex_unlock(&_Collect.taske.taske_mutex);

	if(c_status.runingtask == 'c' || c_status.runingtask == 'b')
	{
		memset(&c_status, 0, sizeof(CollcetStatus));
		c_status.isend = 1;
		SetCollectorStatus(&c_status);
	}
	return 0;
}

/*
 * 函数功能:删除当前任务e
 * 注:删除最近的任务
 * */
void DeleNearTaskE(void)
{
	pthread_mutex_lock(&_Collect.taske.taske_mutex);
	if(_Collect.taske.num <= 0)
	{
		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
		return;
	}
	struct task_e *e_p;
	//提取下一个任务，删除当前任务
	_Collect.taske.num--;
	e_p = _Collect.taske.next;
	_Collect.taske.next = e_p->next;
	//删除上一个任务
	if(e_p != NULL)
	{
		free(e_p);
		e_p = NULL;
	}
	pthread_mutex_unlock(&_Collect.taske.taske_mutex);
}


/****************************************************************************************************/
