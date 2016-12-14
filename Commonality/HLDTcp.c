/*
 * HLDTcp.c
 *
 *  Created on: 2016年12月9日
 *      Author: j
 */
#define _HLDTCP_H_
#include "HLDTcp.h"
#undef _HLDTCP_H_

/*
 * 函数功能:初始化服务端
 * 参数:	ipstr	服务端ip地址	字符串格式192.168.0.66
 * 		port	端口号
 * 		backlog 队列中最大的监听数量
 * 返回值:>=0套接字 -1失败
 * */
int init_server(const char *ipstr, u_short port, int backlog)
{
	//获取服务端套接字
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if(0 > s)
	{
		perror("socket");
		return -1;
	}

	//设置为当socket 关闭时，可以立即使用当前端口
	int used = 1;
	if(0 > setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(int))){
		perror("setsockopt");
		return -1;
	}

	//三元组
	struct sockaddr_in addr = {
		.sin_family	= PF_INET,
		.sin_port	= htons(port),
		.sin_addr = {
			.s_addr = (ipstr == NULL) ? INADDR_ANY : inet_addr(ipstr),
		}
	};

	//绑定套接字
	socklen_t addrlen = sizeof(addr);
	if(0 > bind(s, (struct sockaddr *)&addr, addrlen)){
		perror("bind");
		return -1;
	}

	//监听套接字
	if(0 > listen (s, backlog)){
		perror("listen");
		return -1;
	}

	return s;
}

/*
 * 函数功能:设置文件描述符为非阻塞
 * 参数:	fd	文件描述符
 * 返回值: 0 成功 -1 失败
 * */
int set_nonblock(int fd)
{
	int flag = fcntl(fd, F_GETFL);
	if(0 > flag){
		perror("fcntl");
		return -1;
	}

	flag |= O_NONBLOCK;

	if(0 > fcntl(fd, F_SETFL, flag)){
		perror("fcntl-1");
		return -1;
	}

	return 0;
}

/*
 * 函数功能:添加相应套接字监听事件到队列
 * 参数:	epfd	创建epoll时返回的epoll句柄
 * 		fd		套接字
 * 		event	事件类型
 * 返回值: 0 成功 -1 失败
 * */
int add_event(int epfd, int fd, unsigned int event)
{
	struct epoll_event evt = {
		.events = event,
		.data = {
			.fd = fd,
		},
	};
	return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &evt);
}

int del_event(int epfd, int fd, unsigned int event)
{
	struct epoll_event evt = {
		.events = event,
		.data = {
			.fd = fd,
		},
	};

	return epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &evt);
}
