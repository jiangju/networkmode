/*
 * NetWork.c
 *
 *  Created on: 2016年7月18日
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

//初始化epoll需要的变量
static int epollfd;
static struct epoll_event r_evt[EVTMAX];

/*
 * 函数功能:初始化服务端
 * 参数:	ipstr	服务端ip地址	字符串格式192.168.0.66
 * 		port	端口号
 * 		backlog 队列中最大的监听数量
 * 返回值:>=0套接字 -1失败
 * */
int init_server(const char *ipstr, u_short port, int backlog)
{
	//获取服务端套接字
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if(0 > s)
	{
		perror("socket");
		return -1;
	}

	//设置为当socket 关闭时，可以立即使用当前端口
	int used = 1;
	if(0 > setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(int))){
		perror("setsockopt");
		return -1;
	}

	//三元组
	struct sockaddr_in addr = {
		.sin_family	= PF_INET,
		.sin_port	= htons(port),
		.sin_addr = {
			.s_addr = (ipstr == NULL) ? INADDR_ANY : inet_addr(ipstr),
		}
	};

	//绑定套接字
	socklen_t addrlen = sizeof(addr);
	if(0 > bind(s, (struct sockaddr *)&addr, addrlen)){
		perror("bind");
		return -1;
	}

	//监听套接字
	if(0 > listen (s, backlog)){
		perror("listen");
		return -1;
	}

	return s;
}

/*
 * 函数功能:设置文件描述符为非阻塞
 * 参数:	fd	文件描述符
 * 返回值: 0 成功 -1 失败
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
 * 函数功能:添加相应套接字监听事件到队列
 * 参数:	epfd	创建epoll时返回的epoll句柄
 * 		fd		套接字
 * 		event	事件类型
 * 返回值: 0 成功 -1 失败
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
 * 函数功能:心跳倒计时监控线程
 * 参数:	arg	传递epoll监控集合句柄
 * */
void *SocketTicker(void *arg)
{
	int epoll_fd = *(int *)arg;
	TerSocket *p = NULL;
	TerSocket *temp = NULL;
	int i = 0;

	//申请看门狗
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	printf("NETWOER0  SOCKER　TICKER WDT ID %d\n", wdt_id);
	//设置看门狗
	set_watch_dog(wdt_id, 10);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗

		i = 0;
		sleep(1);
		feed_watch_dog(wdt_id);	//喂狗
		pthread_mutex_lock(&(route_mutex));
		p = _FristNode;
		temp = p;

		if(NULL == p)
		{
			//解锁，防止死锁
			pthread_mutex_unlock(&(route_mutex));
			continue;
		}
		while(NULL != p)
		{
			p->ticker--;
			//如果心跳倒计时为0 则删除该套接字
			if(0 >= p->ticker)
			{
				if(p == _FristNode)
				{
					_FristNode = _FristNode->next;
					temp = _FristNode;
					//删除epoll 集合中的套接字
					del_event(epoll_fd, p->s, EPOLLIN);
					//关闭删除的套接字
					close(p->s);

					//加入节点回收链表，等待回收线程回收
					feed_watch_dog(wdt_id);	//喂狗
					pthread_mutex_lock(&(_route_recycle_mutex));
					AddRouteRecycle(p, ROUTE_RECYCLE_TICKER);
					pthread_mutex_unlock(&(_route_recycle_mutex));
					//
					p = temp;
				}
				else
				{
					temp->next = p->next;
					//删除epoll 集合中的套接字
					del_event(epoll_fd, p->s, EPOLLIN);
					//关闭删除的套接字
					close(p->s);
					//加入节点回收链表，等待回收线程回收
					feed_watch_dog(wdt_id);	//喂狗
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
 *函数功能:处理以太网0接收
 *参数: arg		终端和套接字节点
 *返回值:无
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
			//先维路由链表
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//查看链表中是否存在该终端地址
			pthread_mutex_lock(&(route_mutex));
			p1 = AccordTerSeek(ter);
			pthread_mutex_unlock(&(route_mutex));
			if(NULL != p1)	//存在
			{
				//终端地址所在的位置是否是该套接字所在的位置
				if(p1 != ter_s)	//不是
				{
					//修改套接字对应的终端地址
					memset(p1->Ter, 0xFF, TER_ADDR_LEN);
					memcpy(ter_s->Ter, ter, TER_ADDR_LEN);
				}
			}
			else	//不存在
			{
				//将该终端地址与该套接字匹配
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
 *函数功能:网络线程0
 * */
void *NetWork0(void *arg)
{
	//申请看门狗
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//喂狗
	printf("NETWORK0  WDT  ID %d\n", wdt_id);

	//初始化路由的锁(终端与套接字对应链表)
	if(pthread_mutex_init(&(route_mutex), NULL))
	{
		perror("NetWork 167");
		pthread_exit(NULL);
	}

	//初始化路由节点回收链表锁
	if(pthread_mutex_init(&(_route_recycle_mutex), NULL))
	{
		perror("_route_recycle_mutex");
		pthread_exit(NULL);
	}

	//建立epoll
	epollfd = epoll_create(NETWORK_MAX_CONNCET);
	if(0 > epollfd)
	{
		perror("epoll_create");
		pthread_exit(NULL);
	}

	//初始化服务端
	pthread_mutex_lock(&_RunPara.mutex);
	int s = init_server(NULL, _RunPara.IpPort.Port, 128);
	pthread_mutex_unlock(&_RunPara.mutex);
	if(0 > s)
	{
		close(epollfd);
		pthread_exit(NULL);
	}

	//服务端socket非阻塞
	if(0 > set_nonblock(s))
	{
		close(epollfd);
		close(s);
		pthread_exit(NULL);
	}

	printf("Wait for a new connect...\n");

	//加入epoll 监听事件
	if(0 > add_event(epollfd, s, EPOLLIN))
	{
		perror("add_even");
		close(epollfd);
		close(s);
		pthread_exit(NULL);
	}

	//建立socket心跳维护线程
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, SocketTicker, (void *)&epollfd))
	{
		perror("NetWork 173");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	//建立路由节点回收线程
	pthread_t ppt;
	if(0 != pthread_create(&ppt, NULL, RouteNodeMemRecycle, NULL))
	{
		perror("RouteNodeMemRecycle");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	int i = 0;
	int j = 0;
	TerSocket *p = NULL;
	SocketRv *sp = NULL;
	TerSocket *ter_s = NULL;
	SocketRv *s_rv = NULL;
	int len = 0;
	int l = 0;
	int k = 3;		//读取接受重试次数
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		//监听epoll中的套接字
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

		//遍历处理已发生的事件
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
					//将新连接的套接字加入到路由链表中
					feed_watch_dog(wdt_id);	//喂狗
					pthread_mutex_lock(&(route_mutex));
					p = AccordSocketSeek(rws);
					if(NULL == p)
					{
						//为本次连接上的套接字申请接受缓存
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
				feed_watch_dog(wdt_id);	//喂狗
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

				//读取3次都读取不到数据则关闭套接字
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
					feed_watch_dog(wdt_id);	//喂狗
					pthread_mutex_lock(&(route_mutex));
					AccordSocketDele(r_evt[i].data.fd);
					pthread_mutex_unlock(&(route_mutex));

					continue;
				}

				feed_watch_dog(wdt_id);	//喂狗
				pthread_mutex_lock(&(s_rv->analy_mutex));
				if(s_rv->w >= s_rv->r)  //写>=读
				{
					l = SOCKET_RV_MAX - s_rv->w + s_rv->r;
				}
				else if(s_rv->w < s_rv->r) //写<读
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

				feed_watch_dog(wdt_id);	//喂狗
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

