/*
 * RevBuf.c
 *
 *  Created on: 2016年12月8日
 *      Author: j
 */

#define _REVBUF_C_
#include "RevBuf.h"
#undef _REVBUF_C_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "DL376_1.h"


static hld_rv _hld_rv;

/*
 * 函数功能:初始化接受缓存链表
 * */
void init_hld_rv(void)
{
	_hld_rv.num = 0;
	_hld_rv.node = NULL;
	pthread_rwlock_init(&_hld_rv.rwlock, NULL);		//初始化读写锁
}

/*
 * 函数功能:根据套接字添加接收缓存节点
 * 参数:		s	套接字
 * 返回值:	0 成功  -1 失败
 * */
int add_hld_rv_s(int s)
{
	int ret = 0;
	socket_rv *temp = NULL;
	socket_rv *node = (socket_rv*)malloc(sizeof(socket_rv));
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
	pthread_mutex_init(&node->mutex, NULL);	//初始化锁

	pthread_rwlock_wrlock(&_hld_rv.rwlock);	//写锁
	temp = _hld_rv.node;
	node->next = temp;
	_hld_rv.node = node;
	_hld_rv.num++;
	pthread_rwlock_unlock(&_hld_rv.rwlock);
	return ret;
}

/*
 * 函数功能:根据套接字删除接受缓存节点
 * 参数:		s	套接字
 * 返回值: 0成功 -1失败
 * */
int del_hld_rv_s(int s)
{
	int ret = 0;
	socket_rv *temp = NULL;
	socket_rv *last_temp = NULL;
	pthread_rwlock_wrlock(&_hld_rv.rwlock);	//写锁

	temp = _hld_rv.node;
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
		if(temp == _hld_rv.node)	//删除链表第一个节点
		{
			_hld_rv.node = temp->next;
		}
		else
		{
			last_temp->next = temp->next;
		}
		pthread_mutex_destroy(&temp->mutex);
		free(temp);
		_hld_rv.num--;
		ret = 0;
	}
	pthread_rwlock_unlock(&_hld_rv.rwlock);
	return ret;
}

/*
 * 函数功能:判断接收缓存链表中是否有该套接字的接收缓存
 * 参数:		s	套接字
 * 返回值:0存在  -1不存在
 * */
int check_hld_rv_s(int s)
{
	int ret = 0;
	socket_rv *temp = NULL;
	pthread_rwlock_rdlock(&_hld_rv.rwlock);	//读锁
	temp = _hld_rv.node;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_hld_rv.rwlock);
	return ret;
}

/*
 * 函数功能:填充接收缓存
 * 参数:		s		套接字
 * 			inbuf	输入数据
 * 			len		数据长度
 * 返回值:	0成功  -1失败
 * */
int update_hld_rv_buff(int s, unsigned char *inbuf, int len)
{
	int ret = 0;
	int l = 0;
	int j = 0;
	socket_rv *temp = NULL;
	pthread_rwlock_rdlock(&_hld_rv.rwlock);	//读锁
	temp = _hld_rv.node;
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
		if(temp->w >= temp->r)  //写>=读
		{
			l = SOCKET_RV_MAX - temp->w + temp->r;
		}
		else if(temp->w < temp->r) //写<读
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

	pthread_rwlock_unlock(&_hld_rv.rwlock);
	return ret;
}

/*
 * 函数功能:解析相应socket接收缓存中的数据(376.1)
 * 参数:		s		套接字
 * 			outbuf	解析后输出
 * 			len		输出长度
 * 			src		输出数据源
 * 返回值: 0成功  -1失败
 * */
int analy_hld_rv_buff(int s, unsigned char *outbuf, unsigned short *len, unsigned char *src)
{
	int ret = 0;
	socket_rv *temp = NULL;
	tp3761Buffer buf;

	pthread_rwlock_rdlock(&_hld_rv.rwlock);	//读锁
	temp = _hld_rv.node;
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
	pthread_rwlock_unlock(&_hld_rv.rwlock);
	return ret;
}




