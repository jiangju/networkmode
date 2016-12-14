/*
 * TopBuf.h
 *
 *  Created on: 2016��12��13��
 *      Author: j
 */

#ifndef NETWORK1_TOPBUF_H_
#define NETWORK1_TOPBUF_H_

#include "SysPara.h"

typedef struct ts_node
{
	int	s;								//�׽���
	unsigned short r;
	unsigned short w;
	unsigned char buff[SOCKET_RV_MAX];
	pthread_mutex_t mutex;				//����buffʱ�Ļ����ź���
	struct ts_node *next;
}top_rv;								//�׽��ֽ��ܻ���

typedef struct
{
	int num;							//����
	top_rv *node;						//�ڵ�
	pthread_rwlock_t rwlock;			//��д��
}hld_toprv;

void init_hld_top_rv(void);
int add_hld_top_rv_s(int s);
int del_hld_top_rv_s(int s);
int check_hld_top_rv_s(int s);
int update_hld_top_rv_buff(int s, unsigned char *inbuf, int len);
int analy_hld_top_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src);

#endif /* NETWORK1_TOPBUF_H_ */
