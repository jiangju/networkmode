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

#define NET_COLLECT_TIMER 6	//���糭��ȴ��ظ�ʱ��
#define AFN14_01_TIMER	  15	//���󳭶��������ݵȴ��ظ�ʱ��

#define EXE_COLLECT1	  6		//�㲥�·����������ʱ6��(��ʵ�¼�Ϊ3-4��)

struct task_a
{
	unsigned char amm[AMM_ADDR_LEN];	//���
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char buf[TASK_MAX_LEN];	//����
	int			  len;					//���ݳ���
	unsigned char ter[TER_ADDR_LEN];	//�����������ն�
	struct task_a *next;				//��һ������
};

typedef struct
{
	int num;	//��������
	pthread_mutex_t taska_mutex;	//����a��
	struct task_a *next;	//�¸�����
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
	CollcetTaskD taskd;
	CollcetTaskE taske;
	unsigned char runingtask;			//����ִ�е��������� 'a' 'b' 'c' 'd' 'e'
	unsigned char amm[AMM_ADDR_LEN];	//���
	unsigned char dadt[DADT_LEN];		//���ݱ�ʶ
	unsigned char timer;				//����ʱ�ж�
	unsigned char a_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char b_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char c_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char d_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char e_isend;				//�����ݱ�ʶ�����Ƿ����
	unsigned char conut;				//����ʧ�ܴ���
//	int			  s;					//��ֵ�ն��׽���
//	int  		  flag;					//���׽����Ƿ���Ч
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
int AddTaskA(struct task_a *a);
void DeleNearTaskA(void);
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
