/*
 * StandBook.h
 *
 *  Created on: 2016年7月1日
 *      Author: j
 */

#ifndef ROUTE_STANDBOOK_H_
#define ROUTE_STANDBOOK_H_
#include "SysPara.h"
#include <pthread.h>

typedef struct stand_node
{
	unsigned char 	Amm[AMM_ADDR_LEN];	//电表号
	unsigned char	type;				//通讯规约类型
	unsigned char 	Ter[TER_ADDR_LEN];	//终端地址
	unsigned short 	num;				//存储序号
	unsigned char 	cyFlag;				//抄表标志  (当等于运行参数中的抄表成功的标志时，抄表成功)
}StandNode;	//台账节点信息

#ifdef	_STAND_BOOK_C_

//台账信息
int _StandNodeNum;					//内存中台账节点数量
StandNode *_SortNode[AMM_MAX_NUM];	//存放排序后的节点地址（方便对台账链表的遍历和添加删除）
pthread_mutex_t StandMutex;			//台账锁

unsigned char _Surplus[AMM_MAX_NUM];	//剩余位置 相应索引下的值为0  代表空余

#endif

void StandNodeNumInit(void);
void StandFileAllSurplus(void);
void SetStandFleSurplus(unsigned short index);
void SetStandFleNoSurplus(unsigned short index);
int	GetNearestSurplus(void);
int SeekAmmAddr(unsigned char *addr, unsigned char len);
int AddNodeStand(StandNode *node);
int	DeleNodeStandFile(int fd ,int l);
int DeleNodeStand(int fd, unsigned char *addr);
int AlterNodeStandFile(int fd, StandNode *node);
int GetStandNode(int index, StandNode *node);
int UpdateStandNode(int index, StandNode *node);

#ifndef	_STAND_BOOK_C_

extern int _StandNodeNum;					//内存中台账节点数量
extern StandNode *_SortNode[AMM_MAX_NUM];	//存放排序后的节点地址（方便对台账链表的遍历和添加删除）
extern pthread_mutex_t StandMutex;			//台账锁
extern unsigned char _Surplus[AMM_MAX_NUM];	//剩余位置 相应索引下的值为0  代表空余

#endif

#endif /* ROUTE_STANDBOOK_H_ */
