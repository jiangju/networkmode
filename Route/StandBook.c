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
#include <sys/time.h>

/*
 * ��������:��ʼ��̨�˱���
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
 * ��������:��ʼ�������ڴ��е�̨��
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
 * ��������:��ʼ������̨���ļ�
 * */
int StandFileInit(void)
{
	AmmAttribute amm;
	int offset = 0;
	int ret = 0;
	int fd = -1;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//���ļ�
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

/**********************************̨�˲���************************************/
/*
 * ��������:��ʼ��̨�˽ڵ�����
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
 * ��������:̨���ļ��е�λ��ȫ������
 * */
void StandFileAllSurplus(void)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	memset(_hld_stand.f.surplus, 0, AMM_MAX_NUM);
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * ��������:��̨���ļ���ĳλ�ñ��Ϊ����
 * ����:	index  ��Ҫ���Ϊ�յ�λ��
 * */
void SetStandFleSurplus(unsigned short index)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	_hld_stand.f.surplus[index] = 0;
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * ��������:��̨���ļ���ĳλ�ñ��Ϊ�ǿ���
 * ����:	index  ��Ҫ���Ϊ�ǿյ�λ��
 * */
void SetStandFleNoSurplus(unsigned short index)
{
	pthread_mutex_lock(&_hld_stand.f.mutex);
	_hld_stand.f.surplus[index] = 1;
	pthread_mutex_unlock(&_hld_stand.f.mutex);
}

/*
 * ��������:��ȡ�����ļ���ͷ����Ŀ���λ��
 * ����ֵ:-1 û�п���λ��  >=0����Ŀ���λ������
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
 * ��������:�鿴̨����������û����ͬ�ĵ��
 * ����:	addr	���ַ
 * 		len		���ַ����
 * ����ֵ: -1 û����ͬ��� >=0 ��ͬ����������е����
 * */
int SeekAmmAddr(unsigned char *addr, unsigned char len)
{
	int low = 0;
	int mid = 0;
	int ret = 0;

	pthread_mutex_lock(&_hld_stand.m.mutex);
	int high = _hld_stand.m.num - 1;
	if(0 == _hld_stand.m.num)	//û�нڵ�
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
	if(node->num >= AMM_MAX_NUM)
			return -1;

	pthread_mutex_lock(&_hld_stand.m.mutex);
	StandNode *p = NULL;
	int ret = 0;
	int low = 0;
	int high = _hld_stand.m.num - 1;
	int mid = 0;
	StandNode *temp[AMM_MAX_NUM] = {0};

	//�������Ϊ��һ�����
	if(0 == _hld_stand.m.num)
	{
		p = (StandNode *)malloc(sizeof(StandNode));
		if(NULL == p)
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
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
		_hld_stand.m.stort[0] = p;
		_hld_stand.m.num = 1;
		//��¼̨���ļ�����
		SetStandFleNoSurplus(p->num);
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
	pthread_mutex_unlock(&_hld_stand.m.mutex);
	//���ʱ�������û����ͬ�ĵ��
	ret = SeekAmmAddr(node->Amm, AMM_ADDR_LEN);
	pthread_mutex_lock(&_hld_stand.m.mutex);
	if(-1 == ret)	//û����ͬ�ĵ��
	{
		if(AMM_MAX_NUM == _hld_stand.m.num)	//̨������
		{
			pthread_mutex_unlock(&_hld_stand.m.mutex);
			return -1;
		}
		//����������λ�ã�����
		while(low <= high)
		{
			mid = (low + high) / 2;
			//�Ƚ�̨���м�ĵ��
			ret = CompareUcharArray(node->Amm, _hld_stand.m.stort[mid]->Amm, AMM_ADDR_LEN);
			if(ret == 0)	//����ӵ��� ���� �м���
			{
				if((mid + 1) == _hld_stand.m.num)	//̨��β
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
			else	//����ӱ��С���м���
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
		memcpy(temp, _hld_stand.m.stort, sizeof(StandNode *) * AMM_MAX_NUM);
		_hld_stand.m.stort[mid] = p;
		memcpy((_hld_stand.m.stort + mid + 1), (temp + mid), sizeof(StandNode *)*(_hld_stand.m.num - mid));
		_hld_stand.m.num += 1;
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
	else	//����ͬ�ĵ��
	{
		_hld_stand.m.stort[ret]->type = node->type;
		memcpy(_hld_stand.m.stort[ret]->Ter, node->Ter, TER_ADDR_LEN);
		_hld_stand.m.stort[ret]->cyFlag = node->cyFlag;
		pthread_mutex_unlock(&_hld_stand.m.mutex);
		return 0;
	}
}

/*
 * ��������:ɾ��̨���ļ��еĵ��
 * ����:
 * 		l 	�õ�����ļ��е�λ��
 * ����ֵ 0 �ɹ� -1 ʧ��
 * */
int	DeleNodeStandFile(int l)
{
	AmmAttribute amm;
	int offset = 0;
	int ret = 0;
	int fd = -1;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//���ļ�
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
		_hld_stand.f.surplus[l] = 0; //����ļ��и�λ��Ϊ��
	close(fd);
	pthread_mutex_unlock(&_hld_stand.f.mutex);
	return ret;
}

/*
 *��������:ɾ���ڴ�̨���е�ĳֻ���ͬʱɾ���ļ��еĵ��
 *����:	fd	 �ļ�������
 *		addr ����ַ
 *����ֵ: 0 �ɹ� -1 ʧ��
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

	//��ɾ���ڴ� ��ɾ���ļ�
	pthread_mutex_lock(&_hld_stand.m.mutex);

	f_index = _hld_stand.m.stort[index]->num;	//��ȡ�洢���
	//�ͷ��ڴ�
	if(_hld_stand.m.stort[index] != NULL)
	{
		free((void *)_hld_stand.m.stort[index]);
		_hld_stand.m.stort[index] = NULL;
	}
	//����
	memcpy(temp, _hld_stand.m.stort, sizeof(StandNode *) * AMM_MAX_NUM);
	memcpy(_hld_stand.m.stort, temp, sizeof(StandNode *) * index);
	memcpy((_hld_stand.m.stort + index), (temp + index + 1), sizeof(StandNode *) * (_hld_stand.m.num - 1 - index));
	_hld_stand.m.num -= 1;

	pthread_mutex_unlock(&_hld_stand.m.mutex);

	//ɾ���ļ��еĵ��
	ret = DeleNodeStandFile(f_index);
	return ret;
}

/*
 * ��������:ͳ���ն��µĵ�����
 * ����:	ter	�ն˵�ַ
 * ����ֵ:������
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
 * ��������:��ѯ�ն��µ���Ӧ���
 * ����:	ter		�ն˵�ַ
 * 		num		������
 * ����ֵ:	-1ʧ��  >0 �ɹ�
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
 * ��������:�����ڴ��еĽڵ������޸�̨���ļ��еĽڵ�
 * ����: fd 	 �ļ�������
 * 		node �ڵ�
 * ����ֵ 0 �ɹ� -1 ʧ��
 * */
int AlterNodeStandFile(StandNode *node)
{
	AmmAttribute amm;
	int ret = 0;
	int offset = 0;
	int fd = 0;
	int i = 3;

	pthread_mutex_lock(&_hld_stand.f.mutex);
	//���ļ�
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
 * ��������:����ĳ������ͨ��ʱ��
 * ����:		index 	�����̨���е�����
 * ����ֵ:	0 �ɹ� -1ʧ��
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

	//��
	_hld_stand.m.stort[index]->last_t[0] = HexToBcd((t_tm->tm_year - 100));
	//��
	_hld_stand.m.stort[index]->last_t[1] = HexToBcd(t_tm->tm_mon + 1);
	//��
	_hld_stand.m.stort[index]->last_t[2] = HexToBcd(t_tm->tm_mday);
	//ʱ
	_hld_stand.m.stort[index]->last_t[3] = HexToBcd(t_tm->tm_hour);
	//��
	_hld_stand.m.stort[index]->last_t[4] = HexToBcd(t_tm->tm_min);
	//��
	_hld_stand.m.stort[index]->last_t[5] = HexToBcd(t_tm->tm_sec);

	pthread_mutex_unlock(&_hld_stand.m.mutex);

	return 0;
}

/*
 * ��������:��ȡ̨������Ӧ�������
 * ����	index	����
 * 		node 	���̨�˽ڵ�
 * ����ֵ 0�ɹ� -1ʧ��
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
 * ��������:����̨�˽ڵ����ݣ��ڴ��У�
 * ����:	index	����
 * 		node 	��������
 * ����ֵ: 0 �ɹ� -1 ʧ��
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
