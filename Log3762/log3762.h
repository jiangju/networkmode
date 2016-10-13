/*
 * log3762.h
 *
 *  Created on: 2016年9月18日
 *      Author: j
 */

#ifndef LOG3762_LOG3762_H_
#define LOG3762_LOG3762_H_

#include <pthread.h>
#include "SysPara.h"

#define LOG_3762_LEN	500		//3762最大报文长度

struct log_3762
{
	unsigned char buf[LOG_3762_LEN];	//报文内容
	int len;							//报文长度
	char flag;							//上行0x01 下行0x00
	unsigned char t[TIME_FRA_LEN];		//时标
	struct log_3762 *next;				//下一个任务
};

struct log3762_task
{
	int max;							//最大任务数
	int num;							//当前任务数
	pthread_mutex_t	mutex;				//锁
	struct log_3762 *head;				//第一个任务
	struct log_3762 *tail;				//最后一个任务
	pthread_cond_t no_empty;			//任务不为空条件变量
	int close;							//任务关闭 0x01关闭  0x00 打开
};

#define LOG_3762_FILE0	"/opt/log_3762_0.txt"	//log376.2文件名
#define LOG_3762_FILE1	"/opt/log_3762_1.txt"	//log376.2文件名
#define LOG_3762_FILE2	"/opt/log_3762_2.txt"	//log376.2文件名
#define LOG_3762_FILE3	"/opt/log_3762_3.txt"	//log376.2文件名

#define LOG_3762_FILE_NUM	5				//log376.2文件个数

#define	LOG_3762_FILE_CFG	"/opt/log_3762_config"	//log376.2的信息文件，记录当前log的行号 位置 和开启标志以及文件编号

#define	LOG_3762_FILE_SIZE		1024*1024*3			//log376.2文件最大3M

struct log_config
{
	char flag;				//是否开启log376.2功能
	unsigned char index;	//文件索引
	unsigned char cs;		//校验
};

#ifdef	_LOG3762_H_
int _cfg3762_fd;		//log3762配置文件描述符
int _cfg3762_fd_flag;	//log3762配置文件描述符有效标志
pthread_mutex_t _log_3762_cfg_mutex;
struct log3762_task _log_3762_task;
struct log_config _log_cfg;
char * _log_3762_filename[LOG_3762_FILE_NUM] = {LOG_3762_FILE0, LOG_3762_FILE1, LOG_3762_FILE2, LOG_3762_FILE3};
#endif

void *pthread_log_3762(void *arg);
void log_3762_task_add(struct log3762_task *task, unsigned char *buf, int len, char flag);
void log_3762_task_start(struct log3762_task *task);
void log_3762_task_stop(struct log3762_task *task);
void open_log_3762(void);
void close_log_3762(void);

#ifndef	_LOG3762_H_
extern pthread_mutex_t _log_3762_cfg_mutex;
extern struct log3762_task _log_3762_task;
extern char * _log_3762_filename[LOG_3762_FILE_NUM];
extern struct log_config _log_cfg;
extern int _cfg3762_fd;			//log3762配置文件描述符
extern int _cfg3762_fd_flag;	//log3762配置文件描述符有效标志
#endif

#endif /* LOG3762_LOG3762_H_ */
