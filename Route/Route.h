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

typedef struct s_node
{
	unsigned char Ter[TER_ADDR_LEN];	//终端地址
	int	s;								//套接字
	unsigned char last_t[TIME_FRA_LEN];	//最后一次通信时间
	int ticker;							//心跳倒计时
	pthread_mutex_t smutex;				//写socket时的互斥信号量
	pthread_mutex_t mmutex;				//访问节点锁
	struct s_node *next;				//下一个节点
}route_node;

typedef struct
{
	int num;							//节点个数
	route_node *frist;					//第一个节点
	route_node *last;					//最后一个节点
	pthread_rwlock_t rwlock;			//锁
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
