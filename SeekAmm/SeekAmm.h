/*
 * SeekAmm.h
 *
 *  Created on: 2016年10月13日
 *      Author: j
 */

#ifndef SEEKAMM_SEEKAMM_H_
#define SEEKAMM_SEEKAMM_H_

#include "SysPara.h"
#include "NetWork0.h"
#include <pthread.h>

#define SEEK_AMM_FILE	"/opt/seek_amm_file"		//搜表结果文件
#define SEEK_AMM_TICKER		400						//搜表任务最大超时时间

struct seek_amm_result	//搜表结果
{
	int index;							//在搜表文件中存储的位置
	unsigned char flag;					//有效标志 0x55 有效 其他无效
	unsigned char ter[TER_ADDR_LEN];	//终端地址
	int num;							//终端下电表个数 TER_UNDER_AMM_MAX 192
	unsigned char amm[TER_UNDER_AMM_MAX][AMM_ADDR_LEN];	//终端下的电表地址
};

struct seek_amm_result_store	//搜表结果的存储
{
	struct seek_amm_result result;
	unsigned char cs;
};

struct seek_amm_node	//搜表结果的节点
{
	struct seek_amm_result result;
	struct seek_amm_node *next;
};

struct initiative_stand	//主动台账	搜表建立的台账
{
	int	nun;						//已收到num个终端回复搜表结果
	char index[NETWORK_MAX_CONNCET];//主动台账  位置使用情况  0x33未使用  0x1使用
	pthread_mutex_t mutex;			//主动台账锁
	struct seek_amm_node *frist;	//第一个搜表结果的节点
	struct seek_amm_node *last;		//最后一个搜表结果的节点
};

struct seek_amm_task	//搜表任务
{
	unsigned char ter[TER_ADDR_LEN];	//搜表终端
	char	flag;						//是否已经下发搜表帧  0x66已经下发
	int		ticker;						//搜表任务下发后倒计时
	int 	ticker_ticker;				//任务执行倒计时
	struct seek_amm_task *next;			//下一个搜表任务
};

struct seek_amm_task_queue			//搜表任务队列
{
	pthread_mutex_t mutex;			//搜表任务队列锁
	pthread_cond_t queue_nonempty;	//队列为空条件变量
	struct seek_amm_task *frist;	//第一个任务
	struct seek_amm_task *last;		//最后个任务
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
