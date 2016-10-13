/*
 * log3762.c
 *
 *  Created on: 2016��9��18��
 *      Author: j
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <error.h>
#include "CommLib.h"
#include "HLDWatchDog.h"
#define	_LOG3762_H_
#include "log3762.h"
#undef	_LOG3762_H_
/*
 * ��������:��ʼ��376.2����log����
 * ����:	task		����
 * ����ֵ:��
 * */
void log_3762_task_init(struct log3762_task *task, int max)
{
	task->close = 0x00;	//��
	task->max = max;	//�����������
	task->num = 0;		//��ǰ�������
	task->head = NULL;	//��һ������
	task->tail = NULL;	//���һ������

	//��ʼ����
	pthread_mutex_init(&task->mutex, NULL);

	//��ʼ����������
	pthread_cond_init(&task->no_empty, NULL);
}

/*
 * ��������:����376.2����log����
 * */
void log_3762_task_start(struct log3762_task *task)
{
	pthread_mutex_lock(&task->mutex);
	task->close = 0x00;	//��
	pthread_mutex_unlock(&task->mutex);
}

/*
 * ��������:�ر�376.2����log����
 * */
void log_3762_task_stop(struct log3762_task *task)
{
	struct log_3762 *temp;

	pthread_mutex_lock(&task->mutex);

	task->close = 0x01;	//�ر�
	while(task->head != NULL)
	{
		temp = task->head->next;
		free(task->head);
		task->num--;
		task->head = temp;
	}
	pthread_mutex_unlock(&task->mutex);
}

/*
 * ��������:log376.2������������
 * ����:	task	������
 * 		buf		��������
 * 		len 	����
 * 		flag	����/����
 * ����ֵ:
 * */
void log_3762_task_add(struct log3762_task *task, unsigned char *buf, int len, char flag)
{
	struct log_3762 *temp = NULL;

	pthread_mutex_lock(&task->mutex);

	if(task->max <= task->num || task->close == 0x01)
	{
		pthread_mutex_unlock(&task->mutex);
		return;
	}

	temp = (struct log_3762 *)malloc(sizeof(struct log_3762));
	if(NULL == temp)
	{
		pthread_mutex_unlock(&task->mutex);
		return;
	}
	temp->len = len;
	temp->flag = flag;
	memcpy(temp->buf, buf, len);
	temp->next = NULL;
	//��ȡ��ǰʱ��
	time_t timer;
	struct tm* t_tm;
	time(&timer);
	t_tm = localtime(&timer);
	//��
	temp->t[0] = t_tm->tm_year - 100;
	//��
	temp->t[1] = t_tm->tm_mon + 1;
	//��
	temp->t[2] = t_tm->tm_mday;
	//ʱ
	temp->t[3] = t_tm->tm_hour;
	//��
	temp->t[4] = t_tm->tm_mday;
	//��
	temp->t[5] = t_tm->tm_sec;

	if(task->head == NULL)	//��һ���������
	{
		task->head = temp;
		task->tail = temp;
	}
	else
	{
		task->tail->next = temp;
		task->tail = temp;
	}
	task->num++;
	pthread_mutex_unlock(&task->mutex);
	printf("log add\n");
}

/*
 * ��������:ɾ����ǰ����
 * */
void log_3762_task_dele(struct log3762_task *task)
{
	pthread_mutex_lock(&task->mutex);
	struct log_3762 *temp = NULL;
	temp = task->head;
	if(NULL == temp)
	{
		return;
	}

	if(task->head != task->tail)
	{
		task->head = task->head->next;
	}
	else
	{
		task->head = NULL;
		task->tail = NULL;
	}
	free(temp);
	task->num--;
	pthread_mutex_unlock(&task->mutex);
}

/*
 * ��������:log376.2������ִ��
 * ����:	task	������
 * 		cfg		log ����
 * */
void log_3762_task_exec(struct log3762_task *task, int fd)
{
	pthread_mutex_lock(&task->mutex);

	if(task->num == 0 || task->close != 0x00)	//������
	{
		pthread_mutex_unlock(&task->mutex);
		return;
		//pthread_cond_wait(&(task->no_empty), &(task->mutex));
	}
	unsigned char str[500] = {0};
	unsigned char s[6] = {0};

	if(task->head->flag == 0x01)	//����
	{
		memcpy(s, "up:", sizeof("up:"));
	}
	else
	{
		memcpy(s, "down:", sizeof("down:"));
	}

	//��ʽ���ַ���
	sprintf((char *)str, "%s %d-%d-%d %d:%d:%d", s,task->head->t[0],task->head->t[1],task->head->t[2],task->head->t[3],task->head->t[4],task->head->t[5]);
	//���������ݸ�ʽ�����ַ�����
	int i = 0;
	for(i = 0; i < task->head->len; i++)
	{
		sprintf((char *)str, "%s %02x", str, task->head->buf[i]);
	}
	sprintf((char *)str, "%s\r\n",str);

	//���ַ���д���ļ�
	write(fd, str, strlen((char *)str));

	pthread_mutex_unlock(&task->mutex);

	//ɾ��ִ����ɵ�����
	log_3762_task_dele(task);
}

/*
 * ��������:����log372.2���Ĺ���
 * */
void open_log_3762(void)
{
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//��log�����ļ�����ȡ��ǰ��¼״̬
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_exit(NULL);
		}
	}
	//��ȡ�����ļ�����
	lseek(_cfg3762_fd, 0, SEEK_SET);
	int ret = read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(ret <= 0)
	{
		memset(&_log_cfg, 0, sizeof(struct log_config));
		lseek(_cfg3762_fd, 0, SEEK_SET);
		write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	}
	else
	{
		if(_log_cfg.cs != Func_CS(&_log_cfg, offsetof(struct log_config, cs)))
		{
			memset(&_log_cfg, 0, sizeof(struct log_config));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
		}
		else
		{
			_log_cfg.flag = 0x00;
			_log_cfg.cs = Func_CS(&_log_cfg, offsetof(struct log_config, cs));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
		}
	}
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);

	log_3762_task_start(&_log_3762_task);
}

/*
 * ��������:�ر�log372.2���Ĺ���
 * */
void close_log_3762(void)
{
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//��log�����ļ�����ȡ��ǰ��¼״̬
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_exit(NULL);
		}
	}
	//��ȡ�����ļ�����
	lseek(_cfg3762_fd, 0, SEEK_SET);
	int ret = read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(ret <= 0)
	{
		memset(&_log_cfg, 0, sizeof(struct log_config));
		lseek(_cfg3762_fd, 0, SEEK_SET);
		_log_cfg.flag = 0x01;
		_log_cfg.cs = Func_CS(&_log_cfg, offsetof(struct log_config, cs));
		write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	}
	else
	{
		if(_log_cfg.cs != Func_CS(&_log_cfg, offsetof(struct log_config, cs)))
		{
			memset(&_log_cfg, 0, sizeof(struct log_config));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			_log_cfg.flag = 0x01;
			_log_cfg.cs = Func_CS(&_log_cfg, offsetof(struct log_config, cs));
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
		}
		else
		{
			_log_cfg.flag = 0x01;
			_log_cfg.cs = Func_CS(&_log_cfg, offsetof(struct log_config, cs));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
		}
	}
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);
	log_3762_task_stop(&_log_3762_task);
}

/*
 * ��������:log376.2�߳�
 * */
void *pthread_log_3762(void *arg)
{
	//��ȡ���Ź�id
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��

	//��ʼ�� ������
	log_3762_task_init(&_log_3762_task, 30);

	//��ʼ����
	pthread_mutex_init(&_log_3762_cfg_mutex, NULL);

	//��log�����ļ�����ȡ��ǰ��¼״̬
	_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
	if(_cfg3762_fd < 0)
	{
		perror("open log config: ");
		pthread_exit(NULL);
	}
	_cfg3762_fd_flag = 0x66;	//�����ļ��ѳɹ���

	//��ȡ�����ļ�����
	lseek(_cfg3762_fd, 0, SEEK_SET);
	int ret = read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(ret <= 0)
	{
		memset(&_log_cfg, 0, sizeof(struct log_config));
		lseek(_cfg3762_fd, 0, SEEK_SET);
		write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	}
	else
	{
		if(_log_cfg.cs != Func_CS(&_log_cfg, offsetof(struct log_config, cs)))
		{
			memset(&_log_cfg, 0, sizeof(struct log_config));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
		}
	}
	feed_watch_dog(wdt_id);

	//������log���ļ�
	int log_fd = open(_log_3762_filename[_log_cfg.index], O_RDWR | O_CREAT, 0666);
	if(log_fd < 0)
	{
		perror("open log file :");
		pthread_exit(NULL);
	}
	while(1)
	{
		usleep(1);
		feed_watch_dog(wdt_id);
		pthread_mutex_lock(&_log_3762_cfg_mutex);

		if(_log_cfg.flag != 0x00)
		{
			pthread_mutex_unlock(&_log_3762_cfg_mutex);
			continue;
		}

		//�ж��Ƿ��ļ��洢
		ret = lseek(log_fd, 0, SEEK_END);
		if(ret >= LOG_3762_FILE_SIZE)	//��ǰ�ļ������л���һ���ļ�
		{
			close(log_fd);
			_log_cfg.index = (_log_cfg.index + 1) % LOG_3762_FILE_NUM;
			_log_cfg.cs = Func_CS(&_log_cfg, offsetof(struct log_config, cs));
			lseek(_cfg3762_fd, 0, SEEK_SET);
			write(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
			log_fd = open(_log_3762_filename[_log_cfg.index], O_RDWR | O_CREAT | O_TRUNC, 0666);
			if(log_fd < 0)
			{
				pthread_mutex_unlock(&_log_3762_cfg_mutex);
				pthread_exit(NULL);
			}
		}
		pthread_mutex_unlock(&_log_3762_cfg_mutex);
		log_3762_task_exec(&_log_3762_task,log_fd);
	}

	pthread_exit(NULL);
}
