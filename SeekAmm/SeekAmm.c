/*
 * SeekAmm.c
 *
 *  Created on: 2016年10月13日
 *      Author: j
 */
#include <stdio.h>
#include "SeekAmm.h"
#include "CommLib.h"
#include "DL376_1.h"
#include "DL376_1_DataType.h"
#include "Route.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>

static struct initiative_stand _initiative_stand;		//定义主动台账
static pthread_mutex_t _seek_amm_file_mutex;			//主动台账文件锁
static struct seek_amm_task_queue _seek_amm_task_queue;	//定义搜表任务队列

static struct seek_amm_task_exec _exec_seek;			//定义执行任务队列

/*
 * 函数功能:初始化主动台账(只在开头调用一次)
 * */
void init_initiative_stand(void)
{
	memset(&_initiative_stand, 0, sizeof(struct initiative_stand));
	pthread_mutex_init(&_initiative_stand.mutex, NULL);
	pthread_mutex_init(&_seek_amm_file_mutex, NULL);
	destroy_initiative_stand();
//	destroy_initiative_stand_file();
	initiative_stand_all_index_init();
	initiative_stand_file_add_to_memory();

	pthread_t pt;
	pthread_create(&pt, NULL, SeekAmmPthread, NULL);
}

/*
 * 函数功能:修改主动台账文件某位置是否被使用
 * 参数:		index	文件位置索引
 * 			flag 	1 使用 0 未使用
 * */
void initiative_stand_index_is_using(unsigned int index, char flag)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	if(flag == 0)
		_initiative_stand.index[index] = 0x33;
	else
		_initiative_stand.index[index] = 0x01;

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * 函数功能:修改主动台账文件所有位置都没有使用
 * */
void initiative_stand_all_index_init(void)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	memset(_initiative_stand.index, 0x33, NETWORK_MAX_CONNCET);

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * 函数功能:返回最小未使用索引
 * */
int initiative_stand_min_not_using_index(void)
{
	int i = 0;
	pthread_mutex_lock(&_initiative_stand.mutex);
	for(i = 0; i < NETWORK_MAX_CONNCET; i++)
	{
		if(_initiative_stand.index[i] == 0x33)
		{
			pthread_mutex_unlock(&_initiative_stand.mutex);
			return i;
		}
	}
	pthread_mutex_unlock(&_initiative_stand.mutex);
	return -1;
}

/*
 * 函数功能:清空主动台账
 * */
void destroy_initiative_stand(void)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	struct seek_amm_node *temp = _initiative_stand.frist;
	while(NULL != _initiative_stand.frist)
	{
		temp = _initiative_stand.frist;
		_initiative_stand.frist = _initiative_stand.frist->next;
		free(temp);
	}
	_initiative_stand.last = NULL;
	_initiative_stand.nun = 0;

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * 函数功能:清空主动台账文件
 * */
int destroy_initiative_stand_file(void)
{
	int i = 0;
	int fd;
	struct seek_amm_result_store r_result;
	initiative_stand_all_index_init();
	pthread_mutex_lock(&_seek_amm_file_mutex);
	while(i--)
	{
		fd = open(SEEK_AMM_FILE, O_RDWR | O_CREAT, 0666);
		if(fd >= 0)
		{
			break;
		}
	}
	if(fd < 0)
	{
		pthread_mutex_unlock(&_seek_amm_file_mutex);
		return -1;
	}
	memset(&r_result, 0xFF, sizeof(struct seek_amm_result_store));
	for(i = 0; i < NETWORK_MAX_CONNCET; i++)
	{
		write(fd, &r_result, sizeof(struct seek_amm_result_store));
	}
	close(fd);
	pthread_mutex_unlock(&_seek_amm_file_mutex);
	return 0;
}

/*
 * 函数功能:根据终端地址获取主动台账中的搜表结果
 * 参数:		ter		终端地址
 *			result	搜表结果
 * 返回值:	0成功  -1 失败 或未找到
 * */
int get_seek_amm_result(unsigned char *ter, struct seek_amm_result *result)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	struct seek_amm_node *temp = _initiative_stand.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(temp->result.ter, ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}

	if(NULL != temp)
	{
		memcpy(result, &temp->result, sizeof(struct seek_amm_result));
		pthread_mutex_unlock(&_initiative_stand.mutex);
		return 0;
	}

	pthread_mutex_unlock(&_initiative_stand.mutex);
	return -1;
}

/*
 * 函数功能:修改或添加搜表结果到主动台账(内存)
 * 参数:		result	搜表结果
 * 返回值: 	0成功 -1失败
 * */
int add_seek_amm_result(struct seek_amm_result *result)
{
	if(NULL == result)
		return -1;

	pthread_mutex_lock(&_initiative_stand.mutex);

	struct seek_amm_node *temp = _initiative_stand.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(temp->result.ter, result->ter, TER_ADDR_LEN))
			break;
		temp = temp->next;
	}

	if(NULL != temp)	//修改
	{
		memcpy(&temp->result, result, sizeof(struct seek_amm_result));
	}
	else				//添加
	{
		temp = (struct seek_amm_node *)malloc(sizeof(struct seek_amm_node));
		if(NULL == temp)
		{
			pthread_mutex_unlock(&_initiative_stand.mutex);
			return -1;
		}
		temp->next = NULL;
		memcpy(&temp->result, result, sizeof(struct seek_amm_result));

		if(NULL == _initiative_stand.frist)	//第一次添加
		{
			_initiative_stand.frist = temp;
			_initiative_stand.last = temp;
		}
		else								//非第一次添加
		{
			_initiative_stand.last->next = temp;
			_initiative_stand.last = temp;
		}
	}

	pthread_mutex_unlock(&_initiative_stand.mutex);

	return 0;
}

/*
 * 函数功能:将搜表结果写入主动台账文件中
 * 参数:		result	搜表结果
 * 返回值:	0 成功	-1 失败
 * */
int write_seek_amm_result(struct seek_amm_result *result)
{
	if(NULL == result)
		return -1;
	pthread_mutex_lock(&_seek_amm_file_mutex);

	int i = 3;
	int fd;
	int ret = 0;
	int offset = 0;
	struct seek_amm_result_store result_store;

	while(i--)
	{
		fd = open(SEEK_AMM_FILE, O_RDWR | O_CREAT, 0666);
		if(fd >= 0)
		{
			break;
		}
	}

	if(fd < 0)
	{
		pthread_mutex_unlock(&_seek_amm_file_mutex);
		return -1;
	}

	memcpy(&result_store.result, result, sizeof(struct seek_amm_result));
	offset = offsetof(struct seek_amm_result_store, cs);
	result_store.cs = Func_CS((void*)&result_store, offset);
	offset = (result_store.result.index) * sizeof(struct seek_amm_result_store);
	ret = WriteFile(fd, offset, (void *)&result_store, sizeof(struct seek_amm_result_store));

	close(fd);
	pthread_mutex_unlock(&_seek_amm_file_mutex);

	initiative_stand_index_is_using(result_store.result.index, 1);
	return ret;
}

/*
 * 函数功能:删除主动台账中的某个节点
 * 参数:		ter		终端地址
 * 返回值:
 * */
void dele_seek_amm_result(unsigned char *ter)
{
	if(NULL == ter)
		return;
	pthread_mutex_lock(&_initiative_stand.mutex);

	struct seek_amm_node *temp = _initiative_stand.frist;
	struct seek_amm_node *temp_temp = _initiative_stand.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(temp->result.ter, ter, TER_ADDR_LEN))
			break;
		temp_temp = temp;
		temp = temp->next;
	}

	if(NULL == temp)
	{
		pthread_mutex_unlock(&_initiative_stand.mutex);
		return;
	}
	if(temp == _initiative_stand.frist)	//删除第一个
	{
		_initiative_stand.frist = _initiative_stand.frist->next;
		if(NULL == _initiative_stand.frist)
			_initiative_stand.last = NULL;
		free(temp);
	}
	else	//删除非第一个
	{
		temp_temp->next = temp->next;
		free(temp);
		if(NULL == temp_temp->next)
			_initiative_stand.last = temp_temp;
	}

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * 函数功能:将主动台账文件的搜表结果，加入到内存中的主动台账中
 * 返回值: 0 成功 -1失败
 * */
int initiative_stand_file_add_to_memory(void)
{
	unsigned int i = 3;
	int fd;
	int ret = 0;
	int len = 0;
	struct seek_amm_result_store r_result;

	pthread_mutex_lock(&_seek_amm_file_mutex);

	if(0 == access(SEEK_AMM_FILE, F_OK))	//存在文件
	{
		while(i--)
		{
			fd = open(SEEK_AMM_FILE, O_RDWR, 0666);
			if(fd >= 0)
			{
				break;
			}
		}
		if(fd < 0)
		{

			pthread_mutex_unlock(&_seek_amm_file_mutex);
//			initiative_stand_all_index_init();
			return -1;
		}

		for(i = 0; i < NETWORK_MAX_CONNCET; i++)
		{
			memset(&r_result, 0, sizeof(struct seek_amm_result_store));
			ret = read(fd, &r_result, sizeof(struct seek_amm_result_store));
			if(ret != sizeof(struct seek_amm_result_store))
				break;
			len = offsetof(struct seek_amm_result_store, cs);
			if(r_result.cs == Func_CS(&r_result, len))	//内容正确
			{
				if(0x55 == r_result.result.flag)	//有效
				{

					add_seek_amm_result(&r_result.result);
					initiative_stand_index_is_using(i, 1);
				}
				else	//无效
				{
					initiative_stand_index_is_using(i, 0);	//标志该位置未使用
				}
			}
			else
			{
				initiative_stand_index_is_using(i, 0);	//标志该位置未使用
			}
		}

		close(fd);
	}
	else									//不存在文件
	{
//		initiative_stand_all_index_init();
		while(i--)
		{
			fd = open(SEEK_AMM_FILE, O_RDWR | O_CREAT, 0666);
			if(fd >= 0)
			{
				break;
			}
		}
		if(fd < 0)
		{
			pthread_mutex_unlock(&_seek_amm_file_mutex);
			return -1;
		}
		memset(&r_result, 0xFF, sizeof(struct seek_amm_result_store));
		for(i = 0; i < NETWORK_MAX_CONNCET; i++)
		{
			write(fd, &r_result, sizeof(struct seek_amm_result_store));
		}
		close(fd);
	}

	pthread_mutex_unlock(&_seek_amm_file_mutex);
	return 0;
}

/*
 * 函数功能:初始化搜表任务队列
 * */
void seek_amm_task_queue_init(void)
{
	_seek_amm_task_queue.frist = NULL;
	_seek_amm_task_queue.last = NULL;
	pthread_mutex_init(&_seek_amm_task_queue.mutex, NULL);
	pthread_cond_init(&_seek_amm_task_queue.queue_nonempty, NULL);
}

/*
 * 函数功能:搜表队列是否为空
 * 返回值 0 空  -1 fei空
 * */
int seek_amm_task_empty(void)
{
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	if(NULL == _seek_amm_task_queue.frist)
	{
		pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
		return 0;
	}
	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
	return -1;
}

/*
 * 函数功能:查询搜表任务个数
 * 返回值:任务数量
 * */
int get_seek_amm_task_num(void)
{
	int num = 0;
	struct seek_amm_task *temp;
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	temp = _seek_amm_task_queue.frist;

	while(temp != NULL)
	{
		num++;
		temp = temp->next;
	}
	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
	return num;
}

/*
 * 函数功能:查看搜表队列中是否有该终端
 * 参数:	ter		终端地址
 * 		task	输出任务
 * 返回值 0 有  -1 没有
 * */
int find_seek_amm_task(unsigned char *ter, struct seek_amm_task *task)
{
	if(ter == NULL)
		return -1;
	struct seek_amm_task *temp;
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	temp = _seek_amm_task_queue.frist;

	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->ter, TER_ADDR_LEN))
		{
			memcpy(task, temp, sizeof(struct seek_amm_task));
			pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
			return 0;
		}
		temp = temp->next;
	}

	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);

	return -1;
}

/*
 * 函数功能:添加搜表任务到队列
 * 参数:		task	搜表任务
 * 返回值: 0 成功 -1失败
 * */
int add_seek_amm_task(struct seek_amm_task *task)
{
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);

	if(NULL == _seek_amm_task_queue.frist)	//第一次添加
	{
//		printf("frist add seek amm task\n");
		_seek_amm_task_queue.frist = task;
		_seek_amm_task_queue.last = _seek_amm_task_queue.frist;
		pthread_cond_broadcast(&_seek_amm_task_queue.queue_nonempty);
	}
	else									//非第一次添加
	{
//		printf("add seek amm task\n");
		_seek_amm_task_queue.last->next = task;
		_seek_amm_task_queue.last = _seek_amm_task_queue.last->next;
	}

	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);

	return 0;
}

/*
 * 函数功能:删除该终端的搜表任务
 * 参数：	ter   	终端地址
 * 返回值: 0 成功  -1 失败
 * */
int dele_seek_amm_task(unsigned char *ter)
{
	if(NULL == ter)
		return -1;
	struct seek_amm_task *temp = NULL;
	struct seek_amm_task *temp_temp = NULL;

	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	temp = _seek_amm_task_queue.frist;

	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->ter, TER_ADDR_LEN))
		{
			break;
		}
		temp_temp = temp;
		temp = temp->next;
	}

	if(NULL == temp)
	{
		pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
		return -1;
	}
	if(temp == _seek_amm_task_queue.frist)	//删除第一个
	{
		_seek_amm_task_queue.frist = temp->next;
		if(NULL == _seek_amm_task_queue.frist)
			_seek_amm_task_queue.last = NULL;
		free(temp);
	}
	else
	{
		temp_temp->next = temp->next;
		free(temp);
		if(NULL == temp_temp->next)	//删除的是最后一个
		{
			_seek_amm_task_queue.last = temp_temp;
		}
	}
	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);

	return 0;
}

/*
 * 函数功能:获取第n个任务
 * 参数:		n   第n个
 * 			task返回任务
 * 返回值:0 成功 -1 失败
 * */
int get_n_seek_amm_task(int n, struct seek_amm_task *task)
{
	if(n <= 0)
		return -1;
	struct seek_amm_task *temp;
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	temp = _seek_amm_task_queue.frist;
	while(NULL != temp)
	{
		n--;
		if(0 == n)
			break;
		temp = temp->next;
	}
	if(NULL == temp)
	{
		pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
		return -1;
	}
	memcpy(task, temp, sizeof(struct seek_amm_task));
	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
	return 0;
}

/*
 * 函数功能:查询某终端是否正在搜表
 * 参数		ter		终端地址
 * 返回值	0 正在搜表	-1没有搜表
 * */
int judge_seek_amm_task(unsigned char *ter)
{
	if(NULL == ter)
		return -1;
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);
	struct seek_amm_task *temp = NULL;

	temp = _seek_amm_task_queue.frist;
	while(NULL != temp)
	{
		if(1 == CompareUcharArray(ter, temp->ter, TER_ADDR_LEN))
		{
			if(temp->flag == 0x66)
			{
				pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
				return 0;
			}
			else
			{
				pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
				return -1;
			}
		}
		temp = temp->next;
	}
	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
	return -1;
}

/*
 * 函数功能:构造搜表任务帧
 * 参数:	ter		终端地址
 * 		key		开关		0关 1开
 * 返回值 0  成功  -1 失败
 * */
int creat_afn5_98(unsigned char *ter, unsigned char key, tpFrame376_1 *outbuf)
{
	if(NULL == ter)
		return -1;

	int index = 0;

	memset(outbuf, 0, sizeof(tpFrame376_1));
	/************************************链路层************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	outbuf->Frame376_1Link.EndChar = 0x16;
	//控制域
	outbuf->Frame376_1Link.CtlField.DIR = 0;	//下行
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//启动站
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCB位无效
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//地址域
	outbuf->Frame376_1Link.AddrField.WardCode[0] = ter[0];
	outbuf->Frame376_1Link.AddrField.WardCode[1] = ter[1];
	outbuf->Frame376_1Link.AddrField.Addr[0] = ter[2];
	outbuf->Frame376_1Link.AddrField.Addr[1] = ter[3];

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************应用层****************************************/
	//功能码
	outbuf->Frame376_1App.AFN = AFN3761_CTRL;
	//请求确认标志
	outbuf->Frame376_1App.SEQ.CON = 0;	//不需要确认
	//帧类型
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//帧序号
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//时间标签
	outbuf->Frame376_1App.SEQ.TPV = 0;	//不要时标
	//附加域 时标
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//无效
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//数据标识	0x98
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x02;
	outbuf->Frame376_1App.AppBuf[index++] = 0x0C;
	outbuf->Frame376_1App.AppBuf[index++] = key;

	//控制命令需要pw域，默认设置为16个0
	memset((outbuf->Frame376_1App.AppBuf + index), 0x00, 16);
	index += 16;

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;

	return 0;
}

/*
 * 函数功能:初始化执行任务队列
 * */
void init_exec_seek(void)
{
	_exec_seek.num = 0;
	_exec_seek.frist = NULL;
	_exec_seek.last = NULL;
}

int add_exec_seek(unsigned char *ter)
{
	struct seek_amm_ter *new_seek = (struct seek_amm_ter *)malloc(sizeof(struct seek_amm_ter));
	if(NULL == new_seek)
		return -1;

	memcpy(new_seek->ter, ter, TER_ADDR_LEN);
	new_seek->next = NULL;

	if(0 == _exec_seek.num)	//第一次添加
	{
		_exec_seek.frist = new_seek;
		_exec_seek.last = _exec_seek.frist;
	}
	else
	{
		_exec_seek.last->next = new_seek;
		_exec_seek.last = new_seek;
	}
	_exec_seek.num++;

	return 0;
}

int get_frist_exec_seek(unsigned char *ter)
{
	memset(ter, 0, TER_ADDR_LEN);
	if(_exec_seek.num < 1)
		return -1;
	struct seek_amm_ter *temp;
	if(1 == _exec_seek.num)
	{
		memcpy(ter, _exec_seek.frist->ter, TER_ADDR_LEN);
		free(_exec_seek.frist);
		_exec_seek.frist = NULL;
		_exec_seek.last = NULL;
	}
	else
	{
		memcpy(ter, _exec_seek.frist->ter, TER_ADDR_LEN);
		temp = _exec_seek.frist;
		_exec_seek.frist = _exec_seek.frist->next;
		free(temp);
	}
	_exec_seek.num--;
	return 0;
}

int get_exec_seek_num(void)
{
	return _exec_seek.num;
}

/*
 * 函数功能:搜表任务线程
 * */
void *SeekAmmPthread(void *arg)
{
	struct seek_amm_task *temp;
	struct seek_amm_task *temp_temp;

	unsigned char temp_ter[TER_ADDR_LEN] = {0};//备份的终端地址

	seek_amm_task_queue_init();
	init_exec_seek();

	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&_seek_amm_task_queue.mutex);
		while(NULL == _seek_amm_task_queue.frist)
		{
			usleep(1);
//			printf("wait seek amm task\n");
			pthread_cond_wait(&_seek_amm_task_queue.queue_nonempty, &_seek_amm_task_queue.mutex);
		}
		temp = _seek_amm_task_queue.frist;
		temp_temp = temp;
		while(NULL != temp)
		{
			if(0x66 == temp->flag)	//任务已经下发  进行超时判断
			{
				if(temp->ticker <= 0)	//删除该任务
				{
//					printf("dele seek amm task\n");
					if(_seek_amm_task_queue.frist == temp)	//删除第一个
					{
						_seek_amm_task_queue.frist = _seek_amm_task_queue.frist->next;
						if(NULL == _seek_amm_task_queue.frist)
							_seek_amm_task_queue.last = NULL;
						free(temp);
						temp = _seek_amm_task_queue.frist;
						temp_temp = temp;
					}
					else									//删除非第一个
					{
						temp_temp->next = temp->next;
						free(temp);
						if(NULL == temp_temp->next)
							_seek_amm_task_queue.last = temp_temp;
						temp = temp_temp->next;
					}
				}
				else
				{
					temp->ticker--;
					temp_temp = temp;
					temp = temp_temp->next;
				}
			}
			else					//任务没有下发 则下发任务
			{
				if(temp->ticker_ticker <= 0)
				{
					add_exec_seek(temp->ter);
					temp->flag = 0x66;
					temp->ticker = SEEK_AMM_TICKER;
				}
				else
				{
					temp->ticker_ticker--;
				}

				temp_temp = temp;
				temp = temp_temp->next;
			}
		}
		pthread_mutex_unlock(&_seek_amm_task_queue.mutex);

		while(0 < get_exec_seek_num())	//执行搜表
		{
//					printf("send seek amm task\n");
			tpFrame376_1 outbuf;
			tp3761Buffer snbuf;
			TerSocket *p;
			int ret = 0;
			get_frist_exec_seek(temp_ter);
			creat_afn5_98(temp_ter, 1, &outbuf);

			//将3761格式数据转换为可发送二进制数据
			DL3761_Protocol_LinkPack(&outbuf, &snbuf);
			//差找对应的套接字
			pthread_mutex_lock(&(route_mutex));
			p = AccordTerSeek(temp_ter);
			if(p != NULL)
			{
				pthread_mutex_lock(&(p->write_mutex));

//				int i = 0;
//						printf("**********\n");
//						for(i = 0; i < snbuf.Len; i++)
//							printf(" %02x",snbuf.Data[i]);
//						printf("\n");

				while(1)
				{
					ret = write(p->s, snbuf.Data, snbuf.Len);
					if(ret < 0)
					{
						if(errno == SIGPIPE)	//等待回收
						{
							p->ticker = 0;
						}
						break;
					}
					snbuf.Len -= ret;
					if(0 == snbuf.Len)
					{
						break;
					}
				}
				pthread_mutex_unlock(&(p->write_mutex));
			}

			pthread_mutex_unlock(&(route_mutex));
		}

	}

	pthread_exit(NULL);
}
