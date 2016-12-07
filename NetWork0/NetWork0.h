/*
 * NetWork.h
 *
 *  Created on: 2016年7月18日
 *      Author: j
 */

#ifndef NETWORK0_NETWORK0_H_
#define NETWORK0_NETWORK0_H_

#include "SysPara.h"
//#include <pthread.h>

#define SOCKET_TICKER	300	//套接字心跳倒计时

#define NETWORK_MAX_CONNCET	600				//socket最大连接数
#define EVTMAX 	NETWORK_MAX_CONNCET			//epoll 监控发生事件最大数
#define NETWOEK_R_MAX 	1024				//socket 每次读取最大数量

#define SOCKET_RV_MAX	1024				//套接字接受最大字节数

typedef struct
{
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t analy_mutex;			//解析buff时的互斥信号量
}SocketRv;	//套接字接受缓存

int init_server(const char *ipstr, unsigned short port, int backlog);
int set_nonblock(int fd);
int add_event(int epfd, int fd, unsigned int event);
int del_event(int epfd, int fd, unsigned int event);
void *NetWork0(void *arg);

#endif /* NETWORK0_NETWORK0_H_ */
