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
#include "DL645.h"
#include "SeekAmm.h"
#include "NetWork1.h"
#include "hld_ac.h"
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
	pthread_mutex_init(&_Collect.taskc.taskc_mutex, NULL);	//����C��
	pthread_mutex_init(&_Collect.taske.taske_mutex, NULL);	//����E��

	_Collect.status.isend = 1;								//��ǰ���ս���

	pthread_mutex_init(&_Collect.mutex, NULL);

	BoradcastContorlInit();
}

/*
 * ��������:���ó�����״̬
 * */
void SetCollectorStatus(CollcetStatus *status)
{
	pthread_mutex_lock(&_Collect.mutex);

	memcpy(&_Collect.status, status, sizeof(CollcetStatus));

	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * ��������:��ȡ��������ǰ����״̬
 * ������	status		���س���������״̬
 * */
void GetCollectorStatus(CollcetStatus *status)
{
	pthread_mutex_lock(&_Collect.mutex);
	memcpy(status, &_Collect.status, sizeof(CollcetStatus));
	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * ��������:��ʼ���㲥������
 * */
void BoradcastContorlInit(void)
{
	memset(&_BoradContorl, 0, sizeof(BoradcastContorl));
	pthread_mutex_init(&_BoradContorl.mutex, NULL);
}

/*
 * ��������:��ù㲥ȫ�ֵ���ʱ
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
 * ��������:���ù㲥ȫ�ֵ���ʱʱ��
 * */
void SetBoradcastContorlTimer(int t)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.timer = t;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * ��������:�㲥ȫ�ֵ���ʱ�ݼ�
 * */
void BoradcastContorlTimerTicker(void)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.timer--;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * ��������:��ȡˢ�¹㲥��ɵ���ʱ
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
 * ��������:����ˢ�¹㲥��ɵ���ʱ
 * */
void SetBoradcastContorlAllowTimer(int t)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.flush_allow_timer = t;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * ��������:ˢ�¹㲥��ɵ���ʱ�ݼ�
 * */
void BoradcastContorlAllowTimerTicker(void)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.flush_allow_timer--;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * ��������:��ȡ�㲥��ɴ����������ɴ�������0�����һ
 * ����ֵ: ��ɴ���
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
 * ��������:���ù㲥��ɴ���
 * ����:		num 	���õĴ���
 * */
void SetBoradcastContorlAllowNum(int num)
{
	pthread_mutex_lock(&_BoradContorl.mutex);
	_BoradContorl.allow_num = num;
	pthread_mutex_unlock(&_BoradContorl.mutex);
}

/*
 * ��������:��ȡ���������񵹼�ʱ
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
 * ��������:�������񵹼�ʱ�ݼ�
 * */
void CollectorStatusTimerTicker(void)
{
	pthread_mutex_lock(&_Collect.mutex);
	_Collect.status.timer--;
	pthread_mutex_unlock(&_Collect.mutex);
}

/*
 * ��������:��ʱ����
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
 * ��������:�������߳�
 * */
void *Collector(void *arg)
{
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��
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
		if(0 == TaskaIsEmpty())
		{
			SetTaskaKey(1);	//����
			goto bbb;	//��ת��  ���ڳ������񣬷�ֹ��ֹ̫�����ڳ������񣬼�������λ��̫��ģ��
						//���ڳ����������жϣ��Ƿ��г�ֵ���г�ֵ����ʱ��ֻ�����������󣬲�ִ��
		}
		SetTaskaKey(0);	//������

		//ִ��ʵʱ͸������
		if(GetTaskeNum() > 0)
		{
			CollectTaskE();
			continue;
		}

		//ִ��ʵʱ��������
		if(GetTaskbNum() > 0)
		{
			CollectTaskB();
			continue;
		}
bbb:
		//ִ�����ڳ�������
		if(GetTaskcKey() == 1)
		{
			CollectTaskC();
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
	StandNode node;

	if(0 != _hld_ac.get_status())	//δ��Ȩʱ�����������ݳ���
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
			if(0 == judge_seek_amm_task(p->Ter)) //������ն������ѱ��򲻽��������·�
				break;
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
				if(errno == SIGPIPE)	//�ȴ�����
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

	if(0 != _hld_ac.get_status())	//δ��Ȩʱ�����������ݳ���
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
		//����͸��ת��֡�ṹ
		Create3761AFN10_01(p->Ter, inbuf, len, 1, &outbuf);
		//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
		DL3761_Protocol_LinkPack(&outbuf, &snbuf);

		pthread_mutex_lock(&(p->write_mutex));
		while(1)
		{
			if(0 == judge_seek_amm_task(p->Ter))	//������ն������ѱ��򲻽��������·�
				break;
			ret = write(p->s, snbuf.Data, snbuf.Len);
			if(ret < 0)
			{
				if(errno == SIGPIPE)	//�ȴ�����
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
//	sleep(EXE_COLLECT1);	//�㲥��Ϊ�˷�ֹ������һ�ն�����������ʧ���ݣ���Ҫ����
	return 0;
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
 * ��������:��ֵ��������Ƿ�Ϊ��
 * ����ֵ: 0 ���� -1 ��
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
 * ��������:����ִ�г�ֵ�������񿪹�
 * */
void SetTaskaKey(int key)
{
	pthread_mutex_lock(&_Collect.taska.taska_mutex);

	_Collect.taska.key = key;

	pthread_mutex_unlock(&_Collect.taska.taska_mutex);
}

/*
 * ��������:��ó�ֵ���񿪹�״̬
 * ����ֵ:����״̬ 1��  ������
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
	tpFrame645 f645;
	Buff645 b645;
	tpFrame376_1 snframe3761;
	tp3761Buffer ttpbuffer;
	struct topup_node *pp;
	int ret = 0;
	struct task_a_node *temp_node;
	unsigned char default_mark[TER_ADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char p_mark[TER_ADDR_LEN]; //�̱߳�־  ��־�̵߳�ǰ������ն� pthread mark
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

		if(_Collect.taska.key != 1 && ret == 1)	//�ȴ��ѾôӶ�����ȡ���������ȼ�����ִ�н�������ִ�г�ֵ����
		{								//��ֹ�������ȼ�������ոչ㲥����������ּ����������ݵ��³���
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			continue;
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
						while(GetBoradcastContorlTimer() > 0)
						{
							sleep(1);
						}
						ret = ExecuteCollect0(temp_node->amm, temp_node->task_head->buf, temp_node->task_head->len);
						if(-1 == ret)	//����ִ��ʧ�� ��������
						{

							if(GetBoradcastContorlAllowTimer() <= 0)
							{
								SetBoradcastContorlAllowTimer(60);	//����ˢ�¹㲥��ɵ���ʱʱ��
								SetBoradcastContorlAllowNum(3);	//���ù㲥��ɴ���
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
							//�����Ҳ���·������ֵ�ն�
							memcpy(f645.Address, temp_node->amm, AMM_ADDR_LEN);
							f645.CtlField = 0xD1;
							f645.Datas[0] = 0xFE + 0x33;
							f645.Length = 1;
							if(0 == Create645From(&f645, &b645))
							{
								Create3761AFN10_01(temp_node->task_head->top_ter, b645.buf, b645.len, 0, &snframe3761);
								//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
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
 * ����ֵ 0 �ɹ� -1�޸ñ�  -2 ����ռ  -3
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
		printf("add task a new node\n");
		//����ڵ�  �������ڵ�
		struct task_a_node *new_node = (struct task_a_node *)malloc(sizeof(struct task_a_node));
		if(NULL == new_node)
		{
			perror("malloc task a new node:");
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -3;
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
			return -3;
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
		printf("add task a new task\n");

		//�ж��ն��Ƿ���������ռ
		if(1 != CompareUcharArray(p->amm, amm, AMM_ADDR_LEN))
		{
			//�ն��Ѿ�����ռ
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
			return -2;
		}

		//���������
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
//				printf("dele task a \n");
			}
			break;
		case 1:		//ɾ����Ӧ����ȫ������,��ɾ���ڵ�
//			printf("dele task a node\n");
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
//			if(_Collect.taska.node_head == NULL)
//				printf("task node is empty\n");

			break;
		default:
			break;
	}
	pthread_mutex_unlock(&_Collect.taska.taska_mutex);

	return 0;
}

/******************************************************����B*************************************************/
/*
 * ��������:��ȡ����B����
 * ����ֵ: >=0����  <0����
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
 * ��������:ʵʱ�ɼ�
 * */
void CollectTaskB(void)
{
	unsigned char buf[300] = {0};
	int len = 0;
	tpFrame376_2 snframe3762;
	tp3762Buffer tpbuffer;
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);
	//���������Ƿ����
	if(c_status.isend == 0)	//δ����
	{
		if(c_status.timer == 0)	//����ʱ
		{
			if(c_status.conut == 0)	//��һ�γ�ʱ
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'b';
				memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				//ִ������
				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);

				if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
				{
					while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
					{
						sleep(1);
					}

					//����״̬  �ڳ��������·�֮ǰ���£���ֹ�ڸ��³���״̬ʱ�������Ѿ��յ�
					c_status.conut = 3;
					c_status.timer = NET_COLLECT_TIMER + 2;
					SetCollectorStatus(&c_status);

					ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
					SetBoradcastContorlTimer(3);
				}
			}
			else if(c_status.conut == 1)	//�ڶ��γ�ʱ
			{
				c_status.conut++;
				c_status.runingtask = 'b';
				memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
				{
					sleep(1);
				}
				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);
				ExecuteCollect1(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len);
				SetBoradcastContorlTimer(3);
			}
			else //������ǰ����  ִ����һ������
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
	else						//����
	{
		if(GetTaskbNum() > 0)
		{
			c_status.isend = 0;
			c_status.conut = 0;
			c_status.runingtask = 'b';
			memcpy(c_status.amm, _Collect.taskb.next->amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taskb.next->dadt, DADT_LEN);
			//ִ������
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);
			if(-1 == ExecuteCollect0(_Collect.taskb.next->amm, _Collect.taskb.next->buf, _Collect.taskb.next->len))
			{
				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
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
 * ��������:�������B
 * ����:	b	��������
 * ����ֵ 0 �ɹ� -1ʧ��
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
	if(NULL == _Collect.taskb.next)	//��һ�����
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
 * ��������:ɾ����ǰ����b
 * ע:ɾ�����������
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
	pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
}


/******************************************************����C***********************************************/
/*
 * ��������:ִ�����ڳ������³�������
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
 * ��������:��ȡ���ڳ�����״̬
 * ����ֵ: ����״̬
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
 * ��������:��ȡ�������񵹼�ʱ
 * ����ֵ:����ʱʱ��
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
 * ��������:�������񵹼�ʱ�ݼ�
 * */
void RequestTimerTicker(void)
{
	pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
	_Collect.taskc.timer--;
	pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
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
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);

	//���γ��յ����ݱ�ʶ�Ƿ����
	if(c_status.isend == 0)	//û�н���
	{
		if(c_status.timer == 0)	//����ʱ
		{

			if(c_status.conut == 0)	//��һ�γ�ʱ ����ִ������ʽ1
			{
				c_status.isend = 0;
//				_Collect.flag = 0;
				c_status.conut++;
				c_status.runingtask = 'c';
				memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

				//ִ������
				while(GetBoradcastContorlTimer() > 0)
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);
				//ִ������
				if(0 == GetTaskaKey())
				{
					if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
					{
						while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
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
			else if(c_status.conut == 1)	//�ڶ���ʧ�� ����ִ������ʽ2
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'c';
				memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);

				//ִ������
				if(0 == GetTaskaKey())
				{
					ExecuteCollect1(node.Amm, _Collect.taskc.buf, _Collect.taskc.len);
					SetBoradcastContorlTimer(3);
				}
			}
			else	//������ǰ����  ִ����һ������
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
	else	//ִ��һ�¸�����
	{
		pthread_mutex_lock(&_Collect.taskc.taskc_mutex);
		if(_Collect.taskc.isok == 1)	//��������ɹ�	ִ������
		{
			c_status.isend = 0;
			c_status.conut = 0;
			c_status.runingtask = 'c';
			memcpy(c_status.amm, node.Amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taskc.dadt, DADT_LEN);

			//ִ������
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);

			if(0 == GetTaskaKey())	//��ֵ����ִ��ʱ���������������񣬵���������
			{
				if(-1 == ExecuteCollect0(node.Amm, _Collect.taskc.buf, _Collect.taskc.len))
				{
					while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
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
		else if(_Collect.taskc.timer == 0)	//��������ʱ
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
				_Collect.taskc.index %= GetStandNodeNum();
				if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
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
				if(_Collect.taskc.index >= 0 && GetStandNodeNum() > 0)
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
		pthread_mutex_unlock(&_Collect.taskc.taskc_mutex);
	}
}

/*************************************************����E***************************************************/

/*
 * ��������:��ȡ͸��ת���������
 * ����ֵ:>=0 ���� <0����
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
 * ��������:͸��ת��
 * */
void CollectTaskE(void)
{
	CollcetStatus c_status;

	GetCollectorStatus(&c_status);
	//���������Ƿ����
	if(c_status.isend == 0)	//δ����
	{
		if(c_status.timer == 0)	//����ʱ
		{
			if(c_status.conut == 0)	//��һ�γ�ʱ
			{
				c_status.isend = 0;
				c_status.conut++;
				c_status.runingtask = 'e';
				memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
				//ִ������
				while(GetBoradcastContorlTimer() > 0)
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER;
				SetCollectorStatus(&c_status);
				if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
				{
					while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
					{
						sleep(1);
					}
					c_status.timer = NET_COLLECT_TIMER + 2;
					c_status.conut = 3;	//ִ����㲥��  �����ʱ ֱ�ӽ�������
					SetCollectorStatus(&c_status);

					ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf,  _Collect.taske.next->len);
					SetBoradcastContorlTimer(3);
				}
			}
			else if(c_status.conut == 1)	//�ڶ��γ�ʱ
			{
				c_status.conut++;
				c_status.runingtask = 'e';
				memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
				memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
				{
					sleep(1);
				}

				c_status.timer = NET_COLLECT_TIMER + 2;
				SetCollectorStatus(&c_status);
				ExecuteCollect1(_Collect.taske.next->amm, _Collect.taske.next->buf,  _Collect.taske.next->len);
				SetBoradcastContorlTimer(3);
			}
			else //������ǰ����  ִ����һ������
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
	else						//����
	{
		if(GetTaskeNum() > 0)
		{
			c_status.isend = 0;
//			_Collect.flag = 0;
			c_status.conut = 0;
			c_status.runingtask = 'e';
			memcpy(c_status.amm, _Collect.taske.next->amm, AMM_ADDR_LEN);
			memcpy(c_status.dadt, _Collect.taske.next->dadt, DADT_LEN);
			//ִ������
			while(GetBoradcastContorlTimer() > 0)
			{
				sleep(1);
			}

			c_status.timer = NET_COLLECT_TIMER;
			SetCollectorStatus(&c_status);
			if(-1 == ExecuteCollect0(_Collect.taske.next->amm, _Collect.taske.next->buf, _Collect.taske.next->len))
			{
				while(GetBoradcastContorlTimer() > 0)	//�㲥���������ʱһ��ʱ�䣬����ͨ��
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
 * ��������:�������E
 * ����:	e	��������
 * ����ֵ 0 �ɹ� -1ʧ��
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
	if(NULL == _Collect.taske.next)	//��һ�����
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
 * ��������:ɾ����ǰ����e
 * ע:ɾ�����������
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
	pthread_mutex_unlock(&_Collect.taske.taske_mutex);
}


/****************************************************************************************************/
