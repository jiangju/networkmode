/*
 * RevBuf.h
 *
 *  Created on: 2016年12月8日
 *      Author: j
 */

#ifndef ROUTE_REVBUF_H_
#define ROUTE_REVBUF_H_

#include "SysPara.h"

typedef struct rv_node
{
	int	s;								//套接字
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t mutex;				//解析buff时的互斥信号量
	struct rv_node *next;
}socket_rv;								//套接字接受缓存

typedef struct
{
	int num;							//数量
	socket_rv *node;					//节点
	pthread_rwlock_t rwlock;			//读写锁
}hld_rv;								//接受缓存链表

void init_hld_rv(void);
int add_hld_rv_s(int s);
int del_hld_rv_s(int s);
int check_hld_rv_s(int s);
int update_hld_rv_buff(int s, unsigned char *inbuf, int len);
int analy_hld_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src);

#endif /* ROUTE_REVBUF_H_ */
