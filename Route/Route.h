/*
 * Route.h
 *
 *  Created on: 2016年6月29日
 *      Author: j
 */

#ifndef ROUTE_ROUTE_H_
#define ROUTE_ROUTE_H_

#include "SysPara.h"
#include "pthread.h"
#include <errno.h>
#include <signal.h>

typedef struct route_node
{
	unsigned char Ter[TER_ADDR_LEN];	//终端地址
	int	s;								//套接字
	unsigned char last_t[TIME_FRA_LEN];	//最后一次通信时间
	int ticker;							//心跳倒计时
	void *rvaddr;						//接受缓存在堆中的位置
	pthread_mutex_t write_mutex;		//写socket时的互斥信号量
	struct route_node *next;			//下一个节点
}TerSocket;								//终端地址对应套接字

struct route_recycle_node
{
	struct route_node *route_node;		//路由节点地址
	int ticker;							//回收倒计时
	struct route_recycle_node *next;	//下一个待回收节点
};										//待回收的路由节点

#define ROUTE_RECYCLE_TICKER	10		//路由回收倒计时  10S

#ifdef _ROUTE_C_

TerSocket *_FristNode;			//第一个路由节点地址
pthread_mutex_t route_mutex;	//路由锁

struct route_recycle_node *_FristRecycleNode;	//第一个回收节点
pthread_mutex_t _route_recycle_mutex;			//回收路由节点链表锁

#endif

TerSocket *AccordTerSeek(unsigned char *ter);
TerSocket *AccordSocketSeek(int s);
TerSocket *AccordNumSeek(int num);
TerSocket *AddRoute(unsigned char *ter, int s, int ticker, void *addr);
void AccordSocketDele(int s);
void AccordTerDele(unsigned char *ter);
int AddRouteRecycle(struct route_node *node, int ticker);
void *RouteNodeMemRecycle(void *arg);
unsigned short RouteSocketNum();
int UpdateTerSTime(TerSocket * ter_s);
//void AccordTickerDele(TerSocket *frist);

#ifndef _ROUTE_C_

extern TerSocket *_FristNode;		//第一个路由节点地址
extern pthread_mutex_t route_mutex;	//路由锁

extern struct route_recycle_node *_FristRecycleNode;	//第一个回收节点
extern pthread_mutex_t _route_recycle_mutex;			//回收路由节点链表锁

#endif

#endif /* ROUTE_ROUTE_H_ */
