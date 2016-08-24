/*
 * NetWork1.h
 *
 *  Created on: 2016年8月9日
 *      Author: j
 */

#ifndef NETWORK1_NETWORK1_H_
#define NETWORK1_NETWORK1_H_

#include <pthread.h>
#include "NetWork0.h"

#define TOPUP_MAX	100	//充值终端最大连接数

typedef struct
{
	unsigned char buf[SOCKET_RV_MAX];	//缓存
	unsigned short r;					//读指针
	unsigned short w;					//写指针
	pthread_mutex_t bufmutex;			//缓存访问锁
}TopUpRv;	//充值终端接收缓存

struct topup_node
{
	int s;								//套接字（充值终端）
	unsigned char ter[TER_ADDR_LEN];	//终端地址
	TopUpRv	rvbuf;						//接收缓存
	int ticker;							//心跳倒计时
	pthread_mutex_t write_mutex;		//写socket时的互斥信号量
	struct topup_node *next;			//下一个充值终端的节点
};										//充值终端节点

struct topup_recycle_node
{
	struct topup_node *topup_node;		//充值终端节点地址
	int ticker;							//回收倒计时
	struct topup_recycle_node *next;	//下一个待回收节点
};										//待回收的充值终端节点

#define TOPUP_RECYCLE_TICKER	10		//充值终端节点回收倒计时 10S

#ifdef	_NETWORK1_C_

struct topup_node *_FristTop;		//第一个充值终端节点
pthread_mutex_t topup_node_mutex;	//充值终端节点锁

struct topup_recycle_node *_FristRecycleTopNode;	//第一个待回收充值节点的待回收节点
pthread_mutex_t _topup_recycle_mutex;			//待回收链表锁

#endif

#ifndef	_NETWORK1_C_

extern struct topup_node *_FristTop;		//第一个充值终端节点
extern pthread_mutex_t topup_node_mutex;	//充值终端节点锁

extern struct topup_recycle_node *_FristRecycleTopNode;	//第一个待回收充值节点的待回收节点
extern pthread_mutex_t _topup_recycle_mutex;			//待回收链表锁

#endif

struct topup_node *AccordTerFind(unsigned char *ter);
struct topup_node *AccordSocketFind(int s);
struct topup_node *AddTopup(unsigned char *ter, int s, int ticker);
void AccordSocketDeleTop(int s);
void AccordTerDeleTop(unsigned char *ter);
int AddTopupRecycle(struct topup_node *node, int ticker);
void *TopupNodeMemRecycle(void *arg);
void *TopupSocketTicker(void *arg);

void *NetWork1(void *arg);

#endif /* NETWORK1_NETWORK1_H_ */
