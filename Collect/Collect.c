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
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//任务B锁

	_Collect.b_isend = 1;	//当前抄收结束
	_Collect.c_isend = 1;	//当前抄收结束
//	_Collect.d_isend = 1;	//当前抄收结束
}

/*
 * 函数功能:执行周期抄表重新抄表任务
 * */
void TaskcReset(void)
{
	_Collect.taskc.key = 1;
	_Collect.taskc.count = 3;
	_Collect.taskc.isok = 0;
	_Collect.taskc.index = 0;
	_Collect.taskc.timer = 0;
	memset(_Collect.taskc.buf, 0 ,TASK_MAX_LEN);
	memset(_Collect.taskc.dadt, 0, DADT_LEN);
}

/*
 * 函数功能:定时函数
 * */
void CountDown(int signl)
{
	if(_Collect.timer > 0)
	{
		_Collect.timer--;
	}
	if(_Collect.taskc.timer > 0)
	{
		_Collect.taskc.timer--;
	}
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
	int ret = 0;
	struct task_a_node *temp_node;
	unsigned char default_mark[TER_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char p_mark[TER_ADDR_LEN]; //线程标志  标志线程当前分配的终端 pthread mark
	memset(p_mark, 0xFF, TER_ADDR_LEN);
	while(1)
	{
		ret = CompareUcharArray(p_mark, default_mark, TER_ADDR_LEN);
		pthread_mutex_lock(&_Collect.taska.taska_mutex);
		while(ret == 1 && _Collect.taska.not_assigned_num == 0)
		{
			pthread_cond_wait(&_Collect.taska.need_assigned, &_Collect.taska.taska_mutex);
		}

		while(_Collect.taska.key != 1 && ret == 1)	//等待已久从队列中取出的其他等级任务执行结束，再执行充值任务
		{								//防止有其他等级的任务刚刚广播结束，这边又继续发送数据导致出错
			usleep(1);
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
						ret = ExecuteCollect0(temp_node->amm, temp_node->task_head->buf, temp_node->task_head->len);
						if(-1 == ret)	//任务执行失败 结束任务
						{
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
 * 函数功能:抄表器线程
 * */
void *Collector(void *arg)
{
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//喂狗
	close_watch_dog(wdt_id);

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
		if(_Collect.taska.node_head != NULL)
		{
			_Collect.taska.key = 1;	//允许
			if('a' != _Collect.runingtask)
				_Collect.runingtask = 'a';
			continue;
		}
		_Collect.taska.key = 0;	//不允许

		//执行实时透传任务
		if(_Collect.taske.num != 0)
		{
			CollectTaskE();
			continue;
		}

		//执行实时抄表任务
		if(_Collect.taskb.num != 0)
		{
			CollectTaskB();
			continue;
		}

		//执行周期抄表任务
		if(_Collect.taskc.key == 1)
		{
			CollectTaskC();
			continue;
		}

		//执行路径发现任务
//		if(_Collect.taskd.key == 1)
//		{
//			continue;
//		}
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
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
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
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
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
	sleep(EXE_COLLECT1);	//广播后，为了防止三网合一终端因阻塞，丢失数据，需要休眠
	return 0;
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

	//本次任务是否结束
	if(_Collect.b_isend == 0)	//未结束
	{
		if(_Collect.timer == 0)	//抄表超时
		{
			if(_Collect.conut == 0)	//第一次超时
			{
				_Collect.b_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'b';
				memcpy(_Collect.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				//执行任务
				if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
				{
					ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
				}
			}
			else if(_Collect.conut == 1)	//第二次超时
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'b';
				memcpy(_Collect.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
			}
			else //结束当前任务  执行下一次任务
			{
//				_Collect.b_isend = 1;
//				_Collect.timer = 0;
////				_Collect.flag = 0;
//				_Collect.conut = 0;
//				memset(_Collect.amm, 0, AMM_ADDR_LEN);
//				memset(_Collect.dadt, 0, DADT_LEN);
//				_Collect.runingtask = 0;
//				pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
//				DeleNearTaskB();
//				pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
				//构造376.2 AFN13上报数据
				len = 0;
				Create3762AFN13_01(_Collect.amm, _Collect.taskb.next->type, buf, len, &snframe3762);
				if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
				{
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.b_isend = 1;
					memset(_Collect.amm, 0, AMM_ADDR_LEN);
					memset(_Collect.dadt, 0, DADT_LEN);
					_Collect.conut = 0;
//								_Collect.flag = 0;
					_Collect.runingtask = 0;
					pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
					DeleNearTaskB();
					pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
				}
			}
		}
	}
	else						//结束
	{
		if(_Collect.taskb.num > 0)
		{
			_Collect.b_isend = 0;
			_Collect.timer = NET_COLLECT_TIMER;
//			_Collect.flag = 0;
			_Collect.conut = 0;
			_Collect.runingtask = 'b';
			memcpy(_Collect.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taskb.next->dadt, DADT_LEN);
			//执行任务
			if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
			{
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
			}
		}
	}
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

	//本次抄收的数据标识是否结束
	if(_Collect.c_isend == 0)	//没有结束
	{
		if(_Collect.timer == 0)	//抄表超时
		{

			if(_Collect.conut == 0)	//第一次超时 采用执行任务方式1
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//执行任务
				if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
				}
			}
			else if(_Collect.conut == 1)	//第二次失败 采用执行任务方式2
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//执行任务
				ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
			else	//结束当前任务  执行下一次任务
			{
				_Collect.c_isend = 1;
				_Collect.timer = 0;
//				_Collect.flag = 0;
				_Collect.conut = 0;
				memset(_Collect.amm, 0, AMM_ADDR_LEN);
				memset(_Collect.dadt, 0, DADT_LEN);
				_Collect.taskc.count = 0;
				_Collect.taskc.index++;
				_Collect.taskc.index %= _StandNodeNum;
				_Collect.taskc.isok = 0;
				_Collect.taskc.timer = 0;
				_Collect.taskc.len = 0;
				memset(_Collect.taskc.buf, 0, TASK_MAX_LEN);
				memset(_Collect.taskc.dadt, 0, DADT_LEN);
			}
		}
	}
	else	//执行一下个任务
	{
		if(_Collect.taskc.isok == 1)	//请求任务成功	执行任务
		{
			_Collect.c_isend = 0;
			_Collect.timer = NET_COLLECT_TIMER;
//			_Collect.flag = 0;
			_Collect.conut = 0;
			_Collect.runingtask = 'c';
			memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

			//执行任务
			if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
			{
				ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
		}
		else if(_Collect.taskc.timer == 0)	//请求任务超时
		{
			if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
			{
				if(node.cyFlag == _RunPara.CyFlag)
				{
					_Collect.taskc.index++;
					_Collect.taskc.index %= _StandNodeNum;
//					printf("num  %d  index  %d\n",_StandNodeNum,_Collect.taskc.index);
					return;
				}
			}
			if(_Collect.taskc.count < 2)
			{
				_Collect.taskc.count++;
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
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
				_Collect.taskc.index %= _StandNodeNum;
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
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
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
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
	}
}

/*
 * 函数功能:透明转发
 * */
void CollectTaskE(void)
{
	//本次任务是否结束
	if(_Collect.e_isend == 0)	//未结束
	{
		if(_Collect.timer == 0)	//抄表超时
		{
			if(_Collect.conut == 0)	//第一次超时
			{
				_Collect.e_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'e';
				memcpy(_Collect.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taske.next->dadt, DADT_LEN);
				//执行任务
				if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
				{
					ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
				}
			}
			else if(_Collect.conut == 1)	//第二次超时
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'e';
				memcpy(_Collect.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taske.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
			}
			else //结束当前任务  执行下一次任务
			{
				_Collect.e_isend = 1;
				_Collect.timer = 0;
//				_Collect.flag = 0;
				_Collect.conut = 0;
				memset(_Collect.amm, 0, AMM_ADDR_LEN);
				memset(_Collect.dadt, 0, DADT_LEN);
				_Collect.runingtask = 0;
				pthread_mutex_lock(&_Collect.taske.taske_mutex);
				DeleNearTaskE();
				pthread_mutex_unlock(&_Collect.taske.taske_mutex);
			}
		}
	}
	else						//结束
	{
		if(_Collect.taske.num > 0)
		{
			_Collect.e_isend = 0;
			_Collect.timer = NET_COLLECT_TIMER;
//			_Collect.flag = 0;
			_Collect.conut = 0;
			_Collect.runingtask = 'e';
			memcpy(_Collect.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taske.next->dadt, DADT_LEN);
			//执行任务
			if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
			{
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
			}
		}
	}
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
 * 返回值 0 成功 -1失败
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
//		printf("add task a new node\n");
		//申请节点  填充任务节点
		struct task_a_node *new_node = (struct task_a_node *)malloc(sizeof(struct task_a_node));
		if(NULL == new_node)
		{
			perror("malloc task a new node:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -1;
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
			return -1;
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
//		printf("add task a new task\n");

		//判断终端是否被其他电表独占
		if(1 != CompareUcharArray(p->amm, amm, AMM_ADDR_LEN))
		{
			//终端已经被独占
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -1;
		}

		//添加新任务
		new_task = (struct task_a_content *)malloc(sizeof(struct task_a_content));
		if(NULL == new_task)
		{
			perror("malloc new task a:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -1;
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
				printf("dele task a \n");
			}
			break;
		case 1:		//删除对应电表的全部任务,且删除节点
			printf("dele task a node\n");
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
			if(_Collect.taska.node_head == NULL)
				printf("task node is empty\n");

			break;
		default:
			break;
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);

	return 0;
}

/****************************************************************************************************/
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

	if(NULL == b)
	{
		return -1;
	}
	if(NULL == _Collect.taskb.next)	//第一次添加
	{
		newp = (struct task_b *)malloc(sizeof(struct task_b));
		if(NULL == newp)
		{
			return -1;
		}
		memcpy(newp, b, sizeof(struct task_b));
		_Collect.taskb.next = newp;
		_Collect.taskb.num = 1;
		return 0;
	}
	if(TASK_MAX_NUM == _Collect.taskb.num)
	{
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
		return -1;
	}
	memcpy(newp, b, sizeof(struct task_b));
	temp->next = newp;
	_Collect.taskb.num += 1;
	return 0;
}

/*
 * 函数功能:删除当前任务b
 * 注:删除最近的任务
 * */
void DeleNearTaskB(void)
{
	struct task_b *b_p;
	if(_Collect.taskb.num <= 0)
	{
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

	if(NULL == e)
	{
		return -1;
	}

	if(NULL == _Collect.taske.next)	//第一次添加
	{
		newp = (struct task_e *)malloc(sizeof(struct task_e));
		if(NULL == newp)
		{
			return -1;
		}
		memcpy(newp, e, sizeof(struct task_e));
		_Collect.taske.next = newp;
		_Collect.taske.num = 1;
		return 0;
	}
	if(TASK_MAX_NUM == _Collect.taske.num)
	{
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
		return -1;
	}
	memcpy(newp, e, sizeof(struct task_e));
	temp->next = newp;
	_Collect.taske.num += 1;
	return 0;
}

/*
 * 函数功能:删除当前任务e
 * 注:删除最近的任务
 * */
void DeleNearTaskE(void)
{
	if(_Collect.taske.num <= 0)
	{
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
}


