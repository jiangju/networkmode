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
	unsigned char 	last_t[TIME_FRA_LEN];	//���һ��ͨ��ʱ�� �����������ò�Ҫ���������ṹ�����Ϊ�ļ��в���Ҫʱ�����ݣ�
											//ֻ���ڴ�����Ҫʱ�����ݡ���ý��ļ��д洢�����ݸ�ʽ���ڴ��д洢�����ݸ�ʽʹ�ù�����ϵ����Ҫ
											//���ڴ��е����ݸ�ʽ���ļ��е����ݸ�ʽ����ͬһ����ʽ��������޸ģ�������
}StandNode;	//̨�˽ڵ���Ϣ

#ifdef	_STAND_BOOK_C_

//̨����Ϣ
int _StandNodeNum;					//�ڴ���̨�˽ڵ�����
StandNode *_SortNode[AMM_MAX_NUM];	//��������Ľڵ��ַ�������̨������ı��������ɾ����
pthread_mutex_t StandMutex;			//̨����

unsigned char _Surplus[AMM_MAX_NUM];	//ʣ��λ�� ��Ӧ�����µ�ֵΪ0  �������

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
unsigned char StatTerAmmNum(unsigned char *ter);
int UpdateNodeDate(int index);
int SeekTerOfAmm(unsigned char *ter, unsigned char num);

#ifndef	_STAND_BOOK_C_

extern int _StandNodeNum;					//�ڴ���̨�˽ڵ�����
extern StandNode *_SortNode[AMM_MAX_NUM];	//��������Ľڵ��ַ�������̨������ı��������ɾ����
extern pthread_mutex_t StandMutex;			//̨����
extern unsigned char _Surplus[AMM_MAX_NUM];	//ʣ��λ�� ��Ӧ�����µ�ֵΪ0  �������

#endif

#endif /* ROUTE_STANDBOOK_H_ */
