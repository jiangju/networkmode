/*
 * SeekAmm.c
 *
 *  Created on: 2016��10��13��
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

static struct initiative_stand _initiative_stand;		//��������̨��
static pthread_mutex_t _seek_amm_file_mutex;			//����̨���ļ���
static struct seek_amm_task_queue _seek_amm_task_queue;	//�����ѱ��������

/*
 * ��������:��ʼ������̨��(ֻ�ڿ�ͷ����һ��)
 * */
void init_initiative_stand(void)
{
	memset(&_initiative_stand, 0, sizeof(struct initiative_stand));
	pthread_mutex_init(&_initiative_stand.mutex, NULL);
	pthread_mutex_init(&_seek_amm_file_mutex, NULL);
	destroy_initiative_stand();
	destroy_initiative_stand_file();
	initiative_stand_all_index_init();
	initiative_stand_file_add_to_memory();
}

/*
 * ��������:�޸�����̨���ļ�ĳλ���Ƿ�ʹ��
 * ����:		index	�ļ�λ������
 * 			flag 	1 ʹ�� 0 δʹ��
 * */
void initiative_stand_index_is_using(unsigned int index, char flag)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	if(flag == 0)
		_initiative_stand.index[index] = 0x33;
	else
		_initiative_stand.index[index] = 0x00;

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * ��������:�޸�����̨���ļ�����λ�ö�û��ʹ��
 * */
void initiative_stand_all_index_init(void)
{
	pthread_mutex_lock(&_initiative_stand.mutex);

	memset(_initiative_stand.index, 0x33, NETWORK_MAX_CONNCET);

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * ��������:������Сδʹ������
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
 * ��������:�������̨��
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
 * ��������:�������̨���ļ�
 * */
int destroy_initiative_stand_file(void)
{
	int i = 0;
	int fd;
	struct seek_amm_result_store r_result;
	initiative_stand_all_index_init();
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
	return 0;
}

/*
 * ��������:�����ն˵�ַ��ȡ����̨���е��ѱ����
 * ����:		ter		�ն˵�ַ
 *			result	�ѱ����
 * ����ֵ:	0�ɹ�  -1 ʧ�� ��δ�ҵ�
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
 * ��������:�޸Ļ������ѱ����������̨��(�ڴ�)
 * ����:		result	�ѱ����
 * ����ֵ: 	0�ɹ� -1ʧ��
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

	if(NULL != temp)	//�޸�
	{
		memcpy(&temp->result, result, sizeof(struct seek_amm_result));
	}
	else				//����
	{
		temp = (struct seek_amm_node *)malloc(sizeof(struct seek_amm_node));
		if(NULL == temp)
		{
			pthread_mutex_unlock(&_initiative_stand.mutex);
			return -1;
		}
		temp->next = NULL;
		memcpy(&temp->result, result, sizeof(struct seek_amm_result));

		if(NULL == _initiative_stand.frist)	//��һ������
		{
			_initiative_stand.frist = temp;
			_initiative_stand.last = temp;
		}
		else								//�ǵ�һ������
		{
			_initiative_stand.last->next = temp;
			_initiative_stand.last = temp;
		}
	}

	pthread_mutex_unlock(&_initiative_stand.mutex);

	return 0;
}

/*
 * ��������:���ѱ����д������̨���ļ���
 * ����:		result	�ѱ����
 * ����ֵ:	0 �ɹ�	-1 ʧ��
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
 * ��������:ɾ������̨���е�ĳ���ڵ�
 * ����:		ter		�ն˵�ַ
 * ����ֵ:
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
	if(temp == _initiative_stand.frist)	//ɾ����һ��
	{
		_initiative_stand.frist = _initiative_stand.frist->next;
		if(NULL == _initiative_stand.frist)
			_initiative_stand.last = NULL;
		free(temp);
	}
	else	//ɾ���ǵ�һ��
	{
		temp_temp->next = temp->next;
		free(temp);
		if(NULL == temp_temp->next)
			_initiative_stand.last = temp_temp;
	}

	pthread_mutex_unlock(&_initiative_stand.mutex);
}

/*
 * ��������:������̨���ļ����ѱ���������뵽�ڴ��е�����̨����
 * ����ֵ: 0 �ɹ� -1ʧ��
 * */
int initiative_stand_file_add_to_memory(void)
{
	unsigned int i = 3;
	int fd;
	int ret = 0;
	int len = 0;
	struct seek_amm_result_store r_result;

	pthread_mutex_lock(&_seek_amm_file_mutex);

	if(0 == access(SEEK_AMM_FILE, F_OK))	//�����ļ�
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
			initiative_stand_all_index_init();
			return -1;
		}

		for(i = 0; i < NETWORK_MAX_CONNCET; i++)
		{
			memset(&r_result, 0, sizeof(struct seek_amm_result_store));
			ret = read(fd, &r_result, sizeof(struct seek_amm_result_store));
			if(ret != sizeof(struct seek_amm_result_store))
				break;
			len = offsetof(struct seek_amm_result_store, cs);
			if(r_result.cs == Func_CS(&r_result, len))	//������ȷ
			{
				if(0x55 == r_result.result.flag)	//��Ч
				{

					add_seek_amm_result(&r_result.result);
					initiative_stand_index_is_using(i, 1);
				}
				else	//��Ч
				{
					initiative_stand_index_is_using(i, 0);	//��־��λ��δʹ��
				}
			}
			else
			{
				initiative_stand_index_is_using(i, 0);	//��־��λ��δʹ��
			}
		}

		close(fd);
	}
	else									//�������ļ�
	{
		initiative_stand_all_index_init();
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
 * ��������:��ʼ���ѱ��������
 * */
void seek_amm_task_queue_init(void)
{
	_seek_amm_task_queue.frist = NULL;
	_seek_amm_task_queue.last = NULL;
	pthread_mutex_init(&_seek_amm_task_queue.mutex, NULL);
	pthread_cond_init(&_seek_amm_task_queue.queue_nonempty, NULL);
}

/*
 * ��������:�鿴�ѱ��������Ƿ��и��ն�
 * ����:	ter		�ն˵�ַ
 * 		task	�������
 * ����ֵ 0 ��  -1 û��
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
 * ��������:�����ѱ����񵽶���
 * ����:		task	�ѱ�����
 * ����ֵ: 0 �ɹ� -1ʧ��
 * */
int add_seek_amm_task(struct seek_amm_task *task)
{
	pthread_mutex_lock(&_seek_amm_task_queue.mutex);

	if(NULL == _seek_amm_task_queue.frist)	//��һ������
	{
		_seek_amm_task_queue.frist = task;
		_seek_amm_task_queue.last = _seek_amm_task_queue.frist;
		pthread_cond_broadcast(&_seek_amm_task_queue.queue_nonempty);
	}
	else									//�ǵ�һ������
	{
		_seek_amm_task_queue.last->next = task;
		_seek_amm_task_queue.last = _seek_amm_task_queue.last->next;
	}

	pthread_mutex_unlock(&_seek_amm_task_queue.mutex);

	return 0;
}

/*
 * ��������:��ѯĳ�ն��Ƿ������ѱ�
 * ����		ter		�ն˵�ַ
 * ����ֵ	0 �����ѱ�	-1û���ѱ�
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
 * ��������:�����ѱ�����֡
 * ����:	ter		�ն˵�ַ
 * 		key		����		0�� 1��
 * ����ֵ 0  �ɹ�  -1 ʧ��
 * */
int creat_afn5_98(unsigned char *ter, unsigned char key, tpFrame376_1 *outbuf)
{
	if(NULL == ter)
		return -1;

	int index = 0;

	memset(outbuf, 0, sizeof(tpFrame376_1));
	/************************************��·��************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//������
	outbuf->Frame376_1Link.EndChar = 0x16;
	//������
	outbuf->Frame376_1Link.CtlField.DIR = 0;	//����
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//����վ
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCBλ��Ч
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//��ַ��
	outbuf->Frame376_1Link.AddrField.WardCode[0] = ter[0];
	outbuf->Frame376_1Link.AddrField.WardCode[1] = ter[1];
	outbuf->Frame376_1Link.AddrField.Addr[0] = ter[2];
	outbuf->Frame376_1Link.AddrField.Addr[1] = ter[3];

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************Ӧ�ò�****************************************/
	//������
	outbuf->Frame376_1App.AFN = AFN3761_CTRL;
	//����ȷ�ϱ�־
	outbuf->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//ʱ���ǩ
	outbuf->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//���ݱ�ʶ	0x98
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x02;
	outbuf->Frame376_1App.AppBuf[index++] = 0x0B;
	outbuf->Frame376_1App.AppBuf[index++] = key;


	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;

	return 0;
}

/*
 * ��������:�ѱ������߳�
 * */
void *SeekAmm(void *arg)
{
	struct seek_amm_task *temp;
	struct seek_amm_task *temp_temp;
	while(1)
	{
		sleep(1);
		pthread_mutex_lock(&_seek_amm_task_queue.mutex);
		while(NULL == _seek_amm_task_queue.frist)
		{
			usleep(1);
			pthread_cond_wait(&_seek_amm_task_queue.queue_nonempty, &_seek_amm_task_queue.mutex);
		}

		temp = _seek_amm_task_queue.frist;

		while(NULL != temp)
		{
			temp_temp = temp;
			if(0x66 == temp->flag)	//�����Ѿ��·�  ���г�ʱ�ж�
			{
				if(temp->ticker <= 0)	//ɾ��������
				{
					if(_seek_amm_task_queue.frist == temp)	//ɾ����һ��
					{
						_seek_amm_task_queue.frist = _seek_amm_task_queue.frist->next;
						if(NULL == _seek_amm_task_queue.frist)
							_seek_amm_task_queue.last = NULL;
						free(temp);
					}
					else									//ɾ���ǵ�һ��
					{
						temp_temp->next = temp->next;
						free(temp);
						if(NULL == temp_temp->next)
							_seek_amm_task_queue.last = temp_temp;
					}
				}
				else
				{
					temp->ticker--;
				}
			}
			else					//����û���·� ���·�����
			{
				tpFrame376_1 outbuf;
				tp3761Buffer snbuf;
				TerSocket *p;
				int ret = 0;
				creat_afn5_98(temp->ter, 1, &outbuf);

				//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
				DL3761_Protocol_LinkPack(&outbuf, &snbuf);
				//���Ҷ�Ӧ���׽���
				pthread_mutex_lock(&(route_mutex));
				p = AccordTerSeek(temp->ter);
				if(p != NULL)
				{
					pthread_mutex_lock(&(p->write_mutex));

					while(1)
					{
						ret = write(p->s, snbuf.Data, snbuf.Len);
						if(ret < 0)
						{
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

				temp->flag = 0x66;
				temp->ticker = SEEK_AMM_TICKER;
			}

			temp = temp_temp->next;
		}

		pthread_mutex_unlock(&_seek_amm_task_queue.mutex);
	}

	pthread_exit(NULL);
}