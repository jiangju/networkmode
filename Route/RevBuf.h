/*
 * RevBuf.h
 *
 *  Created on: 2016��12��8��
 *      Author: j
 */

#ifndef ROUTE_REVBUF_H_
#define ROUTE_REVBUF_H_

#include "SysPara.h"

typedef struct rv_node
{
	int	s;								//�׽���
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t mutex;				//����buffʱ�Ļ����ź���
	struct rv_node *next;
}socket_rv;								//�׽��ֽ��ܻ���

typedef struct
{
	int num;							//����
	socket_rv *node;					//�ڵ�
	pthread_rwlock_t rwlock;			//��д��
}hld_rv;								//���ܻ�������

void init_hld_rv(void);
int add_hld_rv_s(int s);
int del_hld_rv_s(int s);
int check_hld_rv_s(int s);
int update_hld_rv_buff(int s, unsigned char *inbuf, int len);
int analy_hld_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src);

#endif /* ROUTE_REVBUF_H_ */
