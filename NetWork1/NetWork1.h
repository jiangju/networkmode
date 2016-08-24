/*
 * NetWork1.h
 *
 *  Created on: 2016��8��9��
 *      Author: j
 */

#ifndef NETWORK1_NETWORK1_H_
#define NETWORK1_NETWORK1_H_

#include <pthread.h>
#include "NetWork0.h"

#define TOPUP_MAX	100	//��ֵ�ն����������

typedef struct
{
	unsigned char buf[SOCKET_RV_MAX];	//����
	unsigned short r;					//��ָ��
	unsigned short w;					//дָ��
	pthread_mutex_t bufmutex;			//���������
}TopUpRv;	//��ֵ�ն˽��ջ���

struct topup_node
{
	int s;								//�׽��֣���ֵ�նˣ�
	unsigned char ter[TER_ADDR_LEN];	//�ն˵�ַ
	TopUpRv	rvbuf;						//���ջ���
	int ticker;							//��������ʱ
	pthread_mutex_t write_mutex;		//дsocketʱ�Ļ����ź���
	struct topup_node *next;			//��һ����ֵ�ն˵Ľڵ�
};										//��ֵ�ն˽ڵ�

struct topup_recycle_node
{
	struct topup_node *topup_node;		//��ֵ�ն˽ڵ��ַ
	int ticker;							//���յ���ʱ
	struct topup_recycle_node *next;	//��һ�������սڵ�
};										//�����յĳ�ֵ�ն˽ڵ�

#define TOPUP_RECYCLE_TICKER	10		//��ֵ�ն˽ڵ���յ���ʱ 10S

#ifdef	_NETWORK1_C_

struct topup_node *_FristTop;		//��һ����ֵ�ն˽ڵ�
pthread_mutex_t topup_node_mutex;	//��ֵ�ն˽ڵ���

struct topup_recycle_node *_FristRecycleTopNode;	//��һ�������ճ�ֵ�ڵ�Ĵ����սڵ�
pthread_mutex_t _topup_recycle_mutex;			//������������

#endif

#ifndef	_NETWORK1_C_

extern struct topup_node *_FristTop;		//��һ����ֵ�ն˽ڵ�
extern pthread_mutex_t topup_node_mutex;	//��ֵ�ն˽ڵ���

extern struct topup_recycle_node *_FristRecycleTopNode;	//��һ�������ճ�ֵ�ڵ�Ĵ����սڵ�
extern pthread_mutex_t _topup_recycle_mutex;			//������������

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
