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

/**********************************台账部分************************************/
/*
 * 函数功能:初始化台账节点数量
 * */
void StandNodeNumInit(void)
{
	_StandNodeNum = 0;
}

/*
 * 函数功能:台账文件中的位置全部空余
 * */
void StandFileAllSurplus(void)
{
	memset(_Surplus, 0, AMM_MAX_NUM);
}

/*
 * 函数功能:将台账文件的某位置标记为空余
 * 参数:	index  需要标记为空的位置
 * */
void SetStandFleSurplus(unsigned short index)
{
	_Surplus[index] = 0;
}

/*
 * 函数功能:将台账文件的某位置标记为非空余
 * 参数:	index  需要标记为非空的位置
 * */
void SetStandFleNoSurplus(unsigned short index)
{
	_Surplus[index] = 1;
}

/*
 * 函数功能:获取距离文件开头最近的空余位置
 * 返回值:-1 没有空余位置  >=0最近的空余位置索引
 * */
int	GetNearestSurplus(void)
{
	int i = 0;
	for(i = 0; i < AMM_MAX_NUM; i++)
	{
		if(0 == _Surplus[i])
			return i;
	}
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
	int high = _StandNodeNum - 1;
	int mid = 0;
	int ret = 0;
	if(0 == _StandNodeNum)	//没有节点
		return -1;

	pthread_mutex_lock(&StandMutex);
	while(low <= high)
	{
		mid = (low + high) / 2;
		ret = CompareUcharArray(addr, _SortNode[mid]->Amm, len);
		if(1 == ret)
		{
			pthread_mutex_unlock(&StandMutex);
			return mid;
		}
		if(-1 == ret)
			high = mid - 1;
		if(0 == ret)
			low = mid + 1;
	}
	pthread_mutex_unlock(&StandMutex);
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
	StandNode *p = NULL;
	int ret = 0;
	int low = 0;
	int high = _StandNodeNum - 1;
	int mid = 0;
	StandNode *temp[AMM_MAX_NUM] = {0};

	if(node->num >= AMM_MAX_NUM)
		return -1;
	pthread_mutex_lock(&StandMutex);
	//本次添加为第一次添加
	if(0 == _StandNodeNum)
	{
		p = (StandNode *)malloc(sizeof(StandNode));
		if(NULL == p)
		{
			pthread_mutex_unlock(&StandMutex);
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
		_SortNode[0] = p;
		_StandNodeNum = 1;
		//记录台账文件空余
		SetStandFleNoSurplus(p->num);
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
	pthread_mutex_unlock(&StandMutex);
	//添加时，检查有没有相同的电表
	ret = SeekAmmAddr(node->Amm, AMM_ADDR_LEN);
	pthread_mutex_lock(&StandMutex);
	if(-1 == ret)	//没有相同的电表
	{
		if(AMM_MAX_NUM == _StandNodeNum)	//台账已满
		{
			pthread_mutex_unlock(&StandMutex);
			return -1;
		}
		//计算电表插入的位置，升序
		while(low <= high)
		{
			mid = (low + high) / 2;
			//比较台账中间的电表
			ret = CompareUcharArray(node->Amm, _SortNode[mid]->Amm, AMM_ADDR_LEN);
			if(ret == 0)	//待添加电表号 大于 中间表号
			{
				if((mid + 1) == _StandNodeNum)	//台账尾
				{
					mid++;
					break;
				}
				else
				{
					ret = CompareUcharArray(node->Amm, _SortNode[mid + 1]->Amm, AMM_ADDR_LEN);
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
			pthread_mutex_unlock(&StandMutex);
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
		memcpy(temp, _SortNode, sizeof(StandNode *) * AMM_MAX_NUM);
		_SortNode[mid] = p;
		memcpy((_SortNode + mid + 1), (temp + mid), sizeof(StandNode *)*(_StandNodeNum - mid));
		_StandNodeNum += 1;
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
	else	//有相同的电表
	{
		_SortNode[ret]->type = node->type;
		memcpy(_SortNode[ret]->Ter, node->Ter, TER_ADDR_LEN);
		_SortNode[ret]->cyFlag = node->cyFlag;
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
}

/*
 * 函数功能:删除台账文件中的电表
 * 参数:	fd	文件描述符
 * 		l 	该电表在文件中的位置
 * 返回值 0 成功 -1 失败
 * */
int	DeleNodeStandFile(int fd ,int l)
{
	AmmAttribute amm;
	int offset = 0;
	int ret = 0;
	memset(&amm, 0xFF, sizeof(AmmAttribute));
	offset = l * sizeof(AmmAttribute);
	ret = WriteFile(fd, offset, &amm, sizeof(AmmAttribute));
	if(0 == ret)
		SetStandFleSurplus(l);	//标记文件中该位置为空
	return ret;
}

/*
 *函数功能:删除内存台账中的某只电表，同时删除文件中的电表
 *参数:	fd	 文件描述符
 *		addr 电表地址
 *返回值: 0 成功 -1 失败
 * */
int DeleNodeStand(int fd, unsigned char *addr)
{
	int index = -1;
	int ret = -1;
	StandNode *temp[AMM_MAX_NUM] = {0};

	index = SeekAmmAddr(addr, AMM_ADDR_LEN);
	if(-1 == index)
		return -1;

	//删除文件中的电表
	pthread_mutex_lock(&StandMutex);
	ret = DeleNodeStandFile(fd, _SortNode[index]->num);
	if(-1 == ret)
		printf("dele stand file amm erro\n");
	//释放内存
	if(_SortNode[index] != NULL)
	{
		free((void *)_SortNode[index]);
		_SortNode[index] = NULL;
	}
	//排序
	memcpy(temp, _SortNode, sizeof(StandNode *) * AMM_MAX_NUM);
	memcpy(_SortNode, temp, sizeof(StandNode *) * index);
	memcpy((_SortNode + index), (temp + index + 1), sizeof(StandNode *) * (_StandNodeNum - 1 - index));
	_StandNodeNum -= 1;
	pthread_mutex_unlock(&StandMutex);
	return 0;
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
		return -1;
	pthread_mutex_lock(&StandMutex);
	memcpy(_SortNode[index]->Ter, ter, TER_ADDR_LEN);
	pthread_mutex_unlock(&StandMutex);
	return 0;
}

/***************************************************************************/
/*
 * 函数功能:根据内存中的节点内容修改台账文件中的节点
 * 参数: fd 	 文件描述符
 * 		node 节点
 * 返回值 0 成功 -1 失败
 * */
int AlterNodeStandFile(int fd, StandNode *node)
{
	AmmAttribute amm;
	int ret = 0;
	int offset = 0;
	memset(&amm, 0, sizeof(AmmAttribute));

	memcpy(amm.Amm, node->Amm, AMM_ADDR_LEN);
	amm.type = node->type;
	amm.cyFlag = node->cyFlag;
	memcpy(amm.Ter, node->Ter, TER_ADDR_LEN);

	ret = offsetof(AmmAttribute, CS);
	amm.CS = Func_CS(&amm, ret);

	offset = node->num * sizeof(AmmAttribute);
	ret = WriteFile(fd, offset, &amm, sizeof(AmmAttribute));
	return ret;
}

/*
 * 函数功能:获取台账中相应电表内容
 * 参数	index	索引
 * 		node 	输出台账节点
 * 返回值 0成功 -1失败
 * */
int GetStandNode(int index, StandNode *node)
{
	if(index >= _StandNodeNum)
	{
		return -1;
	}

	memset(node, 0, sizeof(StandNode));
	pthread_mutex_lock(&StandMutex);
	memcpy(node, _SortNode[index], sizeof(StandNode));
	pthread_mutex_unlock(&StandMutex);

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
	if(index >= _StandNodeNum)
	{
		return -1;
	}
	pthread_mutex_lock(&StandMutex);
	memcpy(_SortNode[index], node, sizeof(StandNode));
	pthread_mutex_unlock(&StandMutex);
	return 0;
}
