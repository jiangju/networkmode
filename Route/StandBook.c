/*
 * StandBook.c
 *
 *  Created on: 2016��7��1��
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

/**********************************̨�˲���************************************/
/*
 * ��������:��ʼ��̨�˽ڵ�����
 * */
void StandNodeNumInit(void)
{
	_StandNodeNum = 0;
}

/*
 * ��������:̨���ļ��е�λ��ȫ������
 * */
void StandFileAllSurplus(void)
{
	memset(_Surplus, 0, AMM_MAX_NUM);
}

/*
 * ��������:��̨���ļ���ĳλ�ñ��Ϊ����
 * ����:	index  ��Ҫ���Ϊ�յ�λ��
 * */
void SetStandFleSurplus(unsigned short index)
{
	_Surplus[index] = 0;
}

/*
 * ��������:��̨���ļ���ĳλ�ñ��Ϊ�ǿ���
 * ����:	index  ��Ҫ���Ϊ�ǿյ�λ��
 * */
void SetStandFleNoSurplus(unsigned short index)
{
	_Surplus[index] = 1;
}

/*
 * ��������:��ȡ�����ļ���ͷ����Ŀ���λ��
 * ����ֵ:-1 û�п���λ��  >=0����Ŀ���λ������
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
 * ��������:�鿴̨����������û����ͬ�ĵ��
 * ����:	addr	���ַ
 * 		len		���ַ����
 * ����ֵ: -1 û����ͬ��� >=0 ��ͬ����������е����
 * */
int SeekAmmAddr(unsigned char *addr, unsigned char len)
{
	int low = 0;
	int high = _StandNodeNum - 1;
	int mid = 0;
	int ret = 0;
	if(0 == _StandNodeNum)	//û�нڵ�
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
 *��������:���ڴ������/�޸�̨����Ϣ
 *����:	addr	����ַ
 *		type	��Լ����
 *		flag    �����־
 *����ֵ -1 ʧ��  0 �ɹ�
 *ס:�����ʱ,����̨��������ͬ�ĵ���򲻸���̨���ļ��Ĵ洢�����������ֻ�޸��ڴ��е�̨����Ϣ
 *���޸��ļ��е�̨����Ϣ,�����޸�̨���е���Ϣ��ʹ��AlterNodeStandFile����
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
	//�������Ϊ��һ�����
	if(0 == _StandNodeNum)
	{
		p = (StandNode *)malloc(sizeof(StandNode));
		if(NULL == p)
		{
			pthread_mutex_unlock(&StandMutex);
			return -1;
		}
		//���ַ
		memcpy(p->Amm, node->Amm, AMM_ADDR_LEN);
		//��Լ����
		p->type = node->type;
		//��Ӧ���ն˵�ַ
		memcpy(p->Ter, node->Ter, TER_ADDR_LEN);
		//����ɹ���־
		p->cyFlag = node->cyFlag;
		//���
		p->num = node->num;
		//����
		_SortNode[0] = p;
		_StandNodeNum = 1;
		//��¼̨���ļ�����
		SetStandFleNoSurplus(p->num);
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
	pthread_mutex_unlock(&StandMutex);
	//���ʱ�������û����ͬ�ĵ��
	ret = SeekAmmAddr(node->Amm, AMM_ADDR_LEN);
	pthread_mutex_lock(&StandMutex);
	if(-1 == ret)	//û����ͬ�ĵ��
	{
		if(AMM_MAX_NUM == _StandNodeNum)	//̨������
		{
			pthread_mutex_unlock(&StandMutex);
			return -1;
		}
		//����������λ�ã�����
		while(low <= high)
		{
			mid = (low + high) / 2;
			//�Ƚ�̨���м�ĵ��
			ret = CompareUcharArray(node->Amm, _SortNode[mid]->Amm, AMM_ADDR_LEN);
			if(ret == 0)	//����ӵ��� ���� �м���
			{
				if((mid + 1) == _StandNodeNum)	//̨��β
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
			else	//����ӱ��С���м���
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
		//���ַ
		memcpy(p->Amm, node->Amm, AMM_ADDR_LEN);
		//��Լ����
		p->type = node->type;
		//��Ӧ���ն˵�ַ
		memcpy(p->Ter, node->Ter, TER_ADDR_LEN);
		//����ɹ���־
		p->cyFlag = node->cyFlag;
		//���
		p->num = node->num;
		//��¼̨���ļ�����
		SetStandFleNoSurplus(p->num);
		//����������
		memcpy(temp, _SortNode, sizeof(StandNode *) * AMM_MAX_NUM);
		_SortNode[mid] = p;
		memcpy((_SortNode + mid + 1), (temp + mid), sizeof(StandNode *)*(_StandNodeNum - mid));
		_StandNodeNum += 1;
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
	else	//����ͬ�ĵ��
	{
		_SortNode[ret]->type = node->type;
		memcpy(_SortNode[ret]->Ter, node->Ter, TER_ADDR_LEN);
		_SortNode[ret]->cyFlag = node->cyFlag;
		pthread_mutex_unlock(&StandMutex);
		return 0;
	}
}

/*
 * ��������:ɾ��̨���ļ��еĵ��
 * ����:	fd	�ļ�������
 * 		l 	�õ�����ļ��е�λ��
 * ����ֵ 0 �ɹ� -1 ʧ��
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
		SetStandFleSurplus(l);	//����ļ��и�λ��Ϊ��
	return ret;
}

/*
 *��������:ɾ���ڴ�̨���е�ĳֻ���ͬʱɾ���ļ��еĵ��
 *����:	fd	 �ļ�������
 *		addr ����ַ
 *����ֵ: 0 �ɹ� -1 ʧ��
 * */
int DeleNodeStand(int fd, unsigned char *addr)
{
	int index = -1;
	int ret = -1;
	StandNode *temp[AMM_MAX_NUM] = {0};

	index = SeekAmmAddr(addr, AMM_ADDR_LEN);
	if(-1 == index)
		return -1;

	//ɾ���ļ��еĵ��
	pthread_mutex_lock(&StandMutex);
	ret = DeleNodeStandFile(fd, _SortNode[index]->num);
	if(-1 == ret)
		printf("dele stand file amm erro\n");
	//�ͷ��ڴ�
	if(_SortNode[index] != NULL)
	{
		free((void *)_SortNode[index]);
		_SortNode[index] = NULL;
	}
	//����
	memcpy(temp, _SortNode, sizeof(StandNode *) * AMM_MAX_NUM);
	memcpy(_SortNode, temp, sizeof(StandNode *) * index);
	memcpy((_SortNode + index), (temp + index + 1), sizeof(StandNode *) * (_StandNodeNum - 1 - index));
	_StandNodeNum -= 1;
	pthread_mutex_unlock(&StandMutex);
	return 0;
}

/**********************************���·��**********************************/
/*
 * ��������:���ն˵�ַ�洢��̨�˶�Ӧ�ĵ����(�ڴ��е�̨��)
 * ����:	amm ����ַ
 * 		ter	�ն˵�ַ
 * ����ֵ	0 �ɹ� 1 ʧ��
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
 * ��������:�����ڴ��еĽڵ������޸�̨���ļ��еĽڵ�
 * ����: fd 	 �ļ�������
 * 		node �ڵ�
 * ����ֵ 0 �ɹ� -1 ʧ��
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
 * ��������:��ȡ̨������Ӧ�������
 * ����	index	����
 * 		node 	���̨�˽ڵ�
 * ����ֵ 0�ɹ� -1ʧ��
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
 * ��������:����̨�˽ڵ����ݣ��ڴ��У�
 * ����:	index	����
 * 		node 	��������
 * ����ֵ: 0 �ɹ� -1 ʧ��
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
