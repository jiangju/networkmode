/*
 * HLDTcp.h
 *
 *  Created on: 2016��12��9��
 *      Author: j
 */

#ifndef COMMONALITY_HLDTCP_H_
#define COMMONALITY_HLDTCP_H_
#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "SysPara.h"

#define EVTMAX 	NETWORK_MAX_CONNCET			//epoll ��ط����¼������
#define NETWOEK_R_MAX 	SOCKET_RV_MAX		//socket ÿ�ζ�ȡ�������

int init_server(const char *ipstr, u_short port, int backlog);
int set_nonblock(int fd);
int add_event(int epfd, int fd, unsigned int event);
int del_event(int epfd, int fd, unsigned int event);

#ifdef _HLDTCP_H_
//������һ�ն�
int _epollfd;
struct epoll_event r_evt[EVTMAX];

//��ֵ�ն�
int _topup_epoll;
struct epoll_event top_evt[EVTMAX];
#endif

#ifndef _HLDTCP_H_
extern int _epollfd;
extern struct epoll_event r_evt[EVTMAX];

extern int _topup_epoll;
extern struct epoll_event top_evt[EVTMAX];
#endif

#endif /* COMMONALITY_HLDTCP_H_ */
