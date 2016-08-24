
/*
 * Route.c
 *
 *  Created on: 2016年6月29日
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

/*
 * 函数功能:根据终端地址查找节点地址
 * 参数:	Ter		终端地址
 * 返回值: NULL 失败 !NULL 查找到的节点地址
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
 * 函数功能:根据套接字查找节点地址
 * 参数:	s		套接字
 * 返回值:NULL 失败  !=NULL 节点地址
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
 * 函数功能:添加路由
 * 参数:ter		终端地址
 * 		s		套接字地址
 * 		ticker	心跳倒计时
 * 		addr	接受缓存在堆中的位置
 *返回值 NULL 失败  ！NULL 新节点的地址
 * */
TerSocket *AddRoute(unsigned char *ter, int s, int ticker, void *addr)
{
	//第一次添加
	if(NULL == _FristNode)
	{
		_FristNode = (TerSocket*)malloc(sizeof(TerSocket));
		if(NULL == _FristNode)
		{
			printf("route.c 87\n");
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

	//遍历到链表末尾
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

	//添加节点到链表
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
 *函数功能:根据socket 删除节点
 *参数:	s		待删除节点的套接字
 *返回值: 无
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

	//链表中只有一个节点，删除第一个节点
	if(p == _FristNode && NULL == p->next)
	{
		_FristNode = NULL;
	}
	else if(p == _FristNode)	//删除第一个节点
	{
		_FristNode = p->next;
	}
	else
	{
		//删除第一个节点以后的节点
		temp->next = p->next;
	}
}

/*
 * 函数功能:根据终端地址删除节点
 * 参数:
 * 		ter		终端地址
 * 返回值:无
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

	//链表中只有一个节点，删除第一个节点
	if(p == _FristNode && NULL == p->next)
	{
		_FristNode = NULL;
	}
	else if(p == _FristNode)	//删除第一个节点
	{
		_FristNode = p->next;
	}
	else
	{
		//删除第一个节点以后的节点
		temp->next = p->next;
	}
}


/*
 * 函数功能:将待回收路由节点加入回收路由链表
 * 参数:	node 	待回收的路由节点
 * 		ticker	节点内存回收倒计时
 * 返回值 0 成功 -1 失败
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

	//判断是否第一次添加节点
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
 * 函数功能:路由节点内存回收线程
 * */
void *RouteNodeMemRecycle(void *arg)
{
	struct route_recycle_node *p = NULL;
	struct route_recycle_node *temp = NULL;
	SocketRv * pp = NULL;
	p = _FristRecycleNode;
	temp = p;

	//申请看门狗
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
//		printf("usart0 apply dog error\n");
		system("reboot");
	}
	//设置看门狗
	set_watch_dog(wdt_id, 6);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		sleep(1);
		pthread_mutex_lock(&(_route_recycle_mutex));
		while(p != NULL)
		{
			if(p->ticker == 0)
			{
				if(p == _FristRecycleNode)	//回收的节点是第一个节点
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
