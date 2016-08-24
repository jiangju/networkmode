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
#include "HLDWatchDog.h"
/*
 * 函数功能:初始化抄表器
 * */
void CollectorInit(void)
{
	memset(&_Collect, 0, sizeof(Collcet));

	pthread_mutex_init(&_Collect.taska.taska_mutex, NULL);	//任务A锁
	pthread_mutex_init(&_Collect.taskb.taskb_mutex, NULL);	//任务B锁
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//任务B锁
	_Collect.a_isend = 1;	//当前抄收结束
	_Collect.b_isend = 1;	//当前抄收结束
	_Collect.c_isend = 1;	//当前抄收结束
	_Collect.d_isend = 1;	//当前抄收结束
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

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		usleep(2);
		//执行充值终端任务
		if(_Collect.taska.num != 0)
		{
			continue;
		}

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
		if(_Collect.taskd.key == 1)
		{
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
	ret = SeekAmmAddr(amm, AMM_ADDR_LEN);
	if(ret < 0)
	{
		return -1;
	}
	//构造透明转发帧结构
	Create3761AFN10_01(_SortNode[ret]->Ter, inbuf, len, 1, &outbuf);
	//将3761格式数据转换为可发送二进制数据
	DL3761_Protocol_LinkPack(&outbuf, &snbuf);
	//差找对应的套接字
	pthread_mutex_lock(&(route_mutex));
	p = AccordTerSeek(_SortNode[ret]->Ter);
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

		pthread_mutex_lock(&(route_mutex));
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
		pthread_mutex_unlock(&(route_mutex));

		p = p->next;
	}

	sleep(EXE_COLLECT1);	//广播后，为了防止三网合一终端因阻塞，丢失数据，需要休眠
	return 0;
}

/*
 * 函数功能:充值终端
 * */
void CollectTaskA(void)
{
	//本次任务是否结束
	if(_Collect.a_isend == 0)	//未结束
	{
		if(_Collect.timer == 0)	//抄表超时
		{
			if(_Collect.conut == 0)	//第一次超时
			{
				_Collect.b_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
				_Collect.conut++;
				_Collect.runingtask = 'a';
				memcpy(_Collect.amm, _Collect.taska.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taska.next->dadt, DADT_LEN);
				//执行任务
				if(-1 == ExecuteCollect0(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len))
				{
					ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
				}
			}
			else if(_Collect.conut == 1)	//第二次超时
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'a';
				memcpy(_Collect.amm, _Collect.taska.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taska.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
			}
			else //结束当前任务  执行下一次任务
			{
				_Collect.a_isend = 1;
				_Collect.timer = 0;
//				_Collect.flag = 0;
				_Collect.conut = 0;
				memset(_Collect.amm, 0, AMM_ADDR_LEN);
				memset(_Collect.dadt, 0, DADT_LEN);
				_Collect.runingtask = 0;
				pthread_mutex_lock(&_Collect.taska.taska_mutex);
				DeleNearTaskA();
				pthread_mutex_unlock(&_Collect.taska.taska_mutex);
//				//任务数量--
//				_Collect.taskb.num--;
//				//提取下一个任务
//				p = _Collect.taskb.next;
//				_Collect.taskb.next = p->next;
//				//删除上一个任务
//				if(p != NULL)
//				{
//					free(p);
//					p = NULL;
//				}
			}
		}
	}
	else	//已经结束
	{
		if(_Collect.taska.num > 0)
		{
			_Collect.a_isend = 0;
			_Collect.timer = NET_COLLECT_TIMER;
//			_Collect.flag = 0;
			_Collect.conut = 0;
			_Collect.runingtask = 'a';
			memcpy(_Collect.amm, _Collect.taska.next->amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taska.next->dadt, DADT_LEN);
			//执行任务
			if(-1 == ExecuteCollect0(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len))
			{
				ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
			}
		}
	}
}

/*
 * 函数功能:实时采集
 * */
void CollectTaskB(void)
{
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
				_Collect.b_isend = 1;
				_Collect.timer = 0;
//				_Collect.flag = 0;
				_Collect.conut = 0;
				memset(_Collect.amm, 0, AMM_ADDR_LEN);
				memset(_Collect.dadt, 0, DADT_LEN);
				_Collect.runingtask = 0;
				pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
				DeleNearTaskB();
				pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
//				//任务数量--
//				_Collect.taskb.num--;
//				//提取下一个任务
//				p = _Collect.taskb.next;
//				_Collect.taskb.next = p->next;
//				//删除上一个任务
//				if(p != NULL)
//				{
//					free(p);
//					p = NULL;
//				}
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
				memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//执行任务
				if(-1 == ExecuteCollect0(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
				}
			}
			else if(_Collect.conut == 1)	//第二次失败 采用执行任务方式2
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//执行任务
				ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
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
			memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

			//执行任务
			if(-1 == ExecuteCollect0(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len))
			{
				ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
		}
		else if(_Collect.taskc.timer == 0)	//请求任务超时
		{
			if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
			{
				if(_SortNode[_Collect.taskc.index]->cyFlag == _RunPara.CyFlag)
				{
					_Collect.taskc.index++;
					_Collect.taskc.index %= _StandNodeNum;
					printf("num  %d  index  %d\n",_StandNodeNum,_Collect.taskc.index);
					return;
				}
			}
			if(_Collect.taskc.count < 2)
			{
				_Collect.taskc.count++;
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
				{
					CreatAFN14_01up(_SortNode[_Collect.taskc.index]->Amm, &snframe3762);
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
					CreatAFN14_01up(_SortNode[_Collect.taskc.index]->Amm, &snframe3762);
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
					CreatAFN14_01up(_SortNode[_Collect.taskc.index]->Amm, &snframe3762);
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

/*
 * 函数功能:添加任务A
 * 参数:a	任务数据
 * 返回值 0 成功 -1失败
 * */
int AddTaskA(struct task_a *a)
{
	struct task_a *newp = NULL;
	struct task_a *p = NULL;
	struct task_a *temp = NULL;

	if(NULL == a)
	{
		return -1;
	}
	if(NULL == _Collect.taska.next)	//第一次添加
	{
		newp = (struct task_a *)malloc(sizeof(struct task_a));
		if(NULL == newp)
		{
			return -1;
		}
		memcpy(newp, a, sizeof(struct task_a));
		_Collect.taska.next = newp;
		_Collect.taska.num = 1;
		return 0;
	}
	if(TASK_MAX_NUM == _Collect.taska.num)
			return -1;
	//遍历到任务尾
	p = _Collect.taska.next;
	while(p != NULL)
	{
		temp = p;
		p = p->next;
	}

	newp = (struct task_a *)malloc(sizeof(struct task_a));
	if(NULL == newp)
	{
		return -1;
	}
	memcpy(newp, a, sizeof(struct task_a));
	temp->next = newp;
	_Collect.taskb.num += 1;

	return 0;
}

/*
 * 函数功能:删除 当前任务a
 * 链表中第一个任务
 * */
void DeleNearTaskA(void)
{
	struct task_a *a_p;
	if(_Collect.taska.num <= 0)
	{
		return;
	}
	//提取下一个任务，删除当前任务
	_Collect.taska.num--;
	a_p = _Collect.taska.next;
	_Collect.taska.next = a_p->next;
	//删除上一个任务
	if(a_p != NULL)
	{
		free(a_p);
		a_p = NULL;
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
