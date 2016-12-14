/*
 * HLDTcp.c
 *
 *  Created on: 2016��12��9��
 *      Author: j
 */
#define _HLDTCP_H_
#include "HLDTcp.h"
#undef _HLDTCP_H_

/*
 * ��������:��ʼ�������
 * ����:	ipstr	�����ip��ַ	�ַ�����ʽ192.168.0.66
 * 		port	�˿ں�
 * 		backlog ���������ļ�������
 * ����ֵ:>=0�׽��� -1ʧ��
 * */
int init_server(const char *ipstr, u_short port, int backlog)
{
	//��ȡ������׽���
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if(0 > s)
	{
		perror("socket");
		return -1;
	}

	//����Ϊ��socket �ر�ʱ����������ʹ�õ�ǰ�˿�
	int used = 1;
	if(0 > setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &used, sizeof(int))){
		perror("setsockopt");
		return -1;
	}

	//��Ԫ��
	struct sockaddr_in addr = {
		.sin_family	= PF_INET,
		.sin_port	= htons(port),
		.sin_addr = {
			.s_addr = (ipstr == NULL) ? INADDR_ANY : inet_addr(ipstr),
		}
	};

	//���׽���
	socklen_t addrlen = sizeof(addr);
	if(0 > bind(s, (struct sockaddr *)&addr, addrlen)){
		perror("bind");
		return -1;
	}

	//�����׽���
	if(0 > listen (s, backlog)){
		perror("listen");
		return -1;
	}

	return s;
}

/*
 * ��������:�����ļ�������Ϊ������
 * ����:	fd	�ļ�������
 * ����ֵ: 0 �ɹ� -1 ʧ��
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
 * ��������:�����Ӧ�׽��ּ����¼�������
 * ����:	epfd	����epollʱ���ص�epoll���
 * 		fd		�׽���
 * 		event	�¼�����
 * ����ֵ: 0 �ɹ� -1 ʧ��
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
