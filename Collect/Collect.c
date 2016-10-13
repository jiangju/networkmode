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
#include "CommLib.h"
#include "HLDWatchDog.h"
#include "DL3762_AFN13.h"
/*
 * ��������:��ʼ��������
 * */
void CollectorInit(void)
{
	memset(&_Collect, 0, sizeof(Collcet));

	pthread_mutex_init(&_Collect.taska.taska_mutex, NULL);	//����A��
	pthread_cond_init(&_Collect.taska.need_assigned, NULL); //��������
	pthread_cond_init(&_Collect.taska.node_not_empty, NULL);//��������

	_Collect.taska.task_num = 0;							//����������
	_Collect.taska.not_assigned_num = 0;					//û�з��ɵĽڵ����
	_Collect.taska.key = 0;									//�������ֵ����ִ��
	_Collect.taska.node_head = NULL;
	_Collect.taska.node_tail = NULL;

	pthread_mutex_init(&_Collect.taskb.taskb_mutex, NULL);	//����B��
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//����B��

	_Collect.b_isend = 1;	//��ǰ���ս���
	_Collect.c_isend = 1;	//��ǰ���ս���
//	_Collect.d_isend = 1;	//��ǰ���ս���
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
 * ��������:��ֵ�ն����񵹼�ʱ�߳�
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
			//���½ڵ��µ�����ִ�е���ʱ
			if((node_p->task_status.is_end == 1) && node_p->task_status.timer > 0)
			{
				node_p->task_status.timer--;
			}

			//���½ڵ���ڵ���ʱ
			if(node_p->timer > 0)
			{
				node_p->timer--;
				node_p = node_p->next;
			}
			else
			{
				//ɾ���ڵ� ������
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
 * ��������:��ֵ�ն�
 * */
void *CollectTaskA(void *arg)
{
	int ret = 0;
	struct task_a_node *temp_node;
	unsigned char default_mark[TER_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char p_mark[TER_ADDR_LEN]; //�̱߳�־  ��־�̵߳�ǰ������ն� pthread mark
	memset(p_mark, 0xFF, TER_ADDR_LEN);
	while(1)
	{
		ret = CompareUcharArray(p_mark, default_mark, TER_ADDR_LEN);
		pthread_mutex_lock(&_Collect.taska.taska_mutex);
		while(ret == 1 && _Collect.taska.not_assigned_num == 0)
		{
			pthread_cond_wait(&_Collect.taska.need_assigned, &_Collect.taska.taska_mutex);
		}

		while(_Collect.taska.key != 1 && ret == 1)	//�ȴ��ѾôӶ�����ȡ���������ȼ�����ִ�н�������ִ�г�ֵ����
		{								//��ֹ�������ȼ�������ոչ㲥����������ּ����������ݵ��³���
			usleep(1);
		}


		if(ret != 1)	//�з�����ն�  ִ���ն��µ�����
		{
			//��ȡ�ڵ�
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);	//���õ�SeekTaskANode0�������м���  ����
			temp_node = SeekTaskANode0(p_mark);
			pthread_mutex_lock(&_Collect.taska.taska_mutex);
			if(NULL == temp_node)	//�޸ýڵ�
			{
				printf("not found taska node\n");
				memset(p_mark, 0xFF, TER_ADDR_LEN);
			}
			else	//�иýڵ�
			{
				if(temp_node->task_status.is_end == 0)	//�������  ����ִ����һ����
				{
					//ִ������
					if(NULL != temp_node->task_head)	//��������ִ�� ��������ִ��
					{
						ret = ExecuteCollect0(temp_node->amm, temp_node->task_head->buf, temp_node->task_head->len);
						if(-1 == ret)	//����ִ��ʧ�� ��������
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
				else	//����δ����  �ж��Ƿ�ʱ  ��ʱ��������
				{
					if(temp_node->task_status.timer > 0)
					{
						//δ��ʱ
					}
					else	//��ʱ
					{
						//ɾ������
						pthread_mutex_unlock(&_Collect.taska.taska_mutex);
						DeleTaskA(temp_node->amm, 0);
						pthread_mutex_lock(&_Collect.taska.taska_mutex);
						temp_node->task_status.is_end = 0;
					}
				}
			}
		}
		else	//û�з����ն� �����ն�
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
	//��ʼ����ֵ����ִ����
	TaskACollectInit();
	//���� ά�� ��ֵ��������߳�
	pthread_t pt;
	pt = pthread_create(&pt, NULL, TaskATimer, NULL);
	if(pt < 0)
	{
		pthread_exit(NULL);
	}

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��
		usleep(2);
		//ִ�г�ֵ�ն�����
		if(_Collect.taska.node_head != NULL)
		{
			_Collect.taska.key = 1;	//����
			if('a' != _Collect.runingtask)
				_Collect.runingtask = 'a';
			continue;
		}
		_Collect.taska.key = 0;	//������

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
//		if(_Collect.taskd.key == 1)
//		{
//			continue;
//		}
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

	//����͸��ת��֡�ṹ
	Create3761AFN10_01(node.Ter, inbuf, len, 1, &outbuf);
	//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
	DL3761_Protocol_LinkPack(&outbuf, &snbuf);
	//���Ҷ�Ӧ���׽���
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

	pthread_mutex_lock(&(route_mutex));
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
	sleep(EXE_COLLECT1);	//�㲥��Ϊ�˷�ֹ������һ�ն�����������ʧ���ݣ���Ҫ����
	return 0;
}

/*
 * ��������:ʵʱ�ɼ�
 * */
void CollectTaskB(void)
{
	unsigned char buf[300] = {0};
	int len = 0;
	tpFrame376_2 snframe3762;
	tp3762Buffer tpbuffer;

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
				//����376.2 AFN13�ϱ�����
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
	StandNode node;
	GetStandNode(_Collect.taskc.index, &node);

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
				memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//ִ������
				if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
				}
			}
			else if(_Collect.conut == 1)	//�ڶ���ʧ�� ����ִ������ʽ2
			{
				_Collect.c_isend = 0;
				_Collect.timer = NET_COLLECT_TIMER;
//				_Collect.flag = 0;
				_Collect.conut++;
				_Collect.runingtask = 'c';
				memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

				//ִ������
				ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
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
			memcpy(_Collect.amm, node.Amm, AMM_ADDR_LEN);
			memcpy(_Collect.dadt, _Collect.taskc.dadt, DADT_LEN);

			//ִ������
			if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
			{
				ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
			}
		}
		else if(_Collect.taskc.timer == 0)	//��������ʱ
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
					CreatAFN14_01up(node.Amm, &snframe3762);
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
					CreatAFN14_01up(node.Amm, &snframe3762);
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

/*******************************************����a�Ľӿ�*********************************************/

/*
 * ��������:��ֵ�ն�����ִ������ʼ��
 * ����:
 * ����ֵ:
 * */
void TaskACollectInit(void)
{
	_Collect.taska.task_num = 0;							//����������
	_Collect.taska.not_assigned_num = 0;					//û�з���Ľڵ����
	_Collect.taska.key = 0;									//�������ֵ����ִ��
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
 * ��������:�����ն˵�ַ�鿴��������е��Ƿ��и÷�֧
 * ����:	ter		�ն˵�ַ
 * ����ֵ: ���� ��֧�Ľڵ��ַ  ������ NULL
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
 * ��������:���ݵ���ַ�鿴����������Ƿ��и÷�֧
 * ����:	amm		����ַ
 * ����ֵ:	����  ��֧�ڵ��ַ		������  NULL
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
 * ��������:�������A
 * ����:	amm		����
 * 		dadt	���ݱ�ʶ
 * 		buf		��������
 * 		len		���ݳ���
 * 		ter		��ֵ�ն˵�ַ
 * ����ֵ 0 �ɹ� -1ʧ��
 * */
int AddTaskA(unsigned char *amm, unsigned char *dadt, unsigned char *buf, int len, unsigned char *ter)
{
	int ret= 0;
	struct task_a_node * p;
	struct task_a_content *new_task;
	//��ȡ������Ҫ�·����ն˵�ַ
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

	//�ж��Ƿ���Ҫ����µķ�����֧
	//�鿴�����������û����ͬ���ն˵�ַ
	p = SeekTaskANode0(node.Ter);
	pthread_mutex_lock(&_Collect.taska.taska_mutex);
	if(NULL == p)	//����½ڵ�
	{
//		printf("add task a new node\n");
		//����ڵ�  �������ڵ�
		struct task_a_node *new_node = (struct task_a_node *)malloc(sizeof(struct task_a_node));
		if(NULL == new_node)
		{
			perror("malloc task a new node:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -1;
		}
		memset(new_node, 0, sizeof(struct task_a_node));
		//�ն˵�ַ
		memcpy(new_node->ter, node.Ter, TER_ADDR_LEN);
		//����ַ
		memcpy(new_node->amm, amm, AMM_ADDR_LEN);
		//δ�����̴߳���
		new_node->is_allot = 0;
		//�ڵ���ڵ���ʱ 1min
		new_node->timer = TASKA_NODE_LIVE_TIMER;
		//����ִ��״̬
		memset(&new_node->task_status, 0, sizeof(struct task_a_status));
		//ͷ����
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

		//β����
		new_node->task_tail = new_node->task_head;
		//��һ���ڵ�
		new_node->next = NULL;

		//���ڵ�����������
		if(NULL == _Collect.taska.node_head)	//��һ����ӽڵ�
		{
			_Collect.taska.node_head = new_node;
			_Collect.taska.node_tail = _Collect.taska.node_head;
			pthread_cond_broadcast(&(_Collect.taska.node_not_empty));
		}
		else	//�ǵ�һ����ӽڵ�
		{
			_Collect.taska.node_tail->next = new_node;
			_Collect.taska.node_tail = _Collect.taska.node_tail->next;
		}
	}
	else			//�����еĽڵ����������
	{
//		printf("add task a new task\n");

		//�ж��ն��Ƿ���������ռ
		if(1 != CompareUcharArray(p->amm, amm, AMM_ADDR_LEN))
		{
			//�ն��Ѿ�����ռ
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -1;
		}

		//���������
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

	//����������
	_Collect.taska.task_num++;
	if(p == NULL)
	{
		//δ������ն�����
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
 * ��������:���ݵ��ɾ����ֵ����
 * ����:	amm		���
 * 		flag	ɾ����ʽ  0  ɾ��������ϵ�����
 * 						1  ɾ��������������
 * ����ֵ: 0�ɹ�  -1 ʧ��
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
		case 0:		//ɾ����Ӧ������ϵ�����
			if(p->task_head != NULL)	//������
			{
				//ɾ������
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
		case 1:		//ɾ����Ӧ����ȫ������,��ɾ���ڵ�
			printf("dele task a node\n");
			//ɾ���ڵ��ϵ�����
			while(p->task_head != NULL)
			{
				temp = p->task_head;
				p->task_head = p->task_head->next;
				free(temp);
				temp = NULL;
				_Collect.taska.task_num--;
			}

			//ɾ���ڵ�
			struct task_a_node *last_p;	//��ɾ���ڵ����һ���ڵ�
			if(p == _Collect.taska.node_head)	//ɾ���Ľڵ�Ϊ��һ���ڵ�
			{
				if(0 == p->is_allot)	//��δ����
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
			else		//ɾ���Ľڵ�Ϊ�ǵ�һ���ڵ�
			{
				//��ȡ��ɾ���ڵ����һ���ڵ�
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
				if(0 == p->is_allot)	//��δ����
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


