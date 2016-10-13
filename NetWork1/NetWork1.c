/*
 * NetWork1.c
 *
 *  Created on: 2016��8��9��
 *      Author: j
 */
#define	_NETWORK1_C_
#include "NetWork1.h"
#undef	_NETWORK1_C_

#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "Route.h"
#include <stdlib.h>
#include "Route.h"
#include "PthreadPool.h"
#include "DL376_1.h"
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include "CommLib.h"
#include "HLDWatchDog.h"

//��ʼ��epoll��Ҫ�ı���
static int topup_epoll;
static struct epoll_event top_evt[TOPUP_MAX];

/*
 *��������:���ݳ�ֵ�ն˵�ַ�������Ľڵ��ַ
 *����:	ter		��ֵ�ն˵��ն˵�ַ
 *����ֵ:NULL	ʧ��	 !NULL ���ҵ��Ľڵ��ַ
 * */
struct topup_node *AccordTerFind(unsigned char *ter)
{
	if(NULL == _FristTop)
		return NULL;
	struct topup_node *p = NULL;
	int ret = 0;
	p = _FristTop;
	while(NULL != p)
	{
		ret = CompareUcharArray(p->ter, ter, TER_ADDR_LEN);
		if(1 == ret)
			break;
		p = p->next;
	}
	return p;
}

/*
 * ��������:�����׽��ֲ��ҳ�ֵ�ն˽ڵ��ֵ
 * ����:	s		�׽���
 * ����ֵ:	NULL ʧ�� ��= NULL  �ڵ��ַ
 * */
struct topup_node *AccordSocketFind(int s)
{
	if(NULL == _FristTop)
		return NULL;
	struct topup_node *p = NULL;
	p = _FristTop;
	while(NULL != p)
	{
		if(p->s == s)
			break;
		p = p->next;
	}
	return p;
}

/*
 * ��������:��ӳ�ֵ�ն˵���ֵ�ն�����
 * ����:	ter		��ֵ�ն��ն˵�ַ
 * 		s		�׽��ֵ�ַ
 * 		ticker	��������ʱ
 * ����ֵ: NULL ʧ�� ��NULL �½ڵ�ĵ�ַ
 * */
struct topup_node *AddTopup(unsigned char *ter, int s, int ticker)
{
	//��һ�����
	if(NULL == _FristTop)
	{
		_FristTop = (struct topup_node *)malloc(sizeof(struct topup_node));
		if(NULL == _FristTop)
		{
			perror("malloc new top up node:");
			return NULL;
		}
		memcpy(_FristTop->ter, ter, TER_ADDR_LEN);
		_FristTop->s = s;
		_FristTop->ticker = ticker;
		memset(&_FristTop->rvbuf, 0, sizeof(TopUpRv));
		pthread_mutex_init(&(_FristTop->rvbuf.bufmutex), NULL);
		_FristTop->next = NULL;
		pthread_mutex_init(&(_FristTop->write_mutex), NULL);
		return _FristTop;
	}

	struct topup_node *p = _FristTop;

	//����������β
	while(NULL != p->next)
	{
		p = p->next;
	}

	struct topup_node *newp = (struct topup_node *)malloc(sizeof(struct topup_node));
	if(NULL == newp)
	{
		perror("malloc top up node:");
		return NULL;
	}

	//��ӽڵ㵽����
	p->next = newp;
	memcpy(newp->ter, ter, TER_ADDR_LEN);
	newp->s = s;
	newp->ticker = ticker;
	memset(&newp->rvbuf, 0, sizeof(TopUpRv));
	pthread_mutex_init(&(newp->rvbuf.bufmutex), NULL);
	newp->next = NULL;
	pthread_mutex_init(&(newp->write_mutex), NULL);
	return newp;
}

/*
 * ��������:����socket ɾ����ֵ�ն˽ڵ�
 * ����:	s		��ɾ���ڵ���׽���
 * ����ֵ:��
 * */
void AccordSocketDeleTop(int s)
{
	if(NULL == _FristTop)
		return;
	struct topup_node *p = _FristTop;
	struct topup_node *temp = p;
	while(NULL != p)
	{
		if(s == p->s)
			break;
		temp = p;
		p = p->next;
	}

	if(NULL == p)
		return;

	pthread_mutex_lock(&_topup_recycle_mutex);
	AddTopupRecycle(p, ROUTE_RECYCLE_TICKER);
	pthread_mutex_unlock(&_topup_recycle_mutex);

	//������ֻ��һ���ڵ㣬ɾ����һ���ڵ�
	if(p == _FristTop && NULL == p->next)
	{
		_FristTop = NULL;
	}
	else if(p == _FristTop)		//ɾ����һ���ڵ�
	{
		_FristTop = p->next;
	}
	else	//ɾ����һ���ڵ��Ժ�Ľڵ�
	{
		temp->next = p->next;
	}
}

/*
 * ��������:�����ն˵�ַɾ���ڵ�
 * */
void AccordTerDeleTop(unsigned char *ter)
{
	if(NULL == _FristTop)
		return;
	struct topup_node *p = _FristTop;
	struct topup_node *temp = p;

	int ret = 0;
	while(NULL != p)
	{
		ret = CompareUcharArray(p->ter, ter, TER_ADDR_LEN);
		if(1 == ret)
			break;
		temp = p;
		p = p->next;
	}
	if(NULL == p)
		return;

	pthread_mutex_lock(&_topup_recycle_mutex);
	AddTopupRecycle(p, ROUTE_RECYCLE_TICKER);
	pthread_mutex_unlock(&_topup_recycle_mutex);

	//������ֻ��һ���ڵ㣬ɾ����һ���ڵ�
	if(p == _FristTop && NULL == p->next)
	{
		_FristTop = NULL;
	}
	else if(p == _FristTop)		//ɾ����һ���ڵ�
	{
		_FristTop = p->next;
	}
	else	//ɾ����һ���ڵ��Ժ�Ľڵ�
	{
		temp->next = p->next;
	}
}

/*
 * ��������:�������ճ�ֵ�ն˽ڵ�������������
 * ����:	node	�����յĳ�ֵ�ն˽ڵ�
 * 		ticker	�ڵ��ڴ���յ���ʱ
 * ����ֵ:0 �ɹ� -1 ʧ��
 * */
int AddTopupRecycle(struct topup_node *node, int ticker)
{
	struct topup_recycle_node *p = NULL;
	struct topup_recycle_node *newp = NULL;

	newp = (struct topup_recycle_node *)malloc(sizeof(struct topup_recycle_node));
	if(NULL == newp)
	{
		perror("malloc top up recycle node:");
		return -1;
	}
	newp->topup_node = node;
	newp->ticker = ticker;
	newp->next = NULL;

	//�ж��Ƿ��ǵ�һ����ӽڵ�
	if(NULL == _FristRecycleTopNode)
	{
		_FristRecycleTopNode = newp;
	}
	else
	{
		p = _FristRecycleTopNode;
		while(p->next != NULL)
		{
			p = p->next;
		}
		p->next = newp;
	}
	return 0;
}

/*
 * ��������:��ֵ�ն˽ڵ��ڴ�����߳�
 * */
void *TopupNodeMemRecycle(void *arg)
{
	struct topup_recycle_node *p = NULL;
	struct topup_recycle_node *temp = NULL;
	p = _FristRecycleTopNode;
	temp = p;

	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&(_topup_recycle_mutex));
		while(p != NULL)
		{
			if(p->ticker == 0)
			{
				if(p == _FristRecycleTopNode)	//���սڵ��ǵ�һ���ڵ�
				{
					_FristRecycleTopNode = temp->next;
				}
				else
				{
					temp->next = p->next;
				}
				if(p->topup_node != NULL)
				{
					pthread_mutex_destroy(&p->topup_node->rvbuf.bufmutex);
					pthread_mutex_destroy(&p->topup_node->write_mutex);
					free(p->topup_node);
					p->topup_node = NULL;
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
		pthread_mutex_unlock(&(_topup_recycle_mutex));
	}

	pthread_exit(NULL);
}

/*
 * ��������:��ֵ�ն���������ʱ����߳�
 * ����:	arg	����epoll��ؼ��Ͼ��
 * */
void *TopupSocketTicker(void *arg)
{
	int epoll_fd = *(int *)arg;
	struct topup_node *p = NULL;
	struct topup_node *temp = NULL;
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&(topup_node_mutex));
		p = _FristTop;
		temp = p;
		if(NULL == p)
		{
			//���� ����ֹ����
			pthread_mutex_unlock(&(topup_node_mutex));
			continue;
		}
		while(NULL != p)
		{
			p->ticker--;
			//�����������ʱΪ0��ɾ�����׽���
			if(0 == p->ticker)
			{
				if(p == _FristTop)
					_FristTop = temp->next;
				else
					temp->next = p->next;
				//ɾ��epoll�����е��׽���
				del_event(epoll_fd, p->s, EPOLLIN);
				//�ر�ɾ�����׽���
				close(p->s);

				//����ڵ���������ȴ������̻߳���
				pthread_mutex_lock(&_topup_recycle_mutex);
				AddTopupRecycle(p, ROUTE_RECYCLE_TICKER);
				pthread_mutex_unlock(&_topup_recycle_mutex);
				p = temp->next;
			}
			else
			{
				temp = p;
				p = p->next;
			}
		}
		pthread_mutex_unlock(&(topup_node_mutex));
	}

	pthread_exit(NULL);
}

/*
 * ��������:������̫��1����
 * ����:	arg		��ֵ�ն˽ڵ�
 * ����ֵ:��
 * */
void *NetWork1Job0(void *arg)
{
	struct topup_node *ter_s = (struct topup_node *)arg;
	struct topup_node *p1 = NULL;
	tp3761Buffer outbuf;
	tpFrame376_1 rvframe3761;
	tpFrame376_1 snframe3761;
	unsigned char ter[4] = {0};
	int ret = -1;
	int Len = 0;
	while(1)
	{
		usleep(10);
		pthread_mutex_lock(&(ter_s->rvbuf.bufmutex));
		ret = ProtoAnaly_Get376_1BufFromCycBuf(ter_s->rvbuf.buf, SOCKET_RV_MAX, &ter_s->rvbuf.r, \
				&ter_s->rvbuf.w, &outbuf);
		if(-1 == ret)
		{
			pthread_mutex_unlock(&(ter_s->rvbuf.bufmutex));
			break;
		}
		pthread_mutex_unlock(&(ter_s->rvbuf.bufmutex));
		DL376_1_LinkFrame(&outbuf, &rvframe3761);
		if(true == rvframe3761.IsHaving)
		{
			//��ά����ֵ�ն�����
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//�鿴�������Ƿ������ͬ�ն˵�ַ
			pthread_mutex_lock(&(topup_node_mutex));
			p1 = AccordTerFind(ter);
			pthread_mutex_unlock(&(topup_node_mutex));
			if(NULL != p1)	//����
			{
				//�ն˵�ַ���ڵ�λ���Ƿ��Ǹ��׽������ڵ�λ��
				if(p1 != ter_s)	//����
				{
					//�޸��׽��ֶ�Ӧ���ն˵�ַ
					memset(p1->ter, 0xFF, TER_ADDR_LEN);
					memcpy(ter_s->ter, ter, TER_ADDR_LEN);
				}
			}
			else	//������
			{
				//�����ն�����׽���ƥ��
				memcpy(ter_s->ter, ter, TER_ADDR_LEN);
			}
			DL3761_Process_Response(&rvframe3761, &snframe3761);
		}

		if(true == snframe3761.IsHaving)
		{
			if(0 == DL3761_Protocol_LinkPack(&snframe3761, &outbuf))
			{
				Len = outbuf.Len;
				pthread_mutex_lock(&(ter_s->write_mutex));
				while(1)
				{
					ret = write(ter_s->s, outbuf.Data, Len);
					if(ret < 0)
					{
						break;
					}
					Len -= ret;
					if(0 <= Len)
						break;
				}
				pthread_mutex_unlock(&(ter_s->write_mutex));
			}
		}
	}
	return NULL;
}

/*
 *��������:�����߳�1
 * */
void *NetWork1(void *arg)
{
	//���뿴�Ź�
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��
	printf("Network1 wdt id %d\n",wdt_id);

	//��ʼ����ֵ�ն�������(��ֵ�ն����׽��ֶ�Ӧ����)
	if(pthread_mutex_init(&(topup_node_mutex), NULL))
	{
		perror("topup_node_mutex init:");
		pthread_exit(NULL);
	}

	//��ʼ����ֵ�ն˽ڵ����������
	if(pthread_mutex_init(&(_topup_recycle_mutex), NULL))
	{
		perror("_topup_recycle_mutex:");
		pthread_exit(NULL);
	}

	//����epoll
	topup_epoll = epoll_create(TOPUP_MAX);
	if(0 > topup_epoll)
	{
		perror("top up epoll create:");
		pthread_exit(NULL);
	}

	//��ʼ�������
	int s = init_server(NULL, _RunPara.IpPort.TopPort, TOPUP_MAX);
	if(0 > s)
	{
		close(topup_epoll);
		pthread_exit(NULL);
	}

	//�����socket ������
	if(0 > set_nonblock(s))
	{
		close(topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	printf("wait for a new top up connect ...\n");

	//����epoll�����¼�
	if(0 > add_event(topup_epoll, s, EPOLLIN))
	{
		perror("add_even");
		close(topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	//������ֵ�ն�����ά���߳�
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, TopupSocketTicker, (void*)&topup_epoll))
	{
		perror("create TopupSocketTicker:");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	//������ֵ�ն˽ڵ��ڴ�����߳�
	pthread_t ppt;
	if(0 != pthread_create(&ppt, NULL, TopupNodeMemRecycle, NULL))
	{
		perror("TopupNodeMemRecycle");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	struct topup_node *p = NULL;
	struct topup_node *ter_s = NULL;
	int i = 0;
	int j = 0;
	int l = 0;
	int len = 0;
	int k = 3;	//��ȡ�������Դ���
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��
		//����epoll�е��׽���
		int num = epoll_wait(topup_epoll, top_evt, TOPUP_MAX, 3000);
		if(0 > num)
		{
			perror("top epoll wait:");
			break;
		}
		else if(0 == num)
		{
			continue;
		}
		feed_watch_dog(wdt_id);	//ι��

		//���������Ѿ��������¼�
		for(i = 0; i < num; i++)
		{
			feed_watch_dog(wdt_id);	//ι��
			if(top_evt[i].data.fd == s)
			{
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(addr));
				socklen_t addrlen = sizeof(addr);
				int rws = accept(s, (struct sockaddr *)&addr, &addrlen);
				if(0 > rws)
				{
					perror("accept");
					continue;
				}
				printf("new top up socket %d\n", rws);
				if(0 > set_nonblock(rws))
				{
					close(rws);
					continue;
				}

				if(0 > add_event(topup_epoll, rws, EPOLLIN))
				{
					perror("add_even");
					close(rws);
					continue;
				}
				else
				{
					printf("new top up con: %s:%d\n", \
					inet_ntoa(addr.sin_addr),\
					ntohs(addr.sin_port));

					//�����������Ӽ��뵽��ֵ�ն�������
					feed_watch_dog(wdt_id);	//ι��
					pthread_mutex_lock(&(topup_node_mutex));
					p = AccordSocketFind(rws);
					if(NULL == p)
					{
						if(NULL == AddTopup(temp_ter, rws, SOCKET_TICKER))
							printf("Add top up node erro\n");
					}
					pthread_mutex_unlock(&(topup_node_mutex));
				}
			}
			else
			{
				feed_watch_dog(wdt_id);	//ι��
				pthread_mutex_lock(&(topup_node_mutex));
				ter_s = AccordSocketFind(top_evt[i].data.fd);
				pthread_mutex_unlock(&(topup_node_mutex));
				if(ter_s == NULL)
				{
					del_event(topup_epoll, top_evt[i].data.fd, EPOLLIN);
					close(top_evt[i].data.fd);
					continue;
				}

				//��ȡ3�ζ���ȡ����������ر��׽���
				k = 3;
				while(k--)
				{
					len = read(top_evt[i].data.fd, buf, NETWOEK_R_MAX);
					//len = recv(r_evt[i].data.fd, buf, NETWOEK_R_MAX, MSG_WAITALL);
					if(len > 0)
						break;
				}

				if(len <= 0)
				{
					if(0 > del_event(topup_epoll, top_evt[i].data.fd, EPOLLIN))
					{
						perror("del_event");
					}
					close(top_evt[i].data.fd);
					pthread_mutex_lock(&(topup_node_mutex));
					AccordSocketDeleTop(top_evt[i].data.fd);
					pthread_mutex_unlock(&(topup_node_mutex));

					continue;
				}

				pthread_mutex_lock(&(ter_s->rvbuf.bufmutex));
				if(ter_s->rvbuf.w >= ter_s->rvbuf.r)  //д>=��
				{
					l = SOCKET_RV_MAX - ter_s->rvbuf.w + ter_s->rvbuf.r;
				}
				else if(ter_s->rvbuf.w < ter_s->rvbuf.r) //д<��
				{
					l = ter_s->rvbuf.w - ter_s->rvbuf.r;
				}
				if(l < len)
				{
					pthread_mutex_unlock(&(ter_s->rvbuf.bufmutex));
					continue;
				}

				//д������
				printf("top up up:");
				for(j=0; j<len; j++)
				{
					printf(" %02x",buf[j]);
					ter_s->rvbuf.buf[ter_s->rvbuf.w] = buf[j];
					ter_s->rvbuf.w++;
					ter_s->rvbuf.w %= SOCKET_RV_MAX;
				}
				printf("\n");
				ter_s->ticker = SOCKET_TICKER;
				pthread_mutex_unlock(&(ter_s->rvbuf.bufmutex));
				threadpool_add_job(_Threadpool, NetWork1Job0, (void*)ter_s);
			}
		}
	}
	close(topup_epoll);
	close(s);
	pthread_join(pt, NULL);
	pthread_join(ppt, NULL);
	pthread_exit(NULL);
}
