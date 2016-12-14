
/*
 * Route.c
 *
 *  Created on: 2016年6月29日
 *      Author: j
 */
#include "Route.h"
#include <string.h>
#include <stdlib.h>
#include "CommLib.h"
#include <error.h>
#include <unistd.h>
#include "HLDWatchDog.h"
#include <sys/time.h>
#include "HLDTcp.h"
#include "errno.h"
#include "RevBuf.h"

static hld_route _hld_route;

/*
 * 函数功能:初始化华立达路由链表
 * */
void init_hld_route(void)
{
	_hld_route.num = 0;
	_hld_route.frist = NULL;
	_hld_route.last = NULL;
	pthread_rwlock_init(&_hld_route.rwlock, NULL);
}

/*
 * 函数功能:根据套接字添加路由节点
 * 参数:		s 	套接字
 * 返回值: 0成功  -1失败
 * */
int add_hld_route_node(int s)
{
	route_node *node = (route_node*)malloc(sizeof(route_node));
	if(NULL == node)
	{
		perror("malloc route_node:");
		return -1;
	}

	memset(node->Ter, 0xFF, TER_ADDR_LEN);		//初始化终端地址
	memset(node->last_t, 0, TIME_FRA_LEN);		//初始化最后通信时间
	node->next = NULL;							//初始化下一个节点地址
	node->s = s;								//初始化套接字
	node->ticker = 40;							//链接后，限制40秒内收到登录报文
	pthread_mutex_init(&(node->smutex), NULL);	//初始化socket发送锁
	pthread_mutex_init(&(node->mmutex), NULL);	//访问节点锁
	//添加
	pthread_rwlock_wrlock(&_hld_route.rwlock);

	if(0 >= _hld_route.num)	//第一次添加
	{
		_hld_route.frist = node;
		_hld_route.last = node;
		_hld_route.num = 1;
	}
	else					//非第一次添加
	{
		_hld_route.last->next = node;
		_hld_route.last = node;
		_hld_route.num++;
	}

	pthread_rwlock_unlock(&_hld_route.rwlock);

	return 0;
}

/*
 * 函数功能:根据socket关闭路由节点及回收资源
 * 参数：	s		套接字
 * 返回值:	0成功 -1失败
 * */
int dele_hld_route_node_s(int s)
{
	route_node *temp = NULL;
	route_node *last_temp = NULL;
	int ret = -1;
	pthread_rwlock_wrlock(&_hld_route.rwlock);

	temp = _hld_route.frist;
	last_temp = temp;
	while(NULL != temp)		//查找对应socket
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
		if(temp == _hld_route.frist)	//删除第一个
		{
			if(_hld_route.last == _hld_route.frist)
			{
				_hld_route.frist = NULL;
				_hld_route.last = NULL;
			}
			else
			{
				_hld_route.frist = _hld_route.frist->next;
			}
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		else
		{
			if(temp == _hld_route.last)
			{
				_hld_route.last = last_temp;
			}
			last_temp->next = temp->next;
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		_hld_route.num--;
	}
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:根据终端地址关闭路由几点及回收资源
 * 参数: ter		终端地址
 * 返回值: 0成功  -1失败
 * */
int dele_hld_route_node_ter(unsigned char *ter)
{
	route_node *temp = NULL;
	route_node *last_temp = NULL;
	int ret = 0;
	pthread_rwlock_wrlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
	last_temp = temp;
	while(NULL != temp)		//查找对应socket
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
		if(temp == _hld_route.frist)	//删除第一个
		{
			if(_hld_route.last == _hld_route.frist)
			{
				_hld_route.frist = NULL;
				_hld_route.last = NULL;
			}
			else
			{
				_hld_route.frist = _hld_route.frist->next;
			}
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		else
		{
			if(temp == _hld_route.last)
			{
				_hld_route.last = last_temp;
			}
			last_temp->next = temp->next;
			pthread_mutex_destroy(&temp->mmutex);
			pthread_mutex_destroy(&temp->smutex);
			close(temp->s);
			free(temp);
		}
		_hld_route.num--;
	}
	pthread_rwlock_unlock(&_hld_route.rwlock);

	return ret;
}

/*
 * 函数功能:查看终端地址与套接字是否在同一节点上
 * 参数:		s	套接字
 * 			ter	终端地址
 * 返回值: 0 在同一节点  -1不在同一节点
 * */
int check_hld_route_node_s_ter(int s, unsigned char *ter)
{
	int ret = 0;
	route_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:删除该终端地址对应节点的终端地址
 * 参数:		ter		终端地址
 * 返回值: 0成功  -1失败
 * */
int del_hld_route_node_ter_ter(unsigned char *ter)
{
	int ret = 0;
	route_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);

	return ret;
}

/*
 * 函数功能:查看路由链表中是否有该套接字
 * 参数:		s	套接字
 * 返回值: 0存在  -1不存在
 * */
int check_hld_route_node_s(int s)
{
	route_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
	while(NULL != temp)
	{
		if(temp->s == s)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:查看路由链表中是否有该终端
 * 参数:		ter		终端地址
 * 返回值: 0 存在		-1 不存在
 * */
int check_hld_route_node_ter(unsigned char *ter)
{
	int ret = 0;
	route_node *temp = NULL;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->Ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}
	if(NULL == temp)
		ret = -1;
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:修改套接字对应的节点的终端地址
 * 参数:		s	套接字
 * 			ter	终端地址
 * 返回值:	0 成功 -1失败
 * */
int update_hld_route_node_s_ter(int s, unsigned char *ter)
{
	route_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:修改套接字对应节点的心跳倒计时
 * 参数:		s		套接字
 * 			ticker	心跳倒计时
 * 返回值:0 成功  -1失败
 * */
int update_hld_route_node_s_ticker(int s, int ticker)
{
	route_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);

	return ret;
}

/*
 * 函数功能:根据套接字修改对应节点的最后吼通信时间
 * 参数:		s		套接字
 * 返回值: 0成功  -1失败
 * */
int update_hld_route_node_s_stime(int s)
{
	route_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
		//年
		temp->last_t[0] = HexToBcd(t_tm->tm_year - 100);
		//月
		temp->last_t[1] = HexToBcd(t_tm->tm_mon + 1);
		//日
		temp->last_t[2] = HexToBcd(t_tm->tm_mday);
		//时
		temp->last_t[3] = HexToBcd(t_tm->tm_hour);
		//分
		temp->last_t[4] = HexToBcd(t_tm->tm_mday);
		//秒
		temp->last_t[5] = HexToBcd(t_tm->tm_sec);
		pthread_mutex_unlock(&temp->mmutex);
	}
	pthread_rwlock_unlock(&_hld_route.rwlock);

	return ret;
}

/*
 * 函数功能:根据节点套接字发送数据
 * 参数:		s		socket
 * 			inbuf	待发送数据
 * 			len		发送数据长度
 * 返回值: <0 失败  >=0成功
 * */
int send_hld_route_node_s_data(int s, unsigned char *inbuf, int len)
{
	route_node *temp = NULL;
	int flag = 0;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);

	if(1 == flag)
	{
		del_event(_epollfd, temp->s, EPOLLIN);
		dele_hld_route_node_s(temp->s);
		del_hld_rv_s(temp->s);
		ret = -1;
	}
	return ret;
}

/*
 * 函数功能:根据终端地址发送数据
 * 参数:		ter		终端地址
 * 			inbuf	待发送数据
 * 			len		发送数据长度
 * 返回值: <0 失败  >=0成功
 * */
int send_hld_route_node_ter_data(unsigned char *ter, unsigned char *inbuf, int len)
{
	int flag = 0;
	route_node *temp = NULL;
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	temp = _hld_route.frist;
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
	pthread_rwlock_unlock(&_hld_route.rwlock);
	if(1 == flag)
	{
		del_event(_epollfd, temp->s, EPOLLIN);
		dele_hld_route_node_s(temp->s);
		del_hld_rv_s(temp->s);
		ret = -1;
	}
	return ret;

}

/*
 * 函数功能:获取节点总数量
 * 返回值: 节点数量
 * */
int get_hld_route_node_num(void)
{
	int ret = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	ret = _hld_route.num;
	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:路由链表节点序号获取终端相应数据
 * 参数:		num		序号
 * 			ter		输出终端地址
 * 			lstime	最后通信时间
 * 返回值: 0成功  -1失败
 * */
int get_hld_route_node_ter_lstime(int num, unsigned char *ter, unsigned char *lstime)
{
	route_node *temp = NULL;
	int ret = 0;
	int i = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	if(num > _hld_route.num)
	{
		ret = -1;
	}
	else
	{
		temp = _hld_route.frist;
		for(i = 1; i < num; i++)
		{
			temp = temp->next;
		}
		pthread_mutex_lock(&temp->mmutex);
		memcpy(ter, temp->Ter, TER_ADDR_LEN);
		memcpy(lstime, temp->last_t, TIME_FRA_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}

	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:路由链表节点序号获取终端地址
 * 参数:		num		序号
 * 			ter		输出终端地址
 * 返回值: 0成功  -1失败
 * */
int get_hld_route_node_ter(int num, unsigned char *ter)
{
	route_node *temp = NULL;
	int ret = 0;
	int i = 0;
	pthread_rwlock_rdlock(&_hld_route.rwlock);
	if(num > _hld_route.num)
	{
		ret = -1;
	}
	else
	{
		temp = _hld_route.frist;
		for(i = 1; i < num; i++)
		{
			temp = temp->next;
		}
		pthread_mutex_lock(&temp->mmutex);
		memcpy(ter, temp->Ter, TER_ADDR_LEN);
		pthread_mutex_unlock(&temp->mmutex);
	}

	pthread_rwlock_unlock(&_hld_route.rwlock);
	return ret;
}

/*
 * 函数功能:心跳倒计时监控线程
 * 参数:	arg	传递epoll监控集合句柄
 * */
void *SocketTicker(void *arg)
{
	int epoll_fd = *(int *)arg;
	route_node *temp = NULL;
	route_node *last_temp = NULL;
	//申请看门狗
	int wdt_id = 0;
	wdt_id = apply_watch_dog();
	if(wdt_id < 0)
	{
		printf("usart0 apply dog error\n");
		system("reboot");
	}
	printf("NETWOER0  SOCKER　TICKER WDT ID %d\n", wdt_id);
	//设置看门狗
	set_watch_dog(wdt_id, 10);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗

		sleep(1);
		feed_watch_dog(wdt_id);	//喂狗
		pthread_rwlock_wrlock(&_hld_route.rwlock);
		printf("NUM: %d\n",_hld_route.num);

		temp = _hld_route.frist;
		last_temp = temp;
		while(NULL != temp)
		{
			if(0 >= temp->ticker)
			{
				printf("socket :ticker timer\n");
				if(temp == _hld_route.frist)	//删除第一个
				{
					if(_hld_route.last == _hld_route.frist)
					{
						_hld_route.frist = NULL;
						_hld_route.last = NULL;
					}
					else
					{
						_hld_route.frist = _hld_route.frist->next;
					}
					pthread_mutex_destroy(&temp->mmutex);
					pthread_mutex_destroy(&temp->smutex);
					del_event(epoll_fd, temp->s, EPOLLIN);
					del_hld_rv_s(temp->s);
					close(temp->s);
					free(temp);
					temp = _hld_route.frist;
					last_temp = temp;
				}
				else
				{
					last_temp->next = temp->next;
					if(temp == _hld_route.last)
					{
						_hld_route.last = last_temp;
					}
					pthread_mutex_destroy(&temp->mmutex);
					pthread_mutex_destroy(&temp->smutex);
					del_event(epoll_fd, temp->s, EPOLLIN);
					del_hld_rv_s(temp->s);
					close(temp->s);
					free(temp);
					temp = last_temp->next;
				}
				_hld_route.num--;
			}
			else
			{
				temp->ticker--;
				last_temp = temp;
				temp = temp->next;
			}
		}

		pthread_rwlock_unlock(&_hld_route.rwlock);
	}
	pthread_exit(NULL);
}
