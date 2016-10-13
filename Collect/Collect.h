/*
 * Collect.h
 *
 *  Created on: 2016��7��21��
 *      Author: j
 */

#ifndef COLLECT_COLLECT_H_
#define COLLECT_COLLECT_H_
#include "SysPara.h"
#include "StandBook.h"
#include <pthread.h>

#define TASK_MAX_LEN	300		//�ɼ����񻺴���󳤶�
#define TASK_MAX_NUM	100		//���񻺴��������������

#define NET_COLLECT_TIMER 6		//���糭��ȴ��ظ�ʱ��
#define AFN14_01_TIMER	  15	//���󳭶��������ݵȴ��ظ�ʱ��

#define EXE_COLLECT1	  6		//�㲥�·����������ʱ6��(��ʵ�¼�Ϊ3-4��)

#define TASKA_THREAD_SIZE	60	//�����ֵ�����̸߳���

struct task_a_content					//��ֵ��������
{
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char top_ter[TER_ADDR_LEN];//��ֵ�ն�  �ն˵�ַ
	unsigned char buf[TASK_MAX_LEN];	//����
	int 		  len;					//���ݳ���
	struct task_a_content *next;		//��һ����������
};

struct task_a_status	//����״̬
{
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char top_ter[TER_ADDR_LEN];//��ֵ�ն�  �ն˵�ַ
	unsigned char timer;				//���񵹼�ʱ(ִ������ �ȴ��ɹ�����)
	unsigned char is_end;				//�����Ƿ����	0����  1�������ڽ���
};

#define TASKA_NODE_LIVE_TIMER	60		//�ڵ����浹��ʱ 1min
#define TASKA_TIMEOUT			4		//��ֵ���� ��ʱʱ�� 4s

struct task_a_node						//��ֵ�������������еĽڵ�(����������ն˵�ַΪ�ڵ�)
{
	unsigned char amm[AMM_ADDR_LEN];	//���
	unsigned char ter[TER_ADDR_LEN];	//�ն�(����Ӧ���նˣ��ն˵�ַ����Ϊ������з�֧�Ľڵ�)
	unsigned char is_allot;				//�Ƿ������߳�  0Ϊδ����  1Ϊ�ѷ���
	unsigned int  timer;				//�ڵ���ڵ���ʱ(Ϊ0ʱ��ɾ���ýڵ�ķ�֧)
	struct task_a_content *task_head;	//ͷ����
	struct task_a_content *task_tail;	//β����
	struct task_a_status task_status;	//����ִ��״̬
	struct task_a_node *next;			//��һ���ڵ�
};

typedef struct
{
	int task_num;					//��������
	int not_assigned_num;			//û�з�����̵߳��ն˸���
	int key;						//����ִ�����񿪹�  1����  ��1 ������
	pthread_mutex_t taska_mutex;	//����a��
	pthread_cond_t need_assigned;	//���ն���Ҫ������߳�  ��������
	pthread_cond_t node_not_empty;	//�ڵ㲻Ϊ����������

	struct task_a_node *node_head;	//����ڵ���е�ͷ�ڵ�
	struct task_a_node *node_tail;	//����ڵ���е�β�ڵ�
}CollcetTaskA;	//��ֵ�ն�����

struct task_b
{
	unsigned char amm[NODE_ADDR_LEN];	//���
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char buf[TASK_MAX_LEN];	//����
	int 		  len;					//���ݳ���
	unsigned char type;					//Э������
	struct task_b *next;				//��һ������
};

typedef struct
{
	int num;	//��������
	pthread_mutex_t taskb_mutex;	//����B��
	struct task_b *next;	//��һ������
}CollcetTaskB;	//ʵʱ��������

typedef struct
{
	int				index;				//̨�˵��
	int 			timer;				//�������񵹼�ʱ
	int             count;				//��������ʧ�ܴ���
	int				isok;				//���������Ƿ�ɹ�
	unsigned char 	dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char 	buf[TASK_MAX_LEN];	//��������
	int				len;				//�������ݳ���
	int				key;//����
}CollcetTaskC;	//���ڳ�������

typedef struct
{
	int	index;	//̨������
	int key;	//����
}CollcetTaskD;

struct task_e
{
	unsigned char amm[NODE_ADDR_LEN];	//���
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char buf[TASK_MAX_LEN];	//����
	int 		  len;					//���ݳ���
	unsigned char type;					//Э������
	struct task_e *next;				//��һ������
};

typedef struct
{
	int num;	//��������
	pthread_mutex_t taske_mutex;	//����E��
	struct task_e *next;	//��һ������
}CollcetTaskE;	//ʵʱ͸��ת������  376.2͸��ת��

typedef struct
{
	CollcetTaskA taska;
	CollcetTaskB taskb;
	CollcetTaskC taskc;
//	CollcetTaskD taskd;
	CollcetTaskE taske;
	unsigned char runingtask;			//����ִ�е��������� 'a' 'b' 'c' 'd' 'e'
	unsigned char amm[AMM_ADDR_LEN];	//���		 (��ֵ����ı�Ų��ڴ��ж�)
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ        (��ֵ��������ݱ�ʶ���ڴ��ж�)
	unsigned char timer;				//����ʱ�ж� (��ֵ����ĳ�ʱ���ڴ��ж�)

	unsigned char b_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char c_isend;				//�����ݱ�ʶ�����Ƿ����
//	unsigned char d_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char e_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char conut;				//����ʧ�ܴ��� (��ֵ�ն˵ĳ���ʧ�ܴ������ڴ��ж�)
}Collcet;

#ifdef	_COOLECT_C_
Collcet _Collect;	//������
#endif

void CollectorInit(void);
void TaskcReset(void);
void *Collector(void *arg);
void CollectTaskB(void);
void CollectTaskC(void);
void CollectTaskE(void);
void TaskACollectInit(void);

void *TaskATimer(void *arg);
void *CollectTaskA(void *arg);
struct task_a_node *SeekTaskANode0(unsigned char *ter);
struct task_a_node *SeekTaskANode1(unsigned char *amm);
int AddTaskA(unsigned char *amm, unsigned char *dadt, unsigned char *buf, int len, unsigned char *ter);
int DeleTaskA(unsigned char *amm, unsigned char flag);

int AddTashB(struct task_b *b);
void DeleNearTaskB(void);
int AddTashE(struct task_e *e);
void DeleNearTaskE(void);
int ExecuteCollect0(unsigned char *amm, unsigned char *inbuf, int len);
int ExecuteCollect1(unsigned char *amm, unsigned char *inbuf, int len);

#ifndef _COOLECT_C_
extern Collcet _Collect;	//������
#endif

#endif /* COLLECT_COLLECT_H_ */
