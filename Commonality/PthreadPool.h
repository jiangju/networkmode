/*
 * PthreadPool.h
 *
 *  Created on: 2016年7月15日
 *      Author: j
 */

#ifndef COMMONALITY_PTHREADPOOL_H_
#define COMMONALITY_PTHREADPOOL_H_
#include <pthread.h>

#define THREAD_POOL_SIZE	200	//线程池大小
#define	QUEUE_MAX_NUM		2040	//线程池任务队列

struct job
{
	void* (*callback_function)(void *arg);	//线程回调函数
	void *arg;								//回调函数参数
	struct job *next;
};

struct threadpool
{
	int thread_num;					//线程池中开启线程的个数
	int queue_max_num;				//队列中最大job的个数
	struct job *head;				//指向job的头指针
	struct job *tail;				//指向job的尾指针
	pthread_t *pthreads;			//线程池中所有线程的pthread_t
	pthread_mutex_t mutex;			//互斥信号量
	pthread_cond_t queue_empty;		//队列为空的条件变量
	pthread_cond_t queue_not_empty;	//队列不为空的条件变量
	pthread_cond_t queue_not_full;	//队列不为满的条件变量
	int queue_cur_num;				//队列当前的job个数
	int queue_close;				//队列是否已经关闭
	int pool_close;					//线程池是否已经关闭
};

#ifdef	_PTHREADPOOL_C_
struct threadpool *_Threadpool;	//线程池地址
#endif

struct threadpool * threadpool_init(int thread_num, int queue_max_num);
int threadpool_add_job(struct threadpool* pool, void* (*callback_function)(void *arg), void *arg);
void* threadpool_function(void* arg);
int threadpool_destroy(struct threadpool *pool);

#ifndef _PTHREADPOOL_C_
extern struct threadpool *_Threadpool;	//线程池地址
#endif

#endif /* COMMONALITY_PTHREADPOOL_H_ */
