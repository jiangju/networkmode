/*
 * NetWork.h
 *
 *  Created on: 2016��7��18��
 *      Author: j
 */

#ifndef NETWORK0_NETWORK0_H_
#define NETWORK0_NETWORK0_H_

#include "SysPara.h"
//#include <pthread.h>

#define SOCKET_TICKER	300	//�׽�����������ʱ

#define NETWORK_MAX_CONNCET	600				//socket���������
#define EVTMAX 	NETWORK_MAX_CONNCET			//epoll ��ط����¼������
#define NETWOEK_R_MAX 	1024				//socket ÿ�ζ�ȡ�������

#define SOCKET_RV_MAX	1024				//�׽��ֽ�������ֽ���

typedef struct
{
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t analy_mutex;			//����buffʱ�Ļ����ź���
}SocketRv;	//�׽��ֽ��ܻ���

int init_server(const char *ipstr, unsigned short port, int backlog);
int set_nonblock(int fd);
int add_event(int epfd, int fd, unsigned int event);
int del_event(int epfd, int fd, unsigned int event);
void *NetWork0(void *arg);

#endif /* NETWORK0_NETWORK0_H_ */
