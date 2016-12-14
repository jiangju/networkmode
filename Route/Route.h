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

typedef struct s_node
{
	unsigned char Ter[TER_ADDR_LEN];	//�ն˵�ַ
	int	s;								//�׽���
	unsigned char last_t[TIME_FRA_LEN];	//���һ��ͨ��ʱ��
	int ticker;							//��������ʱ
	pthread_mutex_t smutex;				//дsocketʱ�Ļ����ź���
	pthread_mutex_t mmutex;				//���ʽڵ���
	struct s_node *next;				//��һ���ڵ�
}route_node;

typedef struct
{
	int num;							//�ڵ����
	route_node *frist;					//��һ���ڵ�
	route_node *last;					//���һ���ڵ�
	pthread_rwlock_t rwlock;			//��
}hld_route;

void init_hld_route(void);
int add_hld_route_node(int s);
int dele_hld_route_node_s(int s);
int dele_hld_route_node_ter(unsigned char *ter);
int del_hld_route_node_ter_ter(unsigned char *ter);
int check_hld_route_node_s_ter(int s, unsigned char *ter);
int check_hld_route_node_s(int s);
int check_hld_route_node_ter(unsigned char *ter);
int update_hld_route_node_s_ter(int s, unsigned char *ter);
int update_hld_route_node_s_ticker(int s, int ticker);
int update_hld_route_node_s_stime(int s);
int send_hld_route_node_s_data(int s, unsigned char *inbuf, int len);
int send_hld_route_node_ter_data(unsigned char *ter, unsigned char *inbuf, int len);

int get_hld_route_node_num(void);
int get_hld_route_node_ter_lstime(int num, unsigned char *ter, unsigned char *lstime);
int get_hld_route_node_ter(int num, unsigned char *ter);

/**********************************************************************/
void *SocketTicker(void *arg);
#endif /* ROUTE_ROUTE_H_ */
