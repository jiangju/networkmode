/*
 * TopBuf.c
 *
 *  Created on: 2016��12��13��
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "DL376_1.h"
#include "TopBuf.h"

static hld_toprv _top_rv;

/*
 * ��������:��ʼ����ֵ���ܻ�������
 * */
void init_hld_top_rv(void)
{
	_top_rv.num = 0;
	_top_rv.node = NULL;
	pthread_rwlock_init(&_top_rv.rwlock, NULL);		//��ʼ����д��
}

/*
 * ��������:�����׽�����ӳ�ֵ���ջ���ڵ�
 * ����:		s	�׽���
 * ����ֵ:	0 �ɹ�  -1 ʧ��
 * */
int add_hld_top_rv_s(int s)
{
	int ret = 0;
	top_rv *temp = NULL;
	top_rv *node = (top_rv*)malloc(sizeof(top_rv));
	if(NULL == node)
	{
		perror("malloc socket_rv:");
		return -1;
	}

	memset(node->buff, 0, SOCKET_RV_MAX);
	node->s = s;
	node->r = 0;
	node->w = 0;
	node->next = NULL;
	pthread_mutex_init(&node->mutex, NULL);	//��ʼ����

	pthread_rwlock_wrlock(&_top_rv.rwlock);	//д��
	temp = _top_rv.node;
	node->next = temp;
	_top_rv.node = node;
	_top_rv.num++;
	pthread_rwlock_unlock(&_top_rv.rwlock);
	return ret;
}

/*
 * ��������:�����׽���ɾ����ֵ���ܻ���ڵ�
 * ����:		s	�׽���
 * ����ֵ: 0�ɹ� -1ʧ��
 * */
int del_hld_top_rv_s(int s)
{
	int ret = 0;
	top_rv *temp = NULL;
	top_rv *last_temp = NULL;
	pthread_rwlock_wrlock(&_top_rv.rwlock);	//д��

	temp = _top_rv.node;
	last_temp = temp;
	while(NULL != temp)
	{
		if(s == temp->s)
			break;
		last_temp = temp;
		temp = temp->next;
	}

	if(NULL == temp)
	{
		ret = -1;
	}
	else
	{
		if(temp == _top_rv.node)	//ɾ�������һ���ڵ�
		{
			_top_rv.node = temp->next;
		}
		else
		{
			last_temp->next = temp->next;
		}
		pthread_mutex_destroy(&temp->mutex);
		free(temp);
		_top_rv.num--;
		ret = 0;
	}
	pthread_rwlock_unlock(&_top_rv.rwlock);
	return ret;
}

/*
 * ��������:�жϳ�ֵ���ջ����������Ƿ��и��׽��ֵĽ��ջ���
 * ����:		s	�׽���
 * ����ֵ:0����  -1������
 * */
int check_hld_top_rv_s(int s)
{
	int ret = 0;
	top_rv *temp = NULL;
	pthread_rwlock_rdlock(&_top_rv.rwlock);	//����
	temp = _top_rv.node;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_top_rv.rwlock);
	return ret;
}

/*
 * ��������:����ֵ���ջ���
 * ����:		s		�׽���
 * 			inbuf	��������
 * 			len		���ݳ���
 * ����ֵ:	0�ɹ�  -1ʧ��
 * */
int update_hld_top_rv_buff(int s, unsigned char *inbuf, int len)
{
	int ret = 0;
	int l = 0;
	int j = 0;
	top_rv *temp = NULL;
	pthread_rwlock_rdlock(&_top_rv.rwlock);	//����
	temp = _top_rv.node;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
	{
		printf("not fount\n");
		ret = -1;
	}
	else
	{
		pthread_mutex_lock(&temp->mutex);
		if(temp->w >= temp->r)  //д>=��
		{
			l = SOCKET_RV_MAX - temp->w + temp->r;
		}
		else if(temp->w < temp->r) //д<��
		{
			l = temp->r - temp->w;
		}

		if(l < len)
		{
			printf("len not ok\n");
			ret = -1;
		}
		else
		{
			for(j=0; j<len; j++)
			{
				temp->buff[temp->w] = inbuf[j];
				temp->w++;
				temp->w %= SOCKET_RV_MAX;
			}
		}
		pthread_mutex_unlock(&temp->mutex);
	}

	pthread_rwlock_unlock(&_top_rv.rwlock);
	return ret;
}

/*
 * ��������:������Ӧsocket��ֵ���ջ����е�����(376.1)
 * ����:		s		�׽���
 * 			outbuf	���������
 * 			len		�������
 * 			src		�������Դ
 * ����ֵ: 0�ɹ�  -1ʧ��
 * */
int analy_hld_top_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src)
{
	int ret = 0;
	top_rv *temp = NULL;
	tp3761Buffer buf;

	pthread_rwlock_rdlock(&_top_rv.rwlock);	//����
	temp = _top_rv.node;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
	{
		ret = -1;
		printf("+++++++++\n");
	}
	else
	{
		pthread_mutex_lock(&temp->mutex);
		ret = ProtoAnaly_Get376_1BufFromCycBuf(temp->buff, SOCKET_RV_MAX, &temp->r, &temp->w, THR, &buf);
		if(0 == ret)
		{
			*len = buf.Len;
			memcpy(outbuf, buf.Data, (*len));
			*src = buf.src;
		}
		pthread_mutex_unlock(&temp->mutex);
	}
	pthread_rwlock_unlock(&_top_rv.rwlock);
	return ret;
}

