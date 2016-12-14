/*
 * NetWork1.c
 *
 *  Created on: 2016年8月9日
 *      Author: j
 */
#define	_NETWORK1_C_
#include "NetWork1.h"
#undef	_NETWORK1_C_
#include <stdlib.h>
#include "PthreadPool.h"
#include "DL376_1.h"
#include <stdbool.h>
#include "CommLib.h"
#include "HLDWatchDog.h"
#include "HLDTcp.h"
#include "TopRoute.h"
#include "TopBuf.h"

/*
 *函数功能:处理以太网1接收
 *参数: arg		终端和套接字节点
 *返回值:无
 * */
void *NetWorkJob1(void *arg)
{
	int s = *(int*)(arg);
	free(arg);

	tp3761Buffer outbuf;
	tpFrame376_1 rvframe3761;
	tpFrame376_1 snframe3761;
	unsigned char ter[TER_ADDR_LEN] = {0};
	int ret = 0;
	while(1)
	{
		usleep(1);
		ret = analy_hld_top_rv_buff(s, outbuf.Data, &outbuf.Len, &outbuf.src);
		if(0 != ret)
		{
			break;
		}
//
//		int jjj = 0;
//		printf("KKK:");
//		for(jjj = 0; jjj < outbuf.Len; jjj++)
//			printf("%02x ", outbuf.Data[jjj]);
//		printf("\n");

		DL376_1_LinkFrame(&outbuf, &rvframe3761);
		if(true == rvframe3761.IsHaving)
		{
			//先维路由链表
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//查看链表中是否存在该终端地址
			ret = check_hld_top_node_ter(ter);
			if(0 == ret)	//存在
			{
				//终端地址所在的位置是否是该套接字所在的位置
				if(0 != check_hld_top_node_s_ter(s, ter))	//不是
				{
					//修改套接字对应的终端地址
					del_hld_top_node_ter_ter(ter);	//
					update_hld_top_node_s_ter(s, ter);
				}
			}
			else	//不存在
			{
				//将该终端地址与该套接字匹配
				update_hld_top_node_s_ter(s, ter);
			}

			if(0 != update_hld_top_node_s_ticker(s, SOCKET_TICKER))
			{
				printf("update_hld_route_node_s_ticker erro\n");
			}
			update_hld_top_node_s_stime(s);

			DL3761_Process_Response(&rvframe3761, &snframe3761);
		}

		if(true == snframe3761.IsHaving)
		{
			if(0 == DL3761_Protocol_LinkPack(&snframe3761, &outbuf))
			{
				send_hld_top_node_s_data(s, outbuf.Data, (int)outbuf.Len);
			}
		}
	}
	return NULL;
}

/*
 *函数功能:网络线程1
 * */
void *NetWork1(void *arg)
{
	int ret = 0;
	int *s_arg = NULL;
	//申请看门狗
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//喂狗
	printf("NETWORK1  WDT  ID %d\n", wdt_id);

	//建立epoll
	_topup_epoll = epoll_create(NETWORK_MAX_CONNCET);
	if(0 > _topup_epoll)
	{
		perror("epoll_create");
		pthread_exit(NULL);
	}

	//初始化服务端
	pthread_mutex_lock(&_RunPara.mutex);
	int s = init_server(NULL, _RunPara.IpPort.TopPort, 128);
	pthread_mutex_unlock(&_RunPara.mutex);
	if(0 > s)
	{
		close(_topup_epoll);
		pthread_exit(NULL);
	}

	//服务端socket非阻塞
	if(0 > set_nonblock(s))
	{
		close(_topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	printf("Wait for a new connect...\n");

	//加入epoll 监听事件
	if(0 > add_event(_topup_epoll, s, EPOLLIN))
	{
		perror("add_even");
		close(_topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	//建立socket心跳维护线程
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, TopSocketTicker, (void *)&_topup_epoll))
	{
		perror("NetWork 173");
		while(1);	//创建线程失败，迫使看门狗复位
	}

	int i = 0;
	int len = 0;
	int k = 3;		//读取接受重试次数
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//喂狗
		//监听epoll中的套接字
		int num = epoll_wait(_topup_epoll, top_evt, EVTMAX, 3000);
		if(0 > num)
		{
			perror("epoll_wait");
			break;
		}
		else if(0 == num)
		{
			continue;
		}

		//遍历处理已发生的事件
		for(i = 0; i < num; i++)
		{
			if(top_evt[i].data.fd == s)
			{
				struct sockaddr_in addr;
				memset(&addr, 0, sizeof(addr));
				socklen_t addrlen = sizeof(addr);
				int rws = accept(s, (struct sockaddr *)&addr, &addrlen);
				if(0 > rws)
				{
					perror("accept");
					continue;
				}
				if(0 > set_nonblock(rws))
				{
					close(rws);
					continue;
				}
				if(0 > add_event(_topup_epoll, rws, EPOLLIN))
				{
					perror("add_even");
					close(rws);
					continue;
				}
				else
				{
					//将新连接的套接字加入到充值链表中
					feed_watch_dog(wdt_id);	//喂狗
					ret = check_hld_top_rv_s(rws);	//是否有该套接字的接收缓存
					if(0 != ret)	//没有
					{
						if(0 != add_hld_top_rv_s(rws))	//申请接收缓存失败
						{
							del_event(_topup_epoll, rws, EPOLLIN);		//路由链表添加节点失败
							close(rws);
							continue;
						}
					}
					ret = check_hld_top_node_s(rws);				//路由链表中是否存在rws
					if(0 == ret)									//存在
					{
						dele_hld_top_node_s(rws);					//关闭该套接字
						del_hld_top_rv_s(rws);							//删除接收缓存
						del_event(_topup_epoll, rws, EPOLLIN);
					}
					else
					{
						if(0 != add_hld_top_node(rws))				//添加路由节点
						{
							del_hld_top_rv_s(rws);						//删除接收缓存
							del_event(_topup_epoll, rws, EPOLLIN);		//路由链表添加节点失败
							close(rws);
						}
					}
				}
			}
			else
			{
				feed_watch_dog(wdt_id);	//喂狗
				ret = check_hld_top_rv_s(top_evt[i].data.fd);
				if(0 != ret)
				{
					if(0 != add_hld_top_rv_s(top_evt[i].data.fd))	//申请接收缓存失败
					{
						del_event(_topup_epoll, top_evt[i].data.fd, EPOLLIN);		//路由链表添加节点失败
						close(top_evt[i].data.fd);
						continue;
					}
				}
				ret = check_hld_top_node_s(top_evt[i].data.fd);
				if(0 != ret)	//路由链表中无该套接字
				{
					del_event(_topup_epoll, top_evt[i].data.fd, EPOLLIN);
					close(top_evt[i].data.fd);
					continue;
				}
				memset(buf, 0, SOCKET_RV_MAX);

				//读取3次都读取不到数据则关闭套接字
				k = 3;
				while(k--)
				{
					len = read(top_evt[i].data.fd, buf, NETWOEK_R_MAX);
					if(len > 0)
						break;
				}
				if(len < 0)
				{
					if(errno == EAGAIN)
					{
					        break;
					}

					if(0 > del_event(_topup_epoll, top_evt[i].data.fd, EPOLLIN))
					{
						perror("del_event");
					}
					close(top_evt[i].data.fd);
					feed_watch_dog(wdt_id);	//喂狗
					dele_hld_top_node_s(top_evt[i].data.fd);
					continue;
				}
				feed_watch_dog(wdt_id);	//喂狗

				/*将读取到的数据存入相应的接收缓存中*/
				if(0 == update_hld_top_rv_buff(top_evt[i].data.fd, buf, len))	//添加成功
				{
					s_arg = (int *)malloc(sizeof(int));
					if(NULL != s_arg)
					{
						*s_arg = top_evt[i].data.fd;
						if(-1 == threadpool_add_job(_Threadpool, NetWorkJob1, (void*)s_arg))
						{
							printf("up add job erro\n");
						}
					}
				}
				feed_watch_dog(wdt_id);	//喂狗
			}
		}
	}
	close(_topup_epoll);
	close(s);
	pthread_join(pt, NULL);
	pthread_exit(NULL);
}
