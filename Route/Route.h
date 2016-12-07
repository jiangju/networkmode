/*
 * Route.h
 *
 *  Created on: 2016��6��29��
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
	unsigned char Ter[TER_ADDR_LEN];	//�ն˵�ַ
	int	s;								//�׽���
	unsigned char last_t[TIME_FRA_LEN];	//���һ��ͨ��ʱ��
	int ticker;							//��������ʱ
	void *rvaddr;						//���ܻ����ڶ��е�λ��
	pthread_mutex_t write_mutex;		//дsocketʱ�Ļ����ź���
	struct route_node *next;			//��һ���ڵ�
}TerSocket;								//�ն˵�ַ��Ӧ�׽���

struct route_recycle_node
{
	struct route_node *route_node;		//·�ɽڵ��ַ
	int ticker;							//���յ���ʱ
	struct route_recycle_node *next;	//��һ�������սڵ�
};										//�����յ�·�ɽڵ�

#define ROUTE_RECYCLE_TICKER	10		//·�ɻ��յ���ʱ  10S

#ifdef _ROUTE_C_

TerSocket *_FristNode;			//��һ��·�ɽڵ��ַ
pthread_mutex_t route_mutex;	//·����

struct route_recycle_node *_FristRecycleNode;	//��һ�����սڵ�
pthread_mutex_t _route_recycle_mutex;			//����·�ɽڵ�������

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

extern TerSocket *_FristNode;		//��һ��·�ɽڵ��ַ
extern pthread_mutex_t route_mutex;	//·����

extern struct route_recycle_node *_FristRecycleNode;	//��һ�����սڵ�
extern pthread_mutex_t _route_recycle_mutex;			//����·�ɽڵ�������

#endif

#endif /* ROUTE_ROUTE_H_ */
