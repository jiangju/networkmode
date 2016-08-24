/*
 * PthreadPool.h
 *
 *  Created on: 2016��7��15��
 *      Author: j
 */

#ifndef COMMONALITY_PTHREADPOOL_H_
#define COMMONALITY_PTHREADPOOL_H_
#include <pthread.h>

#define THREAD_POOL_SIZE	200	//�̳߳ش�С
#define	QUEUE_MAX_NUM		2040	//�̳߳��������

struct job
{
	void* (*callback_function)(void *arg);	//�̻߳ص�����
	void *arg;								//�ص���������
	struct job *next;
};

struct threadpool
{
	int thread_num;					//�̳߳��п����̵߳ĸ���
	int queue_max_num;				//���������job�ĸ���
	struct job *head;				//ָ��job��ͷָ��
	struct job *tail;				//ָ��job��βָ��
	pthread_t *pthreads;			//�̳߳��������̵߳�pthread_t
	pthread_mutex_t mutex;			//�����ź���
	pthread_cond_t queue_empty;		//����Ϊ�յ���������
	pthread_cond_t queue_not_empty;	//���в�Ϊ�յ���������
	pthread_cond_t queue_not_full;	//���в�Ϊ������������
	int queue_cur_num;				//���е�ǰ��job����
	int queue_close;				//�����Ƿ��Ѿ��ر�
	int pool_close;					//�̳߳��Ƿ��Ѿ��ر�
};

#ifdef	_PTHREADPOOL_C_
struct threadpool *_Threadpool;	//�̳߳ص�ַ
#endif

struct threadpool * threadpool_init(int thread_num, int queue_max_num);
int threadpool_add_job(struct threadpool* pool, void* (*callback_function)(void *arg), void *arg);
void* threadpool_function(void* arg);
int threadpool_destroy(struct threadpool *pool);

#ifndef _PTHREADPOOL_C_
extern struct threadpool *_Threadpool;	//�̳߳ص�ַ
#endif

#endif /* COMMONALITY_PTHREADPOOL_H_ */
