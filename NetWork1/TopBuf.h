/*
 * TopBuf.h
 *
 *  Created on: 2016年12月13日
 *      Author: j
 */

#ifndef NETWORK1_TOPBUF_H_
#define NETWORK1_TOPBUF_H_

#include "SysPara.h"

typedef struct ts_node
{
	int	s;								//套接字
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t mutex;				//解析buff时的互斥信号量
	struct ts_node *next;
}top_rv;								//套接字接受缓存

typedef struct
{
	int num;							//数量
	top_rv *node;						//节点
	pthread_rwlock_t rwlock;			//读写锁
}hld_toprv;

void init_hld_top_rv(void);
int add_hld_top_rv_s(int s);
int del_hld_top_rv_s(int s);
int check_hld_top_rv_s(int s);
int update_hld_top_rv_buff(int s, unsigned char *inbuf, int len);
int analy_hld_top_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src);

#endif /* NETWORK1_TOPBUF_H_ */
