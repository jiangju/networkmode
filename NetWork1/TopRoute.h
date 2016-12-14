/*
 * TopRoute.h
 *
 *  Created on: 2016年12月12日
 *      Author: j
 */

#ifndef NETWORK1_TOPROUTE_H_
#define NETWORK1_TOPROUTE_H_

#include "SysPara.h"
#include "pthread.h"
#include <errno.h>
#include <signal.h>

typedef struct t_node
{
	unsigned char Ter[TER_ADDR_LEN];	//终端地址
	int	s;								//套接字
	unsigned char last_t[TIME_FRA_LEN];	//最后一次通信时间
	int ticker;							//心跳倒计时
	pthread_mutex_t smutex;				//写socket时的互斥信号量
	pthread_mutex_t mmutex;				//访问节点锁
	struct t_node *next;				//下一个节点
}top_node;								//充值终端节点

typedef struct
{
	int num;							//节点个数
	top_node *frist;					//第一个节点
	top_node *last;					//最后一个节点
	pthread_rwlock_t rwlock;			//锁
}hld_top;								//华立达充值终端链表头

void init_hld_top(void);
int add_hld_top_node(int s);
int dele_hld_top_node_s(int s);
int dele_hld_top_node_ter(unsigned char *ter);
int check_hld_top_node_s_ter(int s, unsigned char *ter);
int del_hld_top_node_ter_ter(unsigned char *ter);
int check_hld_top_node_s(int s);
int check_hld_top_node_ter(unsigned char *ter);
int update_hld_top_node_s_ter(int s, unsigned char *ter);
int update_hld_top_node_s_ticker(int s, int ticker);
int update_hld_top_node_s_stime(int s);
int send_hld_top_node_s_data(int s, unsigned char *inbuf, int len);
int send_hld_top_node_ter_data(unsigned char *ter, unsigned char *inbuf, int len);
int get_hld_top_node_num(void);
int get_hld_top_node_ter_lstime(int num, unsigned char *ter, unsigned char *lstime);
int get_hld_top_node_ter(int num, unsigned char *ter);
void *TopSocketTicker(void *arg);

#endif /* NETWORK1_TOPROUTE_H_ */
