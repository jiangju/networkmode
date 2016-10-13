/*
 * NetWork1.c
 *
 *  Created on: 2016年8月9日
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

//初始化epoll需要的变量
static int topup_epoll;
static struct epoll_event top_evt[TOPUP_MAX];

/*
 *函数功能:根据充值终端地址查找它的节点地址
 *参数:	ter		充值终端的终端地址
 *返回值:NULL	失败	 !NULL 查找到的节点地址
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
 * 函数功能:根据套接字查找充值终端节点地值
 * 参数:	s		套接字
 * 返回值:	NULL 失败 ！= NULL  节点地址
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
 * 函数功能:添加充值终端到充值终端链表
 * 参数:	ter		充值终端终端地址
 * 		s		套接字地址
 * 		ticker	心跳倒计时
 * 返回值: NULL 失败 ！NULL 新节点的地址
 * */
struct topup_node *AddTopup(unsigned char *ter, int s, int ticker)
{
	//第一次添加
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

	//遍历到链表尾
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

	//添加节点到链表
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
 * 函数功能:根据socket 删除充值终端节点
 * 参数:	s		待删除节点的套接字
 * 返回值:无
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

	//链表中只有一个节点，删除第一个节点
	if(p == _FristTop && NULL == p->next)
	{
		_FristTop = NULL;
	}
	else if(p == _FristTop)		//删除第一个节点
	{
		_FristTop = p->next;
	}
	else	//删除第一个节点以后的节点
	{
		temp->next = p->next;
	}
}

/*
 * 函数功能:根据终端地址删除节点
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

	//链表中只有一个节点，删除第一个节点
	if(p == _FristTop && NULL == p->next)
	{
		_FristTop = NULL;
	}
	else if(p == _FristTop)		//删除第一个节点
	{
		_FristTop = p->next;
	}
	else	//删除第一个节点以后的节点
	{
		temp->next = p->next;
	}
}

/*
 * 函数功能:将待回收充值终端节点加入回收链表中
 * 参数:	node	待回收的充值终端节点
 * 		ticker	节点内存回收倒计时
 * 返回值:0 成功 -1 失败
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

	//判断是否是第一次添加节点
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
 * 函数功能:充值终端节点内存回收线程
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
				if(p == _FristRecycleTopNode)	//回收节点是第一个节点
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
 * 函数功能:充值终端心跳倒计时监控线程
 * 参数:	arg	传递epoll监控集合句柄
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
			//解锁 ，防止死锁
			pthread_mutex_unlock(&(topup_node_mutex));
			continue;
		}
		while(NULL != p)
		{
			p->ticker--;
			//如果心跳倒计时为0则删除该套接字
			if(0 == p->ticker)
			{
				if(p == _FristTop)
					_FristTop = temp->next;
				else
					temp->next = p->next;
				//删除epoll集合中的套接字
				del_event(epoll_fd, p->s, EPOLLIN);
				//关闭删除的套接字
				close(p->s);

				//加入节点回收链表，等待回收线程回收
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
 * 函数功能:处理以太网1接受
 * 参数:	arg		充值终端节点
 * 返回值:无
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
			//先维护充值终端链表
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//查看链表中是否存在相同终端地址
			pthread_mutex_lock(&(topup_node_mutex));
			p1 = AccordTerFind(ter);
			pthread_mutex_unlock(&(topup_node_mutex));
			if(NULL != p1)	//存在
			{
				//终端地址所在的位置是否是该套接字所在的位置
				if(p1 != ter_s)	//不是
				{
					//修改套接字对应的终端地址
					memset(p1->ter, 0xFF, TER_ADDR_LEN);
					memcpy(ter_s->ter, ter, TER_ADDR_LEN);
				}
			}
			else	//不存在
			{
				//将该终端与该套接字匹配
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
 *函数功能:网络线程1
 * */
void *NetWork1(void *arg)
{
	//申请看门狗
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//喂狗
	printf("Network1 wdt id %d\n",wdt_id);

	//初始化充值终端链接锁(充值终端与套接字对应链表)
	if(pthread_mutex_init(&(topup_node_mutex), NULL))
	{
		perror("topup_node_mutex init:");
		pthread_exit(NULL);
	}

	//初始化充值终端节点回收链表锁
	if(pthread_mutex_init(&(_topup_recycle_mutex), NULL))
	{
		perror("_topup_recycle_mutex:");
		pthread_exit(NULL);
	}

	//建立epoll
	topup_epoll = epoll_create(TOPUP_MAX);
	if(0 > topup_epoll)
	{
		perror("top up epoll create:");
		pthread_exit(NULL);
	}

	//初始化服务端
	int s = init_server(NULL, _RunPara.IpPort.TopPort, TOPUP_MAX);
	if(0 > s)
	{
		close(topup_epoll);
		pthread_exit(NULL);
	}

	//服务端socket 非阻塞
	if(0 > set_nonblock(s))
	{
		close(topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	printf("wait for a new top up connect ...\n");

	//加入epoll监听事件
	if(0 > add_event(topup_epoll, s, EPOLLIN))
	{
		perror("add_even");
		close(topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	//建立充值终端心跳维护线程
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, TopupSocketTicker, (void*)&topup_epoll))
	{
		perror("create TopupSocketTicker:");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	//建立充值终端节点内存回收线程
	pthread_t ppt;
	if(0 != pthread_create(&ppt, NULL, TopupNodeMemRecycle, NULL))
	{
		perror("TopupNodeMemRecycle");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	struct topup_node *p = NULL;
	struct topup_node *ter_s = NULL;
	int i = 0;
	int j = 0;
	int l = 0;
	int len = 0;
	int k = 3;	//读取接受重试次数
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		//监听epoll中的套接字
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
		feed_watch_dog(wdt_id);	//喂狗

		//遍历处理已经发生的事件
		for(i = 0; i < num; i++)
		{
			feed_watch_dog(wdt_id);	//喂狗
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

					//将新来的链接加入到充值终端链表中
					feed_watch_dog(wdt_id);	//喂狗
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
				feed_watch_dog(wdt_id);	//喂狗
				pthread_mutex_lock(&(topup_node_mutex));
				ter_s = AccordSocketFind(top_evt[i].data.fd);
				pthread_mutex_unlock(&(topup_node_mutex));
				if(ter_s == NULL)
				{
					del_event(topup_epoll, top_evt[i].data.fd, EPOLLIN);
					close(top_evt[i].data.fd);
					continue;
				}

				//读取3次都读取不到数据则关闭套接字
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
				if(ter_s->rvbuf.w >= ter_s->rvbuf.r)  //写>=读
				{
					l = SOCKET_RV_MAX - ter_s->rvbuf.w + ter_s->rvbuf.r;
				}
				else if(ter_s->rvbuf.w < ter_s->rvbuf.r) //写<读
				{
					l = ter_s->rvbuf.w - ter_s->rvbuf.r;
				}
				if(l < len)
				{
					pthread_mutex_unlock(&(ter_s->rvbuf.bufmutex));
					continue;
				}

				//写入数据
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
