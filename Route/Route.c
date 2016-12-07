
/*
 * Route.c
 *
 *  Created on: 2016��6��29��
 *      Author: j
 */
#define _ROUTE_C_
#include "Route.h"
#undef	_ROUTE_C_

#include <string.h>
#include <stdlib.h>
#include "CommLib.h"
#include <stdio.h>
#include "../NetWork0/NetWork0.h"
#include <pthread.h>
#include <error.h>
#include <unistd.h>
#include "HLDWatchDog.h"
#include <sys/time.h>

/*
 * ��������:�����ն˵�ַ���ҽڵ��ַ
 * ����:	Ter		�ն˵�ַ
 * ����ֵ: NULL ʧ�� !NULL ���ҵ��Ľڵ��ַ
 * */
TerSocket *AccordTerSeek(unsigned char *ter)
{
	if(NULL == _FristNode)
	{
		return NULL;
	}
	TerSocket *p = NULL;
	int ret = 0;
	p = _FristNode;
	while(NULL != p)
	{
		ret = CompareUcharArray(p->Ter, ter, TER_ADDR_LEN);
		if(1 == ret)
		{
			break;
		}
		p = p->next;
	}
	return p;
}

/*
 * ��������:�����׽��ֲ��ҽڵ��ַ
 * ����:	s		�׽���
 * ����ֵ:NULL ʧ��  !=NULL �ڵ��ַ
 * */
TerSocket *AccordSocketSeek(int s)
{
	if(NULL == _FristNode)
	{
		return NULL;
	}
	TerSocket *p = NULL;
	p = _FristNode;
	while(NULL != p)
	{
		if(p->s == s)
		{
			break;
		}
		p = p->next;
	}
	return p;
}

/*
 * ��������:������Ų��ҽڵ��ַ
 * ���� 	num		���
 * ����ֵ:NULL ʧ��  !=NULL �ڵ��ַ
 * */
TerSocket *AccordNumSeek(int num)
{
	TerSocket *p = _FristNode;
	int i = 1;
	if(num <= 0)
	{
		return NULL;
	}

	while(NULL != p)
	{
		if(i == num)
		{
			break;
		}
		p = p->next;
		i++;
	}
	return p;
}

/*
 * ��������:���·��
 * ����:ter		�ն˵�ַ
 * 		s		�׽��ֵ�ַ
 * 		ticker	��������ʱ
 * 		addr	���ܻ����ڶ��е�λ��
 *����ֵ NULL ʧ��  ��NULL �½ڵ�ĵ�ַ
 * */
TerSocket *AddRoute(unsigned char *ter, int s, int ticker, void *addr)
{
	//��һ�����
	if(NULL == _FristNode)
	{
		_FristNode = (TerSocket*)malloc(sizeof(TerSocket));
		if(NULL == _FristNode)
		{
//			printf("route.c 87\n");
			return NULL;
		}
		memcpy(_FristNode->Ter, ter, TER_ADDR_LEN);
		_FristNode->s = s;
		_FristNode->ticker = ticker;
		_FristNode->rvaddr = addr;
		pthread_mutex_init(&(_FristNode->write_mutex), NULL);
		_FristNode->next = NULL;
		return _FristNode;
	}
	TerSocket *p = _FristNode;

	//����������ĩβ
	while(NULL != p->next)
	{
		p = p->next;
	}

	TerSocket *newp = (TerSocket*)malloc(sizeof(TerSocket));
	if(NULL == newp)
	{
		perror("route.c 87");
		return NULL;
	}

	//��ӽڵ㵽����
	p->next = newp;
	memcpy(newp->Ter, ter, TER_ADDR_LEN);
	newp->s = s;
	newp->ticker = ticker;
	newp->rvaddr = addr;
	pthread_mutex_init(&(newp->write_mutex), NULL);
	newp->next = NULL;
	return newp;
}

/*
 *��������:����socket ɾ���ڵ�
 *����:	s		��ɾ���ڵ���׽���
 *����ֵ: ��
 * */
void AccordSocketDele(int s)
{
	if(NULL == _FristNode)
		return;
	TerSocket *p = _FristNode;
	TerSocket *temp = p;
	while(NULL != p)
	{
		if(s == p->s)
		{
			break;
		}
		temp = p;
		p = p->next;
	}

	if(NULL == p)
	{
		return;
	}

	pthread_mutex_lock(&(_route_recycle_mutex));
	AddRouteRecycle(p, ROUTE_RECYCLE_TICKER);
	pthread_mutex_unlock(&(_route_recycle_mutex));

	//������ֻ��һ���ڵ㣬ɾ����һ���ڵ�
	if(p == _FristNode && NULL == p->next)
	{
		_FristNode = NULL;
	}
	else if(p == _FristNode)	//ɾ����һ���ڵ�
	{
		_FristNode = p->next;
	}
	else
	{
		//ɾ����һ���ڵ��Ժ�Ľڵ�
		temp->next = p->next;
	}
}

/*
 * ��������:�����ն˵�ַɾ���ڵ�
 * ����:
 * 		ter		�ն˵�ַ
 * ����ֵ:��
 * */
void AccordTerDele(unsigned char *ter)
{
	if(NULL == _FristNode)
	{
		return;
	}

	TerSocket *p = _FristNode;
	TerSocket *temp = p;

	int ret = 0;
	while(NULL != p)
	{
		ret = CompareUcharArray(p->Ter, ter, TER_ADDR_LEN);
		if(1 == ret)
		{
			break;
		}
		temp = p;
		p = p->next;
	}
	if(NULL == p)
	{
		return;
	}

	pthread_mutex_lock(&(_route_recycle_mutex));
	AddRouteRecycle(p, ROUTE_RECYCLE_TICKER);
	pthread_mutex_unlock(&(_route_recycle_mutex));

	//������ֻ��һ���ڵ㣬ɾ����һ���ڵ�
	if(p == _FristNode && NULL == p->next)
	{
		_FristNode = NULL;
	}
	else if(p == _FristNode)	//ɾ����һ���ڵ�
	{
		_FristNode = p->next;
	}
	else
	{
		//ɾ����һ���ڵ��Ժ�Ľڵ�
		temp->next = p->next;
	}
}

/*
 * ��������:�������һ��ͨ��ʱ��
 * ����:		ter_s	�ն˵�ַ��Ӧ�׽��ֽڵ�
 * ����ֵ: 0 �ɹ�  -1ʧ��
 * */
int UpdateTerSTime(TerSocket * ter_s)
{
	if(NULL == ter_s)
	{
		return -1;
	}

	time_t timer;
	struct tm* t_tm;
	time(&timer);
	t_tm = localtime(&timer);

	//��
	ter_s->last_t[0] = HexToBcd(t_tm->tm_year - 100);
	//��
	ter_s->last_t[1] = HexToBcd(t_tm->tm_mon + 1);
	//��
	ter_s->last_t[2] = HexToBcd(t_tm->tm_mday);
	//ʱ
	ter_s->last_t[3] = HexToBcd(t_tm->tm_hour);
	//��
	ter_s->last_t[4] = HexToBcd(t_tm->tm_min);
	//��
	ter_s->last_t[5] = HexToBcd(t_tm->tm_sec);

	return 0;
}

/*
 * ��������:��ȡ�����ն˸���
 * ����ֵ: ��ǰ�����ն˸���
 * */
unsigned short RouteSocketNum()
{
	unsigned short i = 0;
	TerSocket *p = NULL;

	if(NULL != _FristNode)
	{
		p = _FristNode;
		while(NULL != p)
		{
			i += 1;
			p = p->next;
		}
	}
	return i;
}

/*
 * ��������:��������·�ɽڵ�������·������
 * ����:	node 	�����յ�·�ɽڵ�
 * 		ticker	�ڵ��ڴ���յ���ʱ
 * ����ֵ 0 �ɹ� -1 ʧ��
 * */
int AddRouteRecycle(struct route_node *node, int ticker)
{
	struct route_recycle_node *newp = NULL;
	struct route_recycle_node *p = NULL;
	//struct route_recycle_node *temp = NULL;
	newp = (struct route_recycle_node *)malloc(sizeof(struct route_recycle_node));
	if(NULL == newp)
	{
		perror("malloc route recycle node:");
		return -1;
	}
	newp->route_node = node;
	newp->ticker = ticker;
	newp->next = NULL;

	//�ж��Ƿ��һ����ӽڵ�
	if(NULL == _FristRecycleNode)
	{
		_FristRecycleNode = newp;
	}
	else
	{
		p = _FristRecycleNode;
		while(p->next != NULL)
		{
			//temp = p;
			p = p->next;
		}

		p->next = newp;
	}
	return 0;
}

/*
 * ��������:·�ɽڵ��ڴ�����߳�
 * */
void *RouteNodeMemRecycle(void *arg)
{
	struct route_recycle_node *p = NULL;
	struct route_recycle_node *temp = NULL;
	SocketRv * pp = NULL;
	p = _FristRecycleNode;
	temp = p;

	//���뿴�Ź�
	int wdt_id = 0;
	wdt_id = apply_watch_dog();

	if(wdt_id < 0)
	{
//		printf("usart0 apply dog error\n");
		system("reboot");
	}

	printf("ROUTE��NODE WDT  ID  %d\n", wdt_id);
	//���ÿ��Ź�
	set_watch_dog(wdt_id, 6);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��
		sleep(1);
		pthread_mutex_lock(&(_route_recycle_mutex));
		while(p != NULL)
		{
			if(p->ticker == 0)
			{
				if(p == _FristRecycleNode)	//���յĽڵ��ǵ�һ���ڵ�
				{
					_FristRecycleNode = temp->next;
				}
				else
				{
					temp->next = p->next;
				}

				pp = (SocketRv *)(p->route_node->rvaddr);

				if(p->route_node->rvaddr != NULL)
				{
					pthread_mutex_destroy(&pp->analy_mutex);
					free(pp);
					pp = NULL;
				}
				if(p->route_node != NULL)
				{
					pthread_mutex_destroy(&p->route_node->write_mutex);
					free(p->route_node);
				}
				if(p != NULL)
				{
					free(p);
					p = NULL;
				}
			}
			else
			{
				p->ticker--;
				temp = p;
				p = p->next;
			}
		}
		pthread_mutex_unlock(&(_route_recycle_mutex));
	}
	pthread_exit(NULL);
}
