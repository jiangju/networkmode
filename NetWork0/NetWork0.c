/*
 * NetWork.c
 *
 *  Created on: 2016��7��18��
 *      Author: j
 */
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
#include "NetWork0.h"
#include <HLDWatchDog.h>
//#include <sys/time.h>

//��ʼ��epoll��Ҫ�ı���
static int epollfd;
static struct epoll_event r_evt[EVTMAX];

/*
 * ��������:��ʼ�������
 * ����:	ipstr	�����ip��ַ	�ַ�����ʽ192.168.0.66
 * 		port	�˿ں�
 * 		backlog ���������ļ�������
 * ����ֵ:>=0�׽��� -1ʧ��
 * */
int init_server(const char *ipstr, u_short port, int backlog)
{
	//��ȡ������׽���
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if(0 > s)
	{
		perror("socket");
		return -1;
	}

	//����Ϊ��socket �ر�ʱ����������ʹ�õ�ǰ�˿�
	int used = 1;
	if(0 > setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(int))){
		perror("setsockopt");
		return -1;
	}

	//��Ԫ��
	struct sockaddr_in addr = {
		.sin_family	= PF_INET,
		.sin_port	= htons(port),
		.sin_addr = {
			.s_addr = (ipstr == NULL) ? INADDR_ANY : inet_addr(ipstr),
		}
	};

	//���׽���
	socklen_t addrlen = sizeof(addr);
	if(0 > bind(s, (struct sockaddr *)&addr, addrlen)){
		perror("bind");
		return -1;
	}

	//�����׽���
	if(0 > listen (s, backlog)){
		perror("listen");
		return -1;
	}

	return s;
}

/*
 * ��������:�����ļ�������Ϊ������
 * ����:	fd	�ļ�������
 * ����ֵ: 0 �ɹ� -1 ʧ��
 * */
int set_nonblock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if(0 > flag){
		perror("fcntl");
		return -1;
	}

	flag |= O_NONBLOCK;

	if(0 > fcntl(fd, F_SETFL, flag)){
		perror("fcntl-1");
		return -1;
	}

	return 0;
}

/*
 * ��������:�����Ӧ�׽��ּ����¼�������
 * ����:	epfd	����epollʱ���ص�epoll���
 * 		fd		�׽���
 * 		event	�¼�����
 * ����ֵ: 0 �ɹ� -1 ʧ��
 * */
int add_event(int epfd, int fd, unsigned int event)
{
	struct epoll_event evt = {
		.events = event,
		.data = {
			.fd = fd,
		},
	};
	return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
}

int del_event(int epfd, int fd, unsigned int event)
{
	struct epoll_event evt = {
		.events = event,
		.data = {
			.fd = fd,
		},
	};

	return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &evt);
}

/*
 * ��������:��������ʱ����߳�
 * ����:	arg	����epoll��ؼ��Ͼ��
 * */
void *SocketTicker(void *arg)
{
	int epoll_fd = *(int *)arg;
	TerSocket *p = NULL;
	TerSocket *temp = NULL;
	int i = 0;

	//���뿴�Ź�
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	printf("NETWOER0  SOCKER��TICKER WDT ID %d\n", wdt_id);
	//���ÿ��Ź�
	set_watch_dog(wdt_id, 10);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��

		i = 0;
		sleep(1);
		feed_watch_dog(wdt_id);	//ι��
		pthread_mutex_lock(&(route_mutex));
		p = _FristNode;
		temp = p;

		if(NULL == p)
		{
			//��������ֹ����
			pthread_mutex_unlock(&(route_mutex));
			continue;
		}
		while(NULL != p)
		{
			p->ticker--;
			//�����������ʱΪ0 ��ɾ�����׽���
			if(0 >= p->ticker)
			{
				if(p == _FristNode)
				{
					_FristNode = _FristNode->next;
					temp = _FristNode;
					//ɾ��epoll �����е��׽���
					del_event(epoll_fd, p->s, EPOLLIN);
					//�ر�ɾ�����׽���
					close(p->s);

					//����ڵ���������ȴ������̻߳���
					feed_watch_dog(wdt_id);	//ι��
					pthread_mutex_lock(&(_route_recycle_mutex));
					AddRouteRecycle(p, ROUTE_RECYCLE_TICKER);
					pthread_mutex_unlock(&(_route_recycle_mutex));
					//
					p = temp;
				}
				else
				{
					temp->next = p->next;
					//ɾ��epoll �����е��׽���
					del_event(epoll_fd, p->s, EPOLLIN);
					//�ر�ɾ�����׽���
					close(p->s);
					//����ڵ���������ȴ������̻߳���
					feed_watch_dog(wdt_id);	//ι��
					pthread_mutex_lock(&(_route_recycle_mutex));
					AddRouteRecycle(p, ROUTE_RECYCLE_TICKER);
					pthread_mutex_unlock(&(_route_recycle_mutex));
					//
					p = temp->next;
				}
			}
			else
			{
				temp = p;
				p = p->next;
				i++;
			}
		}
//		printf("socket num  %d\n",i);
		pthread_mutex_unlock(&(route_mutex));
	}
	pthread_exit(NULL);
}

/*
 *��������:������̫��0����
 *����: arg		�ն˺��׽��ֽڵ�
 *����ֵ:��
 * */
void *NetWorkJob0(void *arg)
{
	TerSocket *ter_s = (TerSocket *)arg;
	SocketRv *s_rv = (SocketRv *)ter_s->rvaddr;
	tp3761Buffer outbuf;
	tpFrame376_1 rvframe3761;
	tpFrame376_1 snframe3761;
	unsigned char ter[4] = {0};
	int ret = -1;
	TerSocket *p1= NULL;
	int Len = 0;
	while(1)
	{
		usleep(1);
		pthread_mutex_lock(&(s_rv->analy_mutex));
		ret = ProtoAnaly_Get376_1BufFromCycBuf(s_rv->buff, SOCKET_RV_MAX, &s_rv->r, &s_rv->w, THR, &outbuf);
		if(-1 == ret)
		{
			pthread_mutex_unlock(&(s_rv->analy_mutex));
			break;
		}
		pthread_mutex_unlock(&(s_rv->analy_mutex));
		DL376_1_LinkFrame(&outbuf, &rvframe3761);
		if(true == rvframe3761.IsHaving)
		{
			//��ά·������
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//�鿴�������Ƿ���ڸ��ն˵�ַ
			pthread_mutex_lock(&(route_mutex));
			p1 = AccordTerSeek(ter);
			pthread_mutex_unlock(&(route_mutex));
			if(NULL != p1)	//����
			{
				//�ն˵�ַ���ڵ�λ���Ƿ��Ǹ��׽������ڵ�λ��
				if(p1 != ter_s)	//����
				{
					//�޸��׽��ֶ�Ӧ���ն˵�ַ
					memset(p1->Ter, 0xFF, TER_ADDR_LEN);
					memcpy(ter_s->Ter, ter, TER_ADDR_LEN);
				}
			}
			else	//������
			{
				//�����ն˵�ַ����׽���ƥ��
				memcpy(ter_s->Ter, ter, TER_ADDR_LEN);
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
 *��������:�����߳�0
 * */
void *NetWork0(void *arg)
{
	//���뿴�Ź�
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��
	printf("NETWORK0  WDT  ID %d\n", wdt_id);

	//��ʼ��·�ɵ���(�ն����׽��ֶ�Ӧ����)
	if(pthread_mutex_init(&(route_mutex), NULL))
	{
		perror("NetWork 167");
		pthread_exit(NULL);
	}

	//��ʼ��·�ɽڵ����������
	if(pthread_mutex_init(&(_route_recycle_mutex), NULL))
	{
		perror("_route_recycle_mutex");
		pthread_exit(NULL);
	}

	//����epoll
	epollfd = epoll_create(NETWORK_MAX_CONNCET);
	if(0 > epollfd)
	{
		perror("epoll_create");
		pthread_exit(NULL);
	}

	//��ʼ�������
	pthread_mutex_lock(&_RunPara.mutex);
	int s = init_server(NULL, _RunPara.IpPort.Port, 128);
	pthread_mutex_unlock(&_RunPara.mutex);
	if(0 > s)
	{
		close(epollfd);
		pthread_exit(NULL);
	}

	//�����socket������
	if(0 > set_nonblock(s))
	{
		close(epollfd);
		close(s);
		pthread_exit(NULL);
	}

	printf("Wait for a new connect...\n");

	//����epoll �����¼�
	if(0 > add_event(epollfd, s, EPOLLIN))
	{
		perror("add_even");
		close(epollfd);
		close(s);
		pthread_exit(NULL);
	}

	//����socket����ά���߳�
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, SocketTicker, (void *)&epollfd))
	{
		perror("NetWork 173");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	//����·�ɽڵ�����߳�
	pthread_t ppt;
	if(0 != pthread_create(&ppt, NULL, RouteNodeMemRecycle, NULL))
	{
		perror("RouteNodeMemRecycle");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	int i = 0;
	int j = 0;
	TerSocket *p = NULL;
	SocketRv *sp = NULL;
	TerSocket *ter_s = NULL;
	SocketRv *s_rv = NULL;
	int len = 0;
	int l = 0;
	int k = 3;		//��ȡ�������Դ���
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��
		//����epoll�е��׽���
		int num = epoll_wait(epollfd, r_evt, EVTMAX, 3000);
		if(0 > num)
		{
			perror("epoll_wait");
			break;
		}
		else if(0 == num)
		{
			continue;
		}

		//���������ѷ������¼�
		for(i = 0; i < num; i++)
		{
			if(r_evt[i].data.fd == s)
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

				if(0 > set_nonblock(rws))
				{
					close(rws);
					continue;
				}

				if(0 > add_event(epollfd, rws, EPOLLIN))
				{
					perror("add_even");
					close(rws);
					continue;
				}
				else
				{
					//�������ӵ��׽��ּ��뵽·��������
					feed_watch_dog(wdt_id);	//ι��
					pthread_mutex_lock(&(route_mutex));
					p = AccordSocketSeek(rws);
					if(NULL == p)
					{
						//Ϊ���������ϵ��׽���������ܻ���
						sp = (SocketRv *)malloc(sizeof(SocketRv));
						if(NULL == sp)
						{
							perror("SocketRv");
							close(rws);
							pthread_mutex_unlock(&(route_mutex));
							continue;
						}
						pthread_mutex_init(&(sp->analy_mutex), NULL);
						//pthread_mutexattr_settype(&(sp->analy_mutex), PTHREAD_MUTEX_RECURSIVE_NP);
						memset(sp, 0x00, sizeof(SocketRv));

						if(NULL == AddRoute(temp_ter, rws, SOCKET_TICKER, (void*)sp))
						{
							printf("Add route erro\n");
						}
					}
					pthread_mutex_unlock(&(route_mutex));
				}
			}
			else
			{
				feed_watch_dog(wdt_id);	//ι��
				pthread_mutex_lock(&(route_mutex));
				ter_s = AccordSocketSeek(r_evt[i].data.fd);
				pthread_mutex_unlock(&(route_mutex));
				if(ter_s == NULL)
				{
					del_event(epollfd, r_evt[i].data.fd, EPOLLIN);
					close(r_evt[i].data.fd);

					continue;
				}
				s_rv = (SocketRv *)ter_s->rvaddr;
				memset(buf, 0, SOCKET_RV_MAX);

				//��ȡ3�ζ���ȡ����������ر��׽���
				k = 3;
				while(k--)
				{
					len = read(r_evt[i].data.fd, buf, NETWOEK_R_MAX);
					//len = recv(r_evt[i].data.fd, buf, NETWOEK_R_MAX, MSG_WAITALL);
					if(len > 0)
						break;
				}

				if(len < 0)
				{
					if(errno == EAGAIN)
					{
					        break;
					}

					if(0 > del_event(epollfd, r_evt[i].data.fd, EPOLLIN))
					{
						perror("del_event");
					}
					close(r_evt[i].data.fd);
					feed_watch_dog(wdt_id);	//ι��
					pthread_mutex_lock(&(route_mutex));
					AccordSocketDele(r_evt[i].data.fd);
					pthread_mutex_unlock(&(route_mutex));

					continue;
				}

				feed_watch_dog(wdt_id);	//ι��
				pthread_mutex_lock(&(s_rv->analy_mutex));
				if(s_rv->w >= s_rv->r)  //д>=��
				{
					l = SOCKET_RV_MAX - s_rv->w + s_rv->r;
				}
				else if(s_rv->w < s_rv->r) //д<��
				{
					l = s_rv->r - s_rv->w;
				}

				if(l < len)
				{
					pthread_mutex_unlock(&(s_rv->analy_mutex));

					continue;
				}
				for(j=0; j<len; j++)
				{
					s_rv->buff[s_rv->w] = buf[j];
					s_rv->w++;
					s_rv->w %= SOCKET_RV_MAX;
				}

				pthread_mutex_unlock(&(s_rv->analy_mutex));

				feed_watch_dog(wdt_id);	//ι��
				pthread_mutex_lock(&(route_mutex));
				ter_s->ticker = SOCKET_TICKER;
				UpdateTerSTime(ter_s);
				pthread_mutex_unlock(&(route_mutex));
				if(-1 == threadpool_add_job(_Threadpool, NetWorkJob0, (void*)ter_s))
				{
					printf("up add job erro\n");
				}
			}
		}
	}
	close(epollfd);
	close(s);
	pthread_join(pt, NULL);
	pthread_join(ppt, NULL);
	pthread_exit(NULL);
}

