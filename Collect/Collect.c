/*
 * Collect.c
 *
 *  Created on: 2016��7��21��
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
 * ��������:��ʼ��������
 * */
void CollectorInit(void)
{
	memset(&_Collect, 0, sizeof(Collcet));

	pthread_mutex_init(&_Collect.taska.taska_mutex, NULL);	//����A��
	pthread_mutex_init(&_Collect.taskb.taskb_mutex, NULL);	//����B��
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//����B��
	_Collect.a_isend = 1;	//��ǰ���ս���
	_Collect.b_isend = 1;	//��ǰ���ս���
	_Collect.c_isend = 1;	//��ǰ���ս���
	_Collect.d_isend = 1;	//��ǰ���ս���
}

/*
 * ��������:ִ�����ڳ������³�������
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
 * ��������:��ʱ����
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
 * ��������:�������߳�
 * */
void *Collector(void *arg)
{
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��
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
		feed_watch_dog(wdt_id);	//ι��
		usleep(2);
		//ִ�г�ֵ�ն�����
		if(_Collect.taska.num != 0)
		{
			continue;
		}

		//ִ��ʵʱ͸������
		if(_Collect.taske.num != 0)
		{
			CollectTaskE();
			continue;
		}

		//ִ��ʵʱ��������
		if(_Collect.taskb.num != 0)
		{
			CollectTaskB();
			continue;
		}

		//ִ�����ڳ�������
		if(_Collect.taskc.key == 1)
		{
			CollectTaskC();
			continue;
		}

		//ִ��·����������
		if(_Collect.taskd.key == 1)
		{
			continue;
		}
	}
	pthread_exit(NULL);
}

/*
 *��������:ִ������ʽ0
 *����:	amm		���
 *		inbuf	����������
 *		len		���������ݳ���
 *����ֵ: -1 ʧ�� 0 �ɹ�
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
	//����͸��ת��֡�ṹ
	Create3761AFN10_01(_SortNode[ret]->Ter, inbuf, len, 1, &outbuf);
	//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
	DL3761_Protocol_LinkPack(&outbuf, &snbuf);
	//���Ҷ�Ӧ���׽���
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
 * ��������:ִ������ʽ1
 * ����: amm		����ַ
 * 		inbuf	�����͵�����
 * 		len		�����͵����ݳ���
 * 	����ֵ: -1 ʧ�� 0 �ɹ�
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
		//����͸��ת��֡�ṹ
		Create3761AFN10_01(p->Ter, inbuf, len, 1, &outbuf);
		//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
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

	sleep(EXE_COLLECT1);	//�㲥��Ϊ�˷�ֹ������һ�ն�����������ʧ���ݣ���Ҫ����
	return 0;
}

/*
 * ��������:��ֵ�ն�
 * */
void CollectTaskA(void)
{
	//���������Ƿ����
	if(_Collect.a_isend == 0)	//δ����
	{
		if(_Collect.timer == 0)	//����ʱ
		{
			if(_Collect.conut == 0)	//��һ�γ�ʱ
			{
				_Collect.b_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
				_Collect.conut++;
				_Collect.runingtask = 'a';
				memcpy(_Collect.amm, _Collect.taska.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taska.next->dadt, DADT_LEN);
				//ִ������
				if(-1 == ExecuteCollect0(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len))
				{
					ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
				}
			}
			else if(_Collect.conut == 1)	//�ڶ��γ�ʱ
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'a';
				memcpy(_Collect.amm, _Collect.taska.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taska.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
			}
			else //������ǰ����  ִ����һ������
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
//				//��������--
//				_Collect.taskb.num--;
//				//��ȡ��һ������
//				p = _Collect.taskb.next;
//				_Collect.taskb.next = p->next;
//				//ɾ����һ������
//				if(p != NULL)
//				{
//					free(p);
//					p = NULL;
//				}
			}
		}
	}
	else	//�Ѿ�����
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
			//ִ������
			if(-1 == ExecuteCollect0(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len))
			{
				ExecuteCollect1(_Collect.taska.next->amm, _Collect.taska.next->buf, _Collect.taska.next->len);
			}
		}
	}
}

/*
 * ��������:ʵʱ�ɼ�
 * */
void CollectTaskB(void)
{
	//���������Ƿ����
	if(_Collect.b_isend == 0)	//δ����
	{
		if(_Collect.timer == 0)	//����ʱ
		{
			if(_Collect.conut == 0)	//��һ�γ�ʱ
			{
				_Collect.b_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'b';
				memcpy(_Collect.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				//ִ������
				if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
				{
					ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
				}
			}
			else if(_Collect.conut == 1)	//�ڶ��γ�ʱ
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'b';
				memcpy(_Collect.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
			}
			else //������ǰ����  ִ����һ������
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
//				//��������--
//				_Collect.taskb.num--;
//				//��ȡ��һ������
//				p = _Collect.taskb.next;
//				_Collect.taskb.next = p->next;
//				//ɾ����һ������
//				if(p != NULL)
//				{
//					free(p);
//					p = NULL;
//				}
			}
		}
	}
	else						//����
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
			//ִ������
			if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
			{
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
			}
		}
	}
}

/*
 * ��������:ִ������C ���ڲɼ�����
 * */
void CollectTaskC(void)
{
	tpFrame376_2 snframe3762;
	tp3762Buffer buffer;

	//���γ��յ����ݱ�ʶ�Ƿ����
	if(_Collect.c_isend == 0)	//û�н���
	{
		if(_Collect.timer == 0)	//����ʱ
		{

			if(_Collect.conut == 0)	//��һ�γ�ʱ ����ִ������ʽ1
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//ִ������
				if(-1 == ExecuteCollect0(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
				}
			}
			else if(_Collect.conut == 1)	//�ڶ���ʧ�� ����ִ������ʽ2
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//ִ������
				ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
			else	//������ǰ����  ִ����һ������
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
	else	//ִ��һ�¸�����
	{
		if(_Collect.taskc.isok == 1)	//��������ɹ�	ִ������
		{
			_Collect.c_isend = 0;
			_Collect.timer = NET_COLLECT_TIMER;
//			_Collect.flag = 0;
			_Collect.conut = 0;
			_Collect.runingtask = 'c';
			memcpy(_Collect.amm, _SortNode[_Collect.taskc.index]->Amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

			//ִ������
			if(-1 == ExecuteCollect0(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len))
			{
				ExecuteCollect1(_SortNode[_Collect.taskc.index]->Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
		}
		else if(_Collect.taskc.timer == 0)	//��������ʱ
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

					//����
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, buffer.Data, buffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.taskc.timer = AFN14_01_TIMER;
					_Collect.taskc.isok = 0;
				}
			}
			else if(_Collect.taskc.count == 2)	//��������ʧ��2�� ���л���һֻ����������
			{
				_Collect.taskc.index++;
				_Collect.taskc.index %= _StandNodeNum;
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
				{
					CreatAFN14_01up(_SortNode[_Collect.taskc.index]->Amm, &snframe3762);
					DL3762_Protocol_LinkPack(&snframe3762, &buffer);

					//����
					pthread_mutex_lock(&writelock);
					UsartSend(Usart0Fd, buffer.Data, buffer.Len);
					pthread_mutex_unlock(&writelock);
					_Collect.taskc.timer = AFN14_01_TIMER;
					_Collect.taskc.count = 1;
					_Collect.taskc.isok = 0;
				}
			}
			else if(_Collect.taskc.count >= 3)	//��һ����������
			{
				//���췢��֡
				_Collect.taskc.index = 0;
				if(_Collect.taskc.index >= 0 && _StandNodeNum > 0)
				{
					CreatAFN14_01up(_SortNode[_Collect.taskc.index]->Amm, &snframe3762);
					DL3762_Protocol_LinkPack(&snframe3762, &buffer);

					//����
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
 * ��������:͸��ת��
 * */
void CollectTaskE(void)
{
	//���������Ƿ����
	if(_Collect.e_isend == 0)	//δ����
	{
		if(_Collect.timer == 0)	//����ʱ
		{
			if(_Collect.conut == 0)	//��һ�γ�ʱ
			{
				_Collect.e_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'e';
				memcpy(_Collect.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taske.next->dadt, DADT_LEN);
				//ִ������
				if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
				{
					ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
				}
			}
			else if(_Collect.conut == 1)	//�ڶ��γ�ʱ
			{
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'e';
				memcpy(_Collect.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taske.next->dadt, DADT_LEN);
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
			}
			else //������ǰ����  ִ����һ������
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
	else						//����
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
			//ִ������
			if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
			{
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len);
			}
		}
	}
}

/*
 * ��������:�������A
 * ����:a	��������
 * ����ֵ 0 �ɹ� -1ʧ��
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
	if(NULL == _Collect.taska.next)	//��һ�����
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
	//����������β
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
 * ��������:ɾ�� ��ǰ����a
 * �����е�һ������
 * */
void DeleNearTaskA(void)
{
	struct task_a *a_p;
	if(_Collect.taska.num <= 0)
	{
		return;
	}
	//��ȡ��һ������ɾ����ǰ����
	_Collect.taska.num--;
	a_p = _Collect.taska.next;
	_Collect.taska.next = a_p->next;
	//ɾ����һ������
	if(a_p != NULL)
	{
		free(a_p);
		a_p = NULL;
	}
}

/*
 * ��������:�������B
 * ����:	b	��������
 * ����ֵ 0 �ɹ� -1ʧ��
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
	if(NULL == _Collect.taskb.next)	//��һ�����
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

	//����������β
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
 * ��������:ɾ����ǰ����b
 * ע:ɾ�����������
 * */
void DeleNearTaskB(void)
{
	struct task_b *b_p;
	if(_Collect.taskb.num <= 0)
	{
		return;
	}
	//��ȡ��һ������ɾ����ǰ����
	_Collect.taskb.num--;
	b_p = _Collect.taskb.next;
	_Collect.taskb.next = b_p->next;
	//ɾ����һ������
	if(b_p != NULL)
	{
		free(b_p);
		b_p = NULL;
	}
}

/*
 * ��������:�������E
 * ����:	e	��������
 * ����ֵ 0 �ɹ� -1ʧ��
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

	if(NULL == _Collect.taske.next)	//��һ�����
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
	//����������β
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
 * ��������:ɾ����ǰ����e
 * ע:ɾ�����������
 * */
void DeleNearTaskE(void)
{
	if(_Collect.taske.num <= 0)
	{
		return;
	}
	struct task_e *e_p;
	//��ȡ��һ������ɾ����ǰ����
	_Collect.taske.num--;
	e_p = _Collect.taske.next;
	_Collect.taske.next = e_p->next;
	//ɾ����һ������
	if(e_p != NULL)
	{
		free(e_p);
		e_p = NULL;
	}
}
