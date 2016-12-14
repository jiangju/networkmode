/*
 * TopRoute.h
 *
 *  Created on: 2016��12��12��
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
	unsigned char Ter[TER_ADDR_LEN];	//�ն˵�ַ
	int	s;								//�׽���
	unsigned char last_t[TIME_FRA_LEN];	//���һ��ͨ��ʱ��
	int ticker;							//��������ʱ
	pthread_mutex_t smutex;				//дsocketʱ�Ļ����ź���
	pthread_mutex_t mmutex;				//���ʽڵ���
	struct t_node *next;				//��һ���ڵ�
}top_node;								//��ֵ�ն˽ڵ�

typedef struct
{
	int num;							//�ڵ����
	top_node *frist;					//��һ���ڵ�
	top_node *last;					//���һ���ڵ�
	pthread_rwlock_t rwlock;			//��
}hld_top;								//�������ֵ�ն�����ͷ

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
