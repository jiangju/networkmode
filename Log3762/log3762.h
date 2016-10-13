/*
 * log3762.h
 *
 *  Created on: 2016��9��18��
 *      Author: j
 */

#ifndef LOG3762_LOG3762_H_
#define LOG3762_LOG3762_H_

#include <pthread.h>
#include "SysPara.h"

#define LOG_3762_LEN	500		//3762����ĳ���

struct log_3762
{
	unsigned char buf[LOG_3762_LEN];	//��������
	int len;							//���ĳ���
	char flag;							//����0x01 ����0x00
	unsigned char t[TIME_FRA_LEN];		//ʱ��
	struct log_3762 *next;				//��һ������
};

struct log3762_task
{
	int max;							//���������
	int num;							//��ǰ������
	pthread_mutex_t	mutex;				//��
	struct log_3762 *head;				//��һ������
	struct log_3762 *tail;				//���һ������
	pthread_cond_t no_empty;			//����Ϊ����������
	int close;							//����ر� 0x01�ر�  0x00 ��
};

#define LOG_3762_FILE0	"/opt/log_3762_0.txt"	//log376.2�ļ���
#define LOG_3762_FILE1	"/opt/log_3762_1.txt"	//log376.2�ļ���
#define LOG_3762_FILE2	"/opt/log_3762_2.txt"	//log376.2�ļ���
#define LOG_3762_FILE3	"/opt/log_3762_3.txt"	//log376.2�ļ���

#define LOG_3762_FILE_NUM	5				//log376.2�ļ�����

#define	LOG_3762_FILE_CFG	"/opt/log_3762_config"	//log376.2����Ϣ�ļ�����¼��ǰlog���к� λ�� �Ϳ�����־�Լ��ļ����

#define	LOG_3762_FILE_SIZE		1024*1024*3			//log376.2�ļ����3M

struct log_config
{
	char flag;				//�Ƿ���log376.2����
	unsigned char index;	//�ļ�����
	unsigned char cs;		//У��
};

#ifdef	_LOG3762_H_
int _cfg3762_fd;		//log3762�����ļ�������
int _cfg3762_fd_flag;	//log3762�����ļ���������Ч��־
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
extern int _cfg3762_fd;			//log3762�����ļ�������
extern int _cfg3762_fd_flag;	//log3762�����ļ���������Ч��־
#endif

#endif /* LOG3762_LOG3762_H_ */
