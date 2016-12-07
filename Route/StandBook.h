/*
 * StandBook.h
 *
 *  Created on: 2016��7��1��
 *      Author: j
 */

#ifndef ROUTE_STANDBOOK_H_
#define ROUTE_STANDBOOK_H_
#include "SysPara.h"
#include <pthread.h>

typedef struct stand_node
{
	unsigned char 	Amm[AMM_ADDR_LEN];		//����
	unsigned char	type;					//ͨѶ��Լ����
	unsigned char 	Ter[TER_ADDR_LEN];		//�ն˵�ַ
	unsigned short 	num;					//�洢���
	unsigned char 	cyFlag;					//�����־  (���������в����еĳ���ɹ��ı�־ʱ������ɹ�)
	unsigned char 	last_t[TIME_FRA_LEN];	//���һ��ͨ��ʱ��
}StandNode;	//̨�˽ڵ���Ϣ	����̨��  ��վ�·���̨վ

typedef struct mem_stand
{
	int num;							//�ڴ���̨������
	StandNode *stort[AMM_MAX_NUM];		//��������Ľڵ��ַ�������̨������ı��������ɾ����
	pthread_mutex_t mutex;				//�ڴ��е�̨����
}MemStand;								//�ڴ��е�̨��

typedef struct file_stand
{
	unsigned char surplus[AMM_MAX_NUM];		//w�ļ�ʣ��λ�� ��Ӧ�����µ�ֵΪ0  �������
	char filename[20];						//�ļ���
	pthread_mutex_t mutex;					//̨���ļ���
}FileStand;									//�ļ��е�̨��

typedef struct
{
	MemStand m;
	FileStand f;
}HldStand;

#ifdef	_STAND_BOOK_C_

//̨����Ϣ
//int _StandNodeNum;					//�ڴ���̨�˽ڵ�����
//StandNode *_SortNode[AMM_MAX_NUM];	//��������Ľڵ��ַ�������̨������ı��������ɾ����
//pthread_mutex_t StandMutex;			//̨����
//unsigned char _Surplus[AMM_MAX_NUM];	//ʣ��λ�� ��Ӧ�����µ�ֵΪ0  �������
//
//pthread_mutex_t StandFileMutex;		//̨���ļ���

HldStand _hld_stand;

#endif

void StandNodeNumInit(void);
void StandFileAllSurplus(void);
void SetStandFleSurplus(unsigned short index);
void SetStandFleNoSurplus(unsigned short index);
int	GetNearestSurplus(void);
int SeekAmmAddr(unsigned char *addr, unsigned char len);
int AddNodeStand(StandNode *node);
int	DeleNodeStandFile(int l);
int DeleNodeStand(unsigned char *addr);
int AlterNodeStandFile(StandNode *node);
int GetStandNode(int index, StandNode *node);
int UpdateStandNode(int index, StandNode *node);
unsigned char StatTerAmmNum(unsigned char *ter);
int UpdateNodeDate(int index);
int SeekTerOfAmm(unsigned char *ter, unsigned char num);
int StandFileInit(void);
void StandMemInit(void);
void HldStandInit(void);
int GetStandNodeNum(void);
#ifndef	_STAND_BOOK_C_

//extern int _StandNodeNum;					//�ڴ���̨�˽ڵ�����
//extern StandNode *_SortNode[AMM_MAX_NUM];	//��������Ľڵ��ַ�������̨������ı��������ɾ����
//extern pthread_mutex_t StandMutex;			//̨����
//extern unsigned char _Surplus[AMM_MAX_NUM];	//ʣ��λ�� ��Ӧ�����µ�ֵΪ0  �������
//
//extern pthread_mutex_t StandFileMutex;		//̨���ļ���

extern HldStand _hld_stand;

#endif

#endif /* ROUTE_STANDBOOK_H_ */
