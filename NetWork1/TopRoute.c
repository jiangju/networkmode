/*
 * TopRoute.c
 *
 *  Created on: 2016��12��12��
 *      Author: j
 */
#include <string.h>
#include <stdlib.h>
#include "CommLib.h"
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include "HLDWatchDog.h"
#include <sys/time.h>
#include "HLDTcp.h"
#include "errno.h"
#include "TopRoute.h"
#include "TopBuf.h"

static hld_top _hld_top;

/*
 * ��������:��ʼ��������·������
 * */
void init_hld_top(void)
{
	_hld_top.num = 0;
	_hld_top.frist = NULL;
	_hld_top.last = NULL;
	pthread_rwlock_init(&_hld_top.rwlock, NULL);
}

/*
 * ��������:�����׽�����ӳ�ֵ�ڵ�
 * ����:		s 	�׽���
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int add_hld_top_node(int s)
{
	top_node *node = (top_node*)malloc(sizeof(top_node));
	if(NULL == node)
	{
		perror("malloc top_node:");
		return -1;
	}

	memset(node->Ter, 0xFF, TER_ADDR_LEN);		//��ʼ���ն˵�ַ
	memset(node->last_t, 0, TIME_FRA_LEN);		//��ʼ�����ͨ��ʱ��
	node->next = NULL;							//��ʼ����һ���ڵ��ַ
	node->s = s;								//��ʼ���׽���
	node->ticker = 40;							//���Ӻ�����40�����յ���¼����
	pthread_mutex_init(&(node->smutex), NULL);	//��ʼ��socket������
	pthread_mutex_init(&(node->mmutex), NULL);	//���ʽڵ���
	//���
	pthread_rwlock_wrlock(&_hld_top.rwlock);

	if(0 >= _hld_top.num)	//��һ�����
	{
		_hld_top.frist = node;
		_hld_top.last = node;
		_hld_top.num = 1;
	}
	else					//�ǵ�һ�����
	{
		_hld_top.last->next = node;
		_hld_top.last = node;
		_hld_top.num++;
	}

	pthread_rwlock_unlock(&_hld_top.rwlock);

	return 0;
}

/*
 * ��������:����socket�رճ�ֵ�ڵ㼰������Դ
 * ������	s		�׽���
 * ����ֵ:	0�ɹ� -1ʧ��
 * */
int dele_hld_top_node_s(int s)
{
	top_node *temp = NULL;
	top_node *last_temp = NULL;
	int ret = -1;
	pthread_rwlock_wrlock(&_hld_top.rwlock);

	temp = _hld_top.frist;
	last_temp = temp;
	while(NULL != temp)		//���Ҷ�Ӧsocket
	{
		if(s == temp->s)
			break;
		last_temp = temp;
		temp = temp->next;
	}

	if(temp == NULL)
	{
		ret = -1;
	}
	else
	{
		if(temp == _hld_top.frist)	//ɾ����һ��
		{
			if(_hld_top.last == _hld_top.frist)
			{
				_hld_top.frist = NULL;
				_hld_top.last = NULL;
			}
			else
			{
				_hld_top.frist = _hld_top.frist->next;
			}
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		else
		{
			if(temp == _hld_top.last)
			{
				_hld_top.last = last_temp;
			}
			last_temp->next = temp->next;
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		_hld_top.num--;
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:�����ն˵�ַ��ֵ�ڵ㼰������Դ
 * ����: ter		�ն˵�ַ
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int dele_hld_top_node_ter(unsigned char *ter)
{
	top_node *temp = NULL;
	top_node *last_temp = NULL;
	int ret = 0;
	pthread_rwlock_wrlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	last_temp = temp;
	while(NULL != temp)		//���Ҷ�Ӧsocket
	{
		if(1 == CompareUcharArray(temp->Ter, ter, TER_ADDR_LEN))
			break;
		last_temp = temp;
		temp = temp->next;
	}

	if(temp == NULL)
	{
		ret = -1;
	}
	else
	{
		if(temp == _hld_top.frist)	//ɾ����һ��
		{
			if(_hld_top.last == _hld_top.frist)
			{
				_hld_top.frist = NULL;
				_hld_top.last = NULL;
			}
			else
			{
				_hld_top.frist = _hld_top.frist->next;
			}
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		else
		{
			if(temp == _hld_top.last)
			{
				_hld_top.last = last_temp;
			}
			last_temp->next = temp->next;
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		_hld_top.num--;
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);

	return ret;
}

/*
 * ��������:�鿴�ն˵�ַ���׽����Ƿ���ͬһ�ڵ���
 * ����:		s	�׽���
 * 			ter	�ն˵�ַ
 * ����ֵ: 0 ��ͬһ�ڵ�  -1����ͬһ�ڵ�
 * */
int check_hld_top_node_s_ter(int s, unsigned char *ter)
{
	int ret = 0;
	top_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->mmutex);
		if(1 != CompareUcharArray(ter, temp->Ter, TER_ADDR_LEN))
			ret = -1;
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:ɾ�����ն˵�ַ��Ӧ�ڵ���ն˵�ַ
 * ����:		ter		�ն˵�ַ
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int del_hld_top_node_ter_ter(unsigned char *ter)
{
	int ret = 0;
	top_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->Ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->mmutex);
		memset(temp->Ter, 0xFF, TER_ADDR_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);

	return ret;
}

/*
 * ��������:�鿴��ֵ�������Ƿ��и��׽���
 * ����:		s	�׽���
 * ����ֵ: 0����  -1������
 * */
int check_hld_top_node_s(int s)
{
	top_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:�鿴��ֵ�������Ƿ��и��ն�
 * ����:		ter		�ն˵�ַ
 * ����ֵ: 0 ����		-1 ������
 * */
int check_hld_top_node_ter(unsigned char *ter)
{
	int ret = 0;
	top_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->Ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:�޸��׽��ֶ�Ӧ�Ľڵ���ն˵�ַ
 * ����:		s	�׽���
 * 			ter	�ն˵�ַ
 * ����ֵ:	0 �ɹ� -1ʧ��
 * */
int update_hld_top_node_s_ter(int s, unsigned char *ter)
{
	top_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->mmutex);
		memcpy(temp->Ter, ter, TER_ADDR_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:�޸��׽��ֶ�Ӧ�ڵ����������ʱ
 * ����:		s		�׽���
 * 			ticker	��������ʱ
 * ����ֵ:0 �ɹ�  -1ʧ��
 * */
int update_hld_top_node_s_ticker(int s, int ticker)
{
	top_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->mmutex);
		temp->ticker = ticker;
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);

	return ret;
}

/*
 * ��������:�����׽����޸Ķ�Ӧ�ڵ������ͨ��ʱ��
 * ����:		s		�׽���
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int update_hld_top_node_s_stime(int s)
{
	top_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->mmutex);
		time_t timer;
		struct tm* t_tm;
		time(&timer);
		t_tm = localtime(&timer);
		//��
		temp->last_t[0] = HexToBcd(t_tm->tm_year - 100);
		//��
		temp->last_t[1] = HexToBcd(t_tm->tm_mon + 1);
		//��
		temp->last_t[2] = HexToBcd(t_tm->tm_mday);
		//ʱ
		temp->last_t[3] = HexToBcd(t_tm->tm_hour);
		//��
		temp->last_t[4] = HexToBcd(t_tm->tm_mday);
		//��
		temp->last_t[5] = HexToBcd(t_tm->tm_sec);
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);

	return ret;
}

/*
 * ��������:���ݽڵ��׽��ַ�������
 * ����:		s		socket
 * 			inbuf	����������
 * 			len		�������ݳ���
 * ����ֵ: <0 ʧ��  >=0�ɹ�
 * */
int send_hld_top_node_s_data(int s, unsigned char *inbuf, int len)
{
	top_node *temp = NULL;
	int flag = 0;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->smutex);

		while(1)
		{
			ret = write(temp->s, inbuf, len);
			if(ret < 0)
			{
				ret = -1;
				if(errno == EPIPE)
				{
					printf("*****erro sigpipe******\n");
					flag = 1;
				}
				break;
			}
			len -= ret;
			if(0 >= len)
				break;
		}

		pthread_mutex_unlock(&temp->smutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);

	if(1 == flag)
	{
		del_event(_epollfd, temp->s, EPOLLIN);
		dele_hld_top_node_s(temp->s);
		del_hld_top_rv_s(temp->s);
		ret = -1;
	}
	return ret;
}

/*
 * ��������:�����ն˵�ַ��������
 * ����:		ter		�ն˵�ַ
 * 			inbuf	����������
 * 			len		�������ݳ���
 * ����ֵ: <0 ʧ��  >=0�ɹ�
 * */
int send_hld_top_node_ter_data(unsigned char *ter, unsigned char *inbuf, int len)
{
	int flag = 0;
	top_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	temp = _hld_top.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->Ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	else
	{
		pthread_mutex_lock(&temp->smutex);

		while(1)
		{
			ret = write(temp->s, inbuf, len);
			if(ret < 0)
			{
				ret = -1;
				if(errno == EPIPE)
				{
					printf("*****erro sigpipe******\n");
					flag = 1;
				}
				break;
			}
			len -= ret;
			if(0 >= len)
				break;
		}

		pthread_mutex_unlock(&temp->smutex);
	}
	pthread_rwlock_unlock(&_hld_top.rwlock);
	if(1 == flag)
	{
		del_event(_epollfd, temp->s, EPOLLIN);
		dele_hld_top_node_s(temp->s);
		del_hld_top_rv_s(temp->s);
		ret = -1;
	}
	return ret;

}

/*
 * ��������:��ȡ�ڵ�������
 * ����ֵ: �ڵ�����
 * */
int get_hld_top_node_num(void)
{
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	ret = _hld_top.num;
	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:��ֵ����ڵ���Ż�ȡ�ն���Ӧ����
 * ����:		num		���
 * 			ter		����ն˵�ַ
 * 			lstime	���ͨ��ʱ��
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int get_hld_top_node_ter_lstime(int num, unsigned char *ter, unsigned char *lstime)
{
	top_node *temp = NULL;
	int ret = 0;
	int i = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	if(num > _hld_top.num)
	{
		ret = -1;
	}
	else
	{
		temp = _hld_top.frist;
		for(i = 1; i < num; i++)
		{
			temp = temp->next;
		}
		pthread_mutex_lock(&temp->mmutex);
		memcpy(ter, temp->Ter, TER_ADDR_LEN);
		memcpy(lstime, temp->last_t, TIME_FRA_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}

	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:��ֵ����ڵ���Ż�ȡ�ն˵�ַ
 * ����:		num		���
 * 			ter		����ն˵�ַ
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int get_hld_top_node_ter(int num, unsigned char *ter)
{
	top_node *temp = NULL;
	int ret = 0;
	int i = 0;
	pthread_rwlock_rdlock(&_hld_top.rwlock);
	if(num > _hld_top.num)
	{
		ret = -1;
	}
	else
	{
		temp = _hld_top.frist;
		for(i = 1; i < num; i++)
		{
			temp = temp->next;
		}
		pthread_mutex_lock(&temp->mmutex);
		memcpy(ter, temp->Ter, TER_ADDR_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}

	pthread_rwlock_unlock(&_hld_top.rwlock);
	return ret;
}

/*
 * ��������:��������ʱ����߳�
 * ����:	arg	����epoll��ؼ��Ͼ��
 * */
void *TopSocketTicker(void *arg)
{
	int epoll_fd = *(int *)arg;
	top_node *temp = NULL;
	top_node *last_temp = NULL;
	//���뿴�Ź�
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	printf("NETWOER1  SOCKER��TICKER WDT ID %d\n", wdt_id);
	//���ÿ��Ź�
	set_watch_dog(wdt_id, 10);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��

		sleep(1);
		feed_watch_dog(wdt_id);	//ι��
		pthread_rwlock_wrlock(&_hld_top.rwlock);
		printf("TOP NUM: %d\n",_hld_top.num);

		temp = _hld_top.frist;
		last_temp = temp;
		while(NULL != temp)
		{
			if(0 >= temp->ticker)
			{
				printf("socket :ticker timer\n");
				if(temp == _hld_top.frist)	//ɾ����һ��
				{
					if(_hld_top.last == _hld_top.frist)
					{
						_hld_top.frist = NULL;
						_hld_top.last = NULL;
					}
					else
					{
						_hld_top.frist = _hld_top.frist->next;
					}
					pthread_mutex_destroy(&temp->mmutex);
					pthread_mutex_destroy(&temp->smutex);
					del_event(epoll_fd, temp->s, EPOLLIN);
					del_hld_top_rv_s(temp->s);
					close(temp->s);
					free(temp);
					temp = _hld_top.frist;
					last_temp = temp;
				}
				else
				{
					last_temp->next = temp->next;
					if(temp == _hld_top.last)
					{
						_hld_top.last = last_temp;
					}
					pthread_mutex_destroy(&temp->mmutex);
					pthread_mutex_destroy(&temp->smutex);
					del_event(epoll_fd, temp->s, EPOLLIN);
					del_hld_top_rv_s(temp->s);
					close(temp->s);
					free(temp);
					temp = last_temp->next;
				}
				_hld_top.num--;
			}
			else
			{
				temp->ticker--;
				last_temp = temp;
				temp = temp->next;
			}
		}

		pthread_rwlock_unlock(&_hld_top.rwlock);
	}
	pthread_exit(NULL);
}
