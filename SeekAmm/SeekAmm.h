/*
 * SeekAmm.h
 *
 *  Created on: 2016��10��13��
 *      Author: j
 */

#ifndef SEEKAMM_SEEKAMM_H_
#define SEEKAMM_SEEKAMM_H_

#include "SysPara.h"
#include "NetWork0.h"
#include <pthread.h>

#define SEEK_AMM_FILE	"/opt/seek_amm_file"		//�ѱ����ļ�
#define SEEK_AMM_TICKER		400						//�ѱ��������ʱʱ��

struct seek_amm_result	//�ѱ���
{
	int index;							//���ѱ��ļ��д洢��λ��
	unsigned char flag;					//��Ч��־ 0x55 ��Ч ������Ч
	unsigned char ter[TER_ADDR_LEN];	//�ն˵�ַ
	int num;							//�ն��µ����� TER_UNDER_AMM_MAX 192
	unsigned char amm[TER_UNDER_AMM_MAX][AMM_ADDR_LEN];	//�ն��µĵ���ַ
};

struct seek_amm_result_store	//�ѱ����Ĵ洢
{
	struct seek_amm_result result;
	unsigned char cs;
};

struct seek_amm_node	//�ѱ����Ľڵ�
{
	struct seek_amm_result result;
	struct seek_amm_node *next;
};

struct initiative_stand	//����̨��	�ѱ�����̨��
{
	int	nun;						//���յ�num���ն˻ظ��ѱ���
	char index[NETWORK_MAX_CONNCET];//����̨��  λ��ʹ�����  0x33δʹ��  0x1ʹ��
	pthread_mutex_t mutex;			//����̨����
	struct seek_amm_node *frist;	//��һ���ѱ����Ľڵ�
	struct seek_amm_node *last;		//���һ���ѱ����Ľڵ�
};

struct seek_amm_task	//�ѱ�����
{
	unsigned char ter[TER_ADDR_LEN];	//�ѱ��ն�
	char	flag;						//�Ƿ��Ѿ��·��ѱ�֡  0x66�Ѿ��·�
	int		ticker;						//�ѱ������·��󵹼�ʱ
	int 	ticker_ticker;				//����ִ�е���ʱ
	struct seek_amm_task *next;			//��һ���ѱ�����
};

struct seek_amm_task_queue			//�ѱ��������
{
	pthread_mutex_t mutex;			//�ѱ����������
	pthread_cond_t queue_nonempty;	//����Ϊ����������
	struct seek_amm_task *frist;	//��һ������
	struct seek_amm_task *last;		//��������
};

void init_initiative_stand(void);
int destroy_initiative_stand_file(void);
void initiative_stand_index_is_using(unsigned int index, char flag);
void initiative_stand_all_index_init(void);
void destroy_initiative_stand(void);
int get_seek_amm_result(unsigned char *ter, struct seek_amm_result *result);
int add_seek_amm_result(struct seek_amm_result *result);
int write_seek_amm_result(struct seek_amm_result *result);
void dele_seek_amm_result(unsigned char *ter);
int initiative_stand_file_add_to_memory(void);
int initiative_stand_min_not_using_index(void);
void seek_amm_task_queue_init(void);
int add_seek_amm_task(struct seek_amm_task *task);
int dele_seek_amm_task(unsigned char *ter);
int find_seek_amm_task(unsigned char *ter, struct seek_amm_task *task);
int judge_seek_amm_task(unsigned char *ter);
void *SeekAmm(void *arg);
int seek_amm_task_empty(void);
int get_seek_amm_task_num(void);
int get_n_seek_amm_task(int n, struct seek_amm_task *task);

#endif /* SEEKAMM_SEEKAMM_H_ */
