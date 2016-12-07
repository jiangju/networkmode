/*
 * StandBook.c
 *
 *  Created on: 2016年7月1日
 *      Author: j
 */
#define _STAND_BOOK_C_
#include "StandBook.h"
#undef	_STAND_BOOK_C_
#include "DL376_2_DataType.h"
#include <string.h>
#include "CommLib.h"
#include "stddef.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/*
 * 函数功能:初始化台账变量
 * */
void HldStandInit(void)
{
	pthread_mutex_init(&_hld_stand.f.mutex, NULL);
	strcpy(_hld_stand.f.filename, STAND_BOOK_FILE);
	memset(_hld_stand.f.surplus, 0, AMM_MAX_NUM);

	pthread_mutex_init(&_hld_stand.m.mutex, NULL);
	memset(_hld_stand.m.stort, 0, sizeof(StandNode *) * AMM_MAX_NUM);
	_hld_stand.m.num = 0;
}

/*
 * 函数功能:初始化整个内存中的台账
 * */
void StandMemInit(void)
{
	int i = 0;
	pthread_mutex_lock(&_hld_stand.m.mutex);
	for(i = 0; i < _hld_stand.m.num; i++)
	{
		if(_hld_stand.m.stort[i] != NULL)
			free(_hld_stand.m.stort[i]);
		_hld_stand.m.stort[i] = NULL;
	}
	_hld_stand.m.num = 0;
	pthread_mutex_unlock(&_hld_stand.m.mutex);
}

/*
 * 函数功能:初始化整个台账文件
 * */
int StandFileInit(void)
{
	AmmAttribute amm;
	int offset = 0;
	int ret = 0;
	int fd = -1;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//打开文件
	while(i--)
	{
		fd = open(_hld_stand.f.filename, O_RDWR);
		if(fd >=0 )
			break;
	}

	if(fd < 0)
	{
		pthread_mutex_unlock(&_hld_stand.f.mutex);
		return -1;
	}

	memset(&amm, 0xFF, sizeof(AmmAttribute));
	lseek(fd, 0, SEEK_SET);
	for(i = 0; i < AMM_MAX_NUM; i++)
	{
		write(fd, &amm, sizeof(AmmAttribute));
	}
	close(fd);
	memset(_hld_stand.f.surplus, 0, AMM_MAX_NUM);
	pthread_mutex_unlock(&_hld_stand.f.mutex);

	return 0;
}

/**********************************台账部分************************************/
/*
 * 函数功能:初始化台账节点数量
 * */
void StandNodeNumInit(void)
{
	pthread_mutex_lock(&_hld_stand.m.mutex);
	_hld_stand.m.num = 0;
	pthread_mutex_unlock(&_hld_stand.m.mutex);
}

int GetStandNodeNum(void)
{
	int ret = 0;
	pthread_mutex_lock(&_hld_stand.m.mutex);
	ret = _hld_stand.m.num;
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return ret;
}

/*
 * 函数功能:台账文件中的位置全部空余
 * */
void StandFileAllSurplus(void)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	memset(_hld_stand.f.surplus, 0, AMM_MAX_NUM);
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * 函数功能:将台账文件的某位置标记为空余
 * 参数:	index  需要标记为空的位置
 * */
void SetStandFleSurplus(unsigned short index)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	_hld_stand.f.surplus[index] = 0;
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * 函数功能:将台账文件的某位置标记为非空余
 * 参数:	index  需要标记为非空的位置
 * */
void SetStandFleNoSurplus(unsigned short index)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	_hld_stand.f.surplus[index] = 1;
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * 函数功能:获取距离文件开头最近的空余位置
 * 返回值:-1 没有空余位置  >=0最近的空余位置索引
 * */
int	GetNearestSurplus(void)
{
	int i = 0;
	pthread_mutex_lock(&_hld_stand.f.mutex);
	for(i = 0; i < AMM_MAX_NUM; i++)
	{
		if(0 == _hld_stand.f.surplus[i])
		{
			pthread_mutex_unlock(&_hld_stand.f.mutex);
			return i;
		}
	}
	pthread_mutex_unlock(&_hld_stand.f.mutex);
	return -1;
}

/*
 * 函数功能:查看台账链表中有没有相同的电表
 * 参数:	addr	表地址
 * 		len		表地址长度
 * 返回值: -1 没有相同表号 >=0 相同标号在链表中的序号
 * */
int SeekAmmAddr(unsigned char *addr, unsigned char len)
{
	int low = 0;
	int mid = 0;
	int ret = 0;

	pthread_mutex_lock(&_hld_stand.m.mutex);
	int high = _hld_stand.m.num - 1;
	if(0 == _hld_stand.m.num)	//没有节点
	{
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return -1;
	}

	while(low <= high)
	{
		mid = (low + high) / 2;
		ret = CompareUcharArray(addr, _hld_stand.m.stort[mid]->Amm, len);
		if(1 == ret)
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
			return mid;
		}
		if(-1 == ret)
			high = mid - 1;
		if(0 == ret)
			low = mid + 1;
	}
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return -1;
}

/*
 *函数功能:向内存中添加/修改台账信息
 *参数:	addr	电表地址
 *		type	规约类型
 *		flag    抄表标志
 *返回值 -1 失败  0 成功
 *住:当添加时,发现台账中有相同的电表则不更新台账文件的存储情况，本函数只修改内存中的台账信息
 *不修改文件中的台账信息,如需修改台账中的信息则使用AlterNodeStandFile函数
 * */
int AddNodeStand(StandNode *node)
{
	if(node->num >= AMM_MAX_NUM)
			return -1;

	pthread_mutex_lock(&_hld_stand.m.mutex);
	StandNode *p = NULL;
	int ret = 0;
	int low = 0;
	int high = _hld_stand.m.num - 1;
	int mid = 0;
	StandNode *temp[AMM_MAX_NUM] = {0};

	//本次添加为第一次添加
	if(0 == _hld_stand.m.num)
	{
		p = (StandNode *)malloc(sizeof(StandNode));
		if(NULL == p)
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
			return -1;
		}
		//表地址
		memcpy(p->Amm, node->Amm, AMM_ADDR_LEN);
		//规约类型
		p->type = node->type;
		//对应的终端地址
		memcpy(p->Ter, node->Ter, TER_ADDR_LEN);
		//抄表成功标志
		p->cyFlag = node->cyFlag;
		//序号
		p->num = node->num;
		//排序
		_hld_stand.m.stort[0] = p;
		_hld_stand.m.num = 1;
		//记录台账文件空余
		SetStandFleNoSurplus(p->num);
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	//添加时，检查有没有相同的电表
	ret = SeekAmmAddr(node->Amm, AMM_ADDR_LEN);
	pthread_mutex_lock(&_hld_stand.m.mutex);
	if(-1 == ret)	//没有相同的电表
	{
		if(AMM_MAX_NUM == _hld_stand.m.num)	//台账已满
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
			return -1;
		}
		//计算电表插入的位置，升序
		while(low <= high)
		{
			mid = (low + high) / 2;
			//比较台账中间的电表
			ret = CompareUcharArray(node->Amm, _hld_stand.m.stort[mid]->Amm, AMM_ADDR_LEN);
			if(ret == 0)	//待添加电表号 大于 中间表号
			{
				if((mid + 1) == _hld_stand.m.num)	//台账尾
				{
					mid++;
					break;
				}
				else
				{
					ret = CompareUcharArray(node->Amm, _hld_stand.m.stort[mid + 1]->Amm, AMM_ADDR_LEN);
					if(ret == -1)
					{
						mid++;
						break;
					}
					else
					{
						low = mid + 1;
					}
				}
			}
			else	//待添加表号小于中间表号
			{
				high = mid - 1;
			}
		}

		p = (StandNode *)malloc(sizeof(StandNode));
		if(NULL == p)
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
			return -1;
		}
		//表地址
		memcpy(p->Amm, node->Amm, AMM_ADDR_LEN);
		//规约类型
		p->type = node->type;
		//对应的终端地址
		memcpy(p->Ter, node->Ter, TER_ADDR_LEN);
		//抄表成功标志
		p->cyFlag = node->cyFlag;
		//序号
		p->num = node->num;

		//记录台账文件空余
		SetStandFleNoSurplus(p->num);

		//插入且排序
		memcpy(temp, _hld_stand.m.stort, sizeof(StandNode *) * AMM_MAX_NUM);
		_hld_stand.m.stort[mid] = p;
		memcpy((_hld_stand.m.stort + mid + 1), (temp + mid), sizeof(StandNode *)*(_hld_stand.m.num - mid));
		_hld_stand.m.num += 1;
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
	else	//有相同的电表
	{
		_hld_stand.m.stort[ret]->type = node->type;
		memcpy(_hld_stand.m.stort[ret]->Ter, node->Ter, TER_ADDR_LEN);
		_hld_stand.m.stort[ret]->cyFlag = node->cyFlag;
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
}

/*
 * 函数功能:删除台账文件中的电表
 * 参数:
 * 		l 	该电表在文件中的位置
 * 返回值 0 成功 -1 失败
 * */
int	DeleNodeStandFile(int l)
{
	AmmAttribute amm;
	int offset = 0;
	int ret = 0;
	int fd = -1;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//打开文件
	while(i--)
	{
		fd = open(_hld_stand.f.filename, O_RDWR);
		if(fd >=0 )
			break;
	}

	if(fd < 0)
	{
		pthread_mutex_unlock(&_hld_stand.f.mutex);
		return -1;
	}

	memset(&amm, 0xFF, sizeof(AmmAttribute));
	offset = l * sizeof(AmmAttribute);
	ret = WriteFile(fd, offset, &amm, sizeof(AmmAttribute));
	if(0 == ret)
		_hld_stand.f.surplus[l] = 0; //标记文件中该位置为空
	close(fd);
	pthread_mutex_unlock(&_hld_stand.f.mutex);
	return ret;
}

/*
 *函数功能:删除内存台账中的某只电表，同时删除文件中的电表
 *参数:	fd	 文件描述符
 *		addr 电表地址
 *返回值: 0 成功 -1 失败
 * */
int DeleNodeStand(unsigned char *addr)
{
	int index = -1;
	int ret = -1;
	int f_index = 0;
	StandNode *temp[AMM_MAX_NUM] = {0};

	index = SeekAmmAddr(addr, AMM_ADDR_LEN);
	if(-1 == index)
		return -1;

	//先删除内存 后删除文件
	pthread_mutex_lock(&_hld_stand.m.mutex);

	f_index = _hld_stand.m.stort[index]->num;	//获取存储序号
	//释放内存
	if(_hld_stand.m.stort[index] != NULL)
	{
		free((void *)_hld_stand.m.stort[index]);
		_hld_stand.m.stort[index] = NULL;
	}
	//排序
	memcpy(temp, _hld_stand.m.stort, sizeof(StandNode *) * AMM_MAX_NUM);
	memcpy(_hld_stand.m.stort, temp, sizeof(StandNode *) * index);
	memcpy((_hld_stand.m.stort + index), (temp + index + 1), sizeof(StandNode *) * (_hld_stand.m.num - 1 - index));
	_hld_stand.m.num -= 1;

	pthread_mutex_unlock(&_hld_stand.m.mutex);

	//删除文件中的电表
	ret = DeleNodeStandFile(f_index);
	return ret;
}

/*
 * 函数功能:统计终端下的电表个数
 * 参数:	ter	终端地址
 * 返回值:电表个数
 * */
unsigned char StatTerAmmNum(unsigned char *ter)
{
	unsigned char i = 0;
	int index = 0;
	pthread_mutex_lock(&_hld_stand.m.mutex);
	for(index = 0; index < _hld_stand.m.num; index++)
	{
		if(1 == CompareUcharArray(_hld_stand.m.stort[index]->Ter, ter, TER_ADDR_LEN))
		{
			i++;
		}
	}
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return i;
}

/*
 * 函数功能:查询终端下的相应电表
 * 参数:	ter		终端地址
 * 		num		电表序号
 * 返回值:	-1失败  >0 成功
 * */
int SeekTerOfAmm(unsigned char *ter, unsigned char num)
{
	int i = 0;
	int index = 0;
	if(num == 0)
		num = 1;
	pthread_mutex_lock(&_hld_stand.m.mutex);
	for(index = 0; index < _hld_stand.m.num; index++)
	{
		if(1 == CompareUcharArray(_hld_stand.m.stort[index]->Ter, ter, TER_ADDR_LEN))
		{
			if(i == (num - 1))
				break;
			i++;
		}
	}
	if(i == _hld_stand.m.num)
		i = -1;
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return i;
}

/**********************************电表路径**********************************/
/*
 * 函数功能:将终端地址存储到台账对应的电表下(内存中的台账)
 * 参数:	amm 电表地址
 * 		ter	终端地址
 * 返回值	0 成功 1 失败
 * */
int AlterRouteStand(unsigned char *amm, unsigned char *ter)
{
	int index = 0;
	index = SeekAmmAddr(amm, AMM_ADDR_LEN);
	if(-1 == index)
	{
		return -1;
	}
	pthread_mutex_lock(&_hld_stand.m.mutex);
	memcpy(_hld_stand.m.stort[index]->Ter, ter, TER_ADDR_LEN);
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return 0;
}

/***************************************************************************/
/*
 * 函数功能:根据内存中的节点内容修改台账文件中的节点
 * 参数: fd 	 文件描述符
 * 		node 节点
 * 返回值 0 成功 -1 失败
 * */
int AlterNodeStandFile(StandNode *node)
{
	AmmAttribute amm;
	int ret = 0;
	int offset = 0;
	int fd = 0;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//打开文件
	while(i--)
	{
		fd = open(STAND_BOOK_FILE, O_RDWR);
		if(fd >=0 )
			break;
	}

	if(fd < 0)
	{
		pthread_mutex_unlock(&_hld_stand.f.mutex);
		return -1;
	}
	memset(&amm, 0, sizeof(AmmAttribute));

	memcpy(amm.Amm, node->Amm, AMM_ADDR_LEN);
	amm.type = node->type;
	amm.cyFlag = node->cyFlag;
	memcpy(amm.Ter, node->Ter, TER_ADDR_LEN);

	ret = offsetof(AmmAttribute, CS);
	amm.CS = Func_CS(&amm, ret);

	offset = node->num * sizeof(AmmAttribute);
	ret = WriteFile(fd, offset, &amm, sizeof(AmmAttribute));
	close(fd);
	pthread_mutex_unlock(&_hld_stand.f.mutex);
	return ret;
}

/*
 * 函数功能:更新某电表最后通信时间
 * 参数:		index 	电表在台账中的索引
 * 返回值:	0 成功 -1失败
 * */
int UpdateNodeDate(int index)
{
	if(index >= _hld_stand.m.num || index < 0)
	{
		return -1;
	}

	time_t timer;
	struct tm* t_tm;
	time(&timer);
	t_tm = localtime(&timer);

	pthread_mutex_lock(&_hld_stand.m.mutex);

	//年
	_hld_stand.m.stort[index]->last_t[0] = HexToBcd((t_tm->tm_year - 100));
	//月
	_hld_stand.m.stort[index]->last_t[1] = HexToBcd(t_tm->tm_mon + 1);
	//日
	_hld_stand.m.stort[index]->last_t[2] = HexToBcd(t_tm->tm_mday);
	//时
	_hld_stand.m.stort[index]->last_t[3] = HexToBcd(t_tm->tm_hour);
	//分
	_hld_stand.m.stort[index]->last_t[4] = HexToBcd(t_tm->tm_min);
	//秒
	_hld_stand.m.stort[index]->last_t[5] = HexToBcd(t_tm->tm_sec);

	pthread_mutex_unlock(&_hld_stand.m.mutex);

	return 0;
}

/*
 * 函数功能:获取台账中相应电表内容
 * 参数	index	索引
 * 		node 	输出台账节点
 * 返回值 0成功 -1失败
 * */
int GetStandNode(int index, StandNode *node)
{
	pthread_mutex_lock(&_hld_stand.m.mutex);
	if(index >= _hld_stand.m.num || index < 0)
	{
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return -1;
	}

	memset(node, 0, sizeof(StandNode));

	memcpy(node, _hld_stand.m.stort[index], sizeof(StandNode));
	pthread_mutex_unlock(&_hld_stand.m.mutex);

	return 0;
}

/*
 * 函数功能:更新台账节点内容（内存中）
 * 参数:	index	索引
 * 		node 	更新内容
 * 返回值: 0 成功 -1 失败
 * */
int UpdateStandNode(int index, StandNode *node)
{
	pthread_mutex_lock(&_hld_stand.m.mutex);
	if(index >= _hld_stand.m.num || index < 0)
	{
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return -1;
	}

	memcpy(_hld_stand.m.stort[index], node, sizeof(StandNode));
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	return 0;
}
