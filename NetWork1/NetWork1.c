/*
 * NetWork1.c
 *
 *  Created on: 2016��8��9��
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
 *��������:������̫��1����
 *����: arg		�ն˺��׽��ֽڵ�
 *����ֵ:��
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
			//��ά·������
			memcpy(ter, rvframe3761.Frame376_1Link.AddrField.WardCode, 2);
			memcpy(ter+2, rvframe3761.Frame376_1Link.AddrField.Addr, 2);
			//�鿴�������Ƿ���ڸ��ն˵�ַ
			ret = check_hld_top_node_ter(ter);
			if(0 == ret)	//����
			{
				//�ն˵�ַ���ڵ�λ���Ƿ��Ǹ��׽������ڵ�λ��
				if(0 != check_hld_top_node_s_ter(s, ter))	//����
				{
					//�޸��׽��ֶ�Ӧ���ն˵�ַ
					del_hld_top_node_ter_ter(ter);	//
					update_hld_top_node_s_ter(s, ter);
				}
			}
			else	//������
			{
				//�����ն˵�ַ����׽���ƥ��
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
 *��������:�����߳�1
 * */
void *NetWork1(void *arg)
{
	int ret = 0;
	int *s_arg = NULL;
	//���뿴�Ź�
	int wdt_id = *(int *)arg;
	feed_watch_dog(wdt_id);	//ι��
	printf("NETWORK1  WDT  ID %d\n", wdt_id);

	//����epoll
	_topup_epoll = epoll_create(NETWORK_MAX_CONNCET);
	if(0 > _topup_epoll)
	{
		perror("epoll_create");
		pthread_exit(NULL);
	}

	//��ʼ�������
	pthread_mutex_lock(&_RunPara.mutex);
	int s = init_server(NULL, _RunPara.IpPort.TopPort, 128);
	pthread_mutex_unlock(&_RunPara.mutex);
	if(0 > s)
	{
		close(_topup_epoll);
		pthread_exit(NULL);
	}

	//�����socket������
	if(0 > set_nonblock(s))
	{
		close(_topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	printf("Wait for a new connect...\n");

	//����epoll �����¼�
	if(0 > add_event(_topup_epoll, s, EPOLLIN))
	{
		perror("add_even");
		close(_topup_epoll);
		close(s);
		pthread_exit(NULL);
	}

	//����socket����ά���߳�
	pthread_t pt;
	if(0 != pthread_create(&pt, NULL, TopSocketTicker, (void *)&_topup_epoll))
	{
		perror("NetWork 173");
		while(1);	//�����߳�ʧ�ܣ���ʹ���Ź���λ
	}

	int i = 0;
	int len = 0;
	int k = 3;		//��ȡ�������Դ���
	unsigned char buf[SOCKET_RV_MAX] = {0};
	unsigned char temp_ter[TER_ADDR_LEN];
	memset(temp_ter, 0xFF, TER_ADDR_LEN);

	while(1)
	{
		feed_watch_dog(wdt_id);	//ι��
		//����epoll�е��׽���
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

		//���������ѷ������¼�
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
					//�������ӵ��׽��ּ��뵽��ֵ������
					feed_watch_dog(wdt_id);	//ι��
					ret = check_hld_top_rv_s(rws);	//�Ƿ��и��׽��ֵĽ��ջ���
					if(0 != ret)	//û��
					{
						if(0 != add_hld_top_rv_s(rws))	//������ջ���ʧ��
						{
							del_event(_topup_epoll, rws, EPOLLIN);		//·��������ӽڵ�ʧ��
							close(rws);
							continue;
						}
					}
					ret = check_hld_top_node_s(rws);				//·���������Ƿ����rws
					if(0 == ret)									//����
					{
						dele_hld_top_node_s(rws);					//�رո��׽���
						del_hld_top_rv_s(rws);							//ɾ�����ջ���
						del_event(_topup_epoll, rws, EPOLLIN);
					}
					else
					{
						if(0 != add_hld_top_node(rws))				//���·�ɽڵ�
						{
							del_hld_top_rv_s(rws);						//ɾ�����ջ���
							del_event(_topup_epoll, rws, EPOLLIN);		//·��������ӽڵ�ʧ��
							close(rws);
						}
					}
				}
			}
			else
			{
				feed_watch_dog(wdt_id);	//ι��
				ret = check_hld_top_rv_s(top_evt[i].data.fd);
				if(0 != ret)
				{
					if(0 != add_hld_top_rv_s(top_evt[i].data.fd))	//������ջ���ʧ��
					{
						del_event(_topup_epoll, top_evt[i].data.fd, EPOLLIN);		//·��������ӽڵ�ʧ��
						close(top_evt[i].data.fd);
						continue;
					}
				}
				ret = check_hld_top_node_s(top_evt[i].data.fd);
				if(0 != ret)	//·���������޸��׽���
				{
					del_event(_topup_epoll, top_evt[i].data.fd, EPOLLIN);
					close(top_evt[i].data.fd);
					continue;
				}
				memset(buf, 0, SOCKET_RV_MAX);

				//��ȡ3�ζ���ȡ����������ر��׽���
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
					feed_watch_dog(wdt_id);	//ι��
					dele_hld_top_node_s(top_evt[i].data.fd);
					continue;
				}
				feed_watch_dog(wdt_id);	//ι��

				/*����ȡ�������ݴ�����Ӧ�Ľ��ջ�����*/
				if(0 == update_hld_top_rv_buff(top_evt[i].data.fd, buf, len))	//��ӳɹ�
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
				feed_watch_dog(wdt_id);	//ι��
			}
		}
	}
	close(_topup_epoll);
	close(s);
	pthread_join(pt, NULL);
	pthread_exit(NULL);
}
