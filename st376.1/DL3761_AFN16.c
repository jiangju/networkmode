/*
 * DL3761_AFN13.c
 *
 *  Created on: 2016��8��11��
 *      Author: j
 */

#include "DL376_1.h"
#include "DL3761_AFN16.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CommLib.h"
#include <stdlib.h>
#include <stddef.h>
#include "SysPara.h"
#include "Route.h"
#include "StandBook.h"
#include <sys/time.h>
#include "SeekAmm.h"
#include "log3762.h"
#include "hld_ac.h"
#include "HLDUsb.h"

/*
 * �������ܣ���ѯ��Ȩ��վip���˿�
 * */
void DL3761_AFN16_05(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	tpServer ip_port;
	memset(&ip_port, 0, sizeof(tpServer));
	int fd;
	int len = 0;
	int i = 3;
	while(i--)
	{
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd >= 0)
		{
			break;
		}
	}

	if(fd < 0)
	{
		printf("AFN 16 open CONFIG_FILE erro\n");
		return;
	}
	else
	{
		len = offsetof(tpConfiguration, server);
		if(0 == ReadFile(fd, len, (void *)(&ip_port), sizeof(tpServer)))
		{
			len = offsetof(tpServer, CS);
			if(ip_port.CS != Func_CS((void*)&ip_port, len))
			{
				memcpy(&ip_port, &_RunPara.server, sizeof(tpServer));
				ip_port.CS = Func_CS((void*)&ip_port, len);
				len = offsetof(tpConfiguration, server);
				WriteFile(fd, len, (void*)&ip_port, sizeof(tpServer));
			}
			close(fd);
		}
		else
		{
			printf("AFN 16 read CONFIG_FILE erro\n");
			close(fd);
			return;
		}
	}
	//ip
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.ip, 4);
	index += 4;

	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.port % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.port / 256;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;

}

/*
 * ��������:��ѯ�汾��
 * */
void DL3761_AFN16_06(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;

	//���̴���
	snframe3761->Frame376_1App.AppBuf[index++] = VENDOR_CODE0;
	snframe3761->Frame376_1App.AppBuf[index++] = VENDOR_CODE1;

	//оƬ����
	snframe3761->Frame376_1App.AppBuf[index++] = CHIP_CODE0;
	snframe3761->Frame376_1App.AppBuf[index++] = CHIP_CODE1;

	//�汾���� 	��
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE0;

	//�汾����	��
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE1;

	//�汾����	��
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE2;

	//�汾
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_NUM0;
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_NUM1;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * �������ܣ���ѯip �˿�
 * */
void DL3761_AFN16_07(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	tpIpPort ip_port;
	memset(&ip_port, 0, sizeof(tpIpPort));
	int fd;
	int len = 0;
	int i = 3;
	while(i--)
	{
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd >= 0)
		{
			break;
		}
	}

	if(fd < 0)
	{
		printf("AFN 16 open CONFIG_FILE erro\n");
		return;
	}
	else
	{
		len = offsetof(tpConfiguration, IpPort);
		if(0 == ReadFile(fd, len, (void *)(&ip_port), sizeof(tpIpPort)))
		{
			len = offsetof(tpIpPort, CS);
			if(ip_port.CS != Func_CS((void*)&ip_port, len))
			{
				memcpy(&ip_port, &_RunPara.IpPort, sizeof(tpIpPort));
				ip_port.CS = Func_CS((void*)&ip_port, len);
				len = offsetof(tpConfiguration, IpPort);
				WriteFile(fd, len, (void*)&ip_port, sizeof(tpIpPort));
			}
			close(fd);
		}
		else
		{
			printf("AFN 16 read CONFIG_FILE erro\n");
			close(fd);
			return;
		}
	}
	//ip
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.Ip, 4);
	index += 4;
	//��������
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.NetMask, 4);
	index += 4;
	//����
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.GwIp, 4);
	index += 4;
	//�˿ں�0
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port / 256;
	//�˿ں�1
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.TopPort % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.TopPort / 256;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;

}

/*
 * ��������:��ѯ���ڵ��ַ
 * */
void DL3761_AFN16_08(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	memcpy(snframe3761->Frame376_1App.AppBuf + index, _RunPara.AFN05_1.HostNode, 6);
	index += 6;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�����ն�������ѯ
 * */
void DL3761_AFN16_09(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	unsigned short num = 0;
	num = get_hld_route_node_num();

	snframe3761->Frame376_1App.AppBuf[index++] = num % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = num / 256;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�����ն˲�ѯ
 * */
void DL3761_AFN16_10(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;

	unsigned short all_num = 0;
	unsigned short start_num;
	unsigned char num = 0;
	unsigned char temp_num = 0;

	//��ȡ�����ն�������
	all_num = get_hld_route_node_num();

	//��ȡ����ѯ����ʼ���
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++];
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++] * 256 + start_num;
	if(0 == start_num)
	{
		start_num = 1;
	}

	//��ȡ��ѯ����
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//�ն�������
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num % 256;
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num / 256;

	if(start_num > all_num)
	{
		//����Ӧ������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//���㱾�λظ��ն�����
		if(all_num - start_num + 1 >= num)
		{
			temp_num = num;
		}
		else
		{
			temp_num = all_num - start_num + 1;
		}

		//����Ӧ���ն�����
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp_num;

		//����ն˵�ַ
		while(temp_num > 0)
		{
			//������Ż����Ӧ·�ɽڵ�
			if(0 != get_hld_route_node_ter_lstime(start_num,
					(snframe3761->Frame376_1App.AppBuf + out_inidex),
					(snframe3761->Frame376_1App.AppBuf + out_inidex+TER_ADDR_LEN)))
			{
				memset((snframe3761->Frame376_1App.AppBuf + out_inidex), 0xFF, TER_ADDR_LEN);
				memset((snframe3761->Frame376_1App.AppBuf + out_inidex + TER_ADDR_LEN), 0xFF, TIME_FRA_LEN);
			}
			out_inidex += TER_ADDR_LEN;
			out_inidex += TIME_FRA_LEN;
			start_num++;
			temp_num--;
		}
	}
	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�ն��������������ѯ
 * */
void DL3761_AFN16_11(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char all_num = 0;

	//��ȡ����ѯ�ն˵�ַ
	memcpy(ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);

	//��ѯ�ն��Ƿ�����
	if(0 != check_hld_route_node_ter(ter))
	{
		//������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//����
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	}

	//��ȡ�ն��µ������
	all_num = StatTerAmmNum(ter);

	//�ն��µ������
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�ն���������ѯ
 * */
void DL3761_AFN16_12(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char all_num = 0;
	unsigned short start_num;
	unsigned char num = 0;
	unsigned char temp_num = 0;
	int index = 0;
	StandNode node;

	//��ȡ����ѯ�ն˵�ַ
	memcpy(ter, rvframe3761->Frame376_1App.AppBuf + in_index, TER_ADDR_LEN);
	in_index += TER_ADDR_LEN;

	//��ȡ�����ʼ���
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//��ȡ����ȡ�������
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//��ѯ�ն��Ƿ�����
	if(0 != check_hld_route_node_ter(ter))
	{
		//������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//����
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	}

	//��ȡ�ն��µ������
	all_num = StatTerAmmNum(ter);

	//�ն��µ������
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num;

	if(start_num > all_num)
	{
		//����Ӧ������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//���㱾�λظ��������
		if(all_num - start_num + 1 >= num)
		{
			temp_num = num;
		}
		else
		{
			temp_num = all_num - start_num + 1;
		}

		//����Ӧ��������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp_num;

		while(temp_num > 0)
		{
			index = SeekTerOfAmm(ter, start_num);
			if(index == -1)
			{
				memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, AMM_ADDR_LEN);
				out_inidex += AMM_ADDR_LEN;
				memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, TIME_FRA_LEN);
				out_inidex += TIME_FRA_LEN;
			}
			else
			{
				if(-1 == GetStandNode(index, &node))
				{
					memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, AMM_ADDR_LEN);
					out_inidex += AMM_ADDR_LEN;
					memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, TIME_FRA_LEN);
					out_inidex += TIME_FRA_LEN;
				}
				else
				{
					memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.Amm, AMM_ADDR_LEN);
					out_inidex += AMM_ADDR_LEN;
					memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.last_t, TIME_FRA_LEN);
					out_inidex += TIME_FRA_LEN;
				}
			}
			start_num++;
			temp_num--;
		}
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯ�����Ϣ
 * */
void DL3761_AFN16_13(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;

	unsigned char amm[AMM_ADDR_LEN] = {0};
	int index = 0;
	StandNode node;
	//��ȡ����ַ
	memcpy(amm, (rvframe3761->Frame376_1App.AppBuf + in_index), AMM_ADDR_LEN);
	in_index += AMM_ADDR_LEN;

	//��ȡ�������
	index = SeekAmmAddr(amm, AMM_ADDR_LEN);
	//��ȡ�����Ϣ
	if(-1 == UpdateStandNode(index, &node))	//ʧ��
	{
		//������
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//����
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;

		//����Ӧ�ն�
		memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.Ter, TER_ADDR_LEN);
		out_inidex += TER_ADDR_LEN;

		//����Լ
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = node.type;

		//���һ��ͨ��ʱ��
		memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.last_t, TIME_FRA_LEN);
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯlog��־����״̬
 * */
void DL3761_AFN16_14(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;

	//
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//��log�����ļ�����ȡ��ǰ��¼״̬
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_mutex_unlock(&_log_3762_cfg_mutex);
			return;
		}
	}
	//��ȡ�����ļ�����
	lseek(_cfg3762_fd, 0, SEEK_SET);
	read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(_log_cfg.flag == 0x00)
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	else
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯģ�鵱ǰ״̬��
 * */
void DL3761_AFN16_15(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;
	unsigned char status = 0;

	//����״̬
	status = status | 0x01;

	//�ѱ�״̬  �Ƿ����ն������ѱ�
	if(0 == seek_amm_task_empty())
		status = status & 0xFD;
	else
		status = status | 0x02;
	//
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//��log�����ļ�����ȡ��ǰ��¼״̬
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_mutex_unlock(&_log_3762_cfg_mutex);
			return;
		}
	}
	//��ȡ�����ļ�����
	lseek(_cfg3762_fd, 0, SEEK_SET);
	read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(_log_cfg.flag == 0x00)
		status = status | 0x04;
	else
		status = status & 0xFB;
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);

	//��Ȩ״̬
	status = status | ((_hld_ac.get_status()) << 3);
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = status;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯģ�鵱ǰUSB״̬
 * */
void DL3761_AFN16_16(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;
	struct usb_status temp;

	get_usb_status(&temp);

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_in;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_log;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_update;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�ѱ���
 * */
void DL3761_AFN16_20(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned short i = 0;
	unsigned short num = 0;
	struct seek_amm_result result;
	//��ȡ�ն�������
	num = rvframe3761->Frame376_1App.AppBuf[in_index] + rvframe3761->Frame376_1App.AppBuf[in_index + 1] * 256;
	snframe3761->Frame376_1App.AppBuf[out_index] =  rvframe3761->Frame376_1App.AppBuf[in_index];
	snframe3761->Frame376_1App.AppBuf[out_index + 1] =  rvframe3761->Frame376_1App.AppBuf[in_index + 1];
	in_index += 2;
	out_index += 2;
	for(i = 0; i < num; i++)
	{
		//��ȡ����ѯ�ѱ������ն�
		memcpy(result.ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);
		in_index += TER_ADDR_LEN;
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), result.ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		//��ȡ�ѱ���
		if(0 == get_seek_amm_result(result.ter, &result))
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = (unsigned char)result.num;
		}
		else
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
		}
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�ѱ�������
 * */
void DL3761_AFN16_21(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned char num = 0;
	unsigned char index = 0;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char i = 0;
	struct seek_amm_result result;

	//��ȡ�ն˵�ַ
	memcpy(ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);
	in_index += TER_ADDR_LEN;
	//��ȡ��ʼ���
	index = rvframe3761->Frame376_1App.AppBuf[in_index++];
	if(0 == index)
		index = 1;
	//��ȡ����
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	if(0 == get_seek_amm_result(ter, &result))
	{
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		if((index + num - 1) > (unsigned char)result.num)
		{
			printf("index:    %d num:    %d NUM:   %d\n", index, num, result.num);
			if(index > (unsigned char)result.num)
			{
				snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
			}
			else
			{
				snframe3761->Frame376_1App.AppBuf[out_index++] = (unsigned char)result.num - index - 1;
				for(i = (index - 1); i < (unsigned char)result.num; i++)
				{
					memcpy((snframe3761->Frame376_1App.AppBuf + out_index),(unsigned char *)(result.amm + i), AMM_ADDR_LEN);
					out_index += AMM_ADDR_LEN;
				}
			}
		}
		else
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = num;
			for(i = (index - 1); i < (index - 1 + num); i++)
			{
				memcpy((snframe3761->Frame376_1App.AppBuf + out_index),(unsigned char *)(result.amm + i), AMM_ADDR_LEN);
				out_index += AMM_ADDR_LEN;
			}
		}

	}
	else
	{
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:�ѱ��ն�������ѯ
 * */
void DL3761_AFN16_22(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_index = 4;
	unsigned short num = 0;
	//��ȡ�ѱ��ն˸���
	num = (unsigned short)get_seek_amm_task_num();
	printf("afn 16 22 num: %d\n",num);
	//
	snframe3761->Frame376_1App.AppBuf[out_index++] = num % 256;
	snframe3761->Frame376_1App.AppBuf[out_index++] = num / 256;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * �������ܣ��ѱ�������ϸ��ѯ
 * */
void DL3761_AFN16_23(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned short start_num = 0;
	unsigned short all_num = 0;
	unsigned char  num = 0;
	unsigned char  r_num;
	struct seek_amm_task task;
	//��ȡ��ʼ���
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index] + rvframe3761->Frame376_1App.AppBuf[in_index + 1] * 256;
	if(start_num == 0)
		start_num = 1;
	//��ȡ��ȡ����
	num = rvframe3761->Frame376_1App.AppBuf[in_index + 2];

	//���㱾��Ӧ������
	all_num = (unsigned short)get_seek_amm_task_num();
	if((num + start_num - 1) > all_num)
	{
		if(start_num > all_num)
		{
			r_num = 0;
		}
		else
		{
			r_num = all_num - start_num + 1;
		}
	}
	else
	{
		r_num = num;
	}

	//������
	snframe3761->Frame376_1App.AppBuf[out_index++] = all_num % 256;
	snframe3761->Frame376_1App.AppBuf[out_index++] = all_num / 256;

	//����Ӧ������
	snframe3761->Frame376_1App.AppBuf[out_index++] = r_num;

	//�ն˵�ַ
	while(0 != r_num)
	{
		if(0 != get_n_seek_amm_task(start_num, &task))
		{
			memset((snframe3761->Frame376_1App.AppBuf + out_index), 0xFF, TER_ADDR_LEN);
		}
		else
		{
			memcpy((snframe3761->Frame376_1App.AppBuf + out_index), task.ter, TER_ADDR_LEN);
		}
		out_index += TER_ADDR_LEN;
		start_num++;
		r_num--;
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯ��Ȩʧ��ԭ��
 * */
void DL3761_AFN16_24(tpFrame376_1 *snframe3761)
{
	unsigned short out_index = 4;
	//
	snframe3761->Frame376_1App.AppBuf[out_index++] = _hld_ac.get_status();

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * ��������:��ѯ����
 * */
void DL3761_AFN16_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	if(NULL == rvframe3761 || NULL == snframe3761)
			return;
	/***********************************�����·��**************************/
	//��ʼ��
	snframe3761->Frame376_1Link.BeginChar0 = 0x68;
	snframe3761->Frame376_1Link.BeginChar1 = 0x68;
	//������
	snframe3761->Frame376_1Link.EndChar = 0x16;
	//������
	snframe3761->Frame376_1Link.CtlField.DIR = 1;	//����
	snframe3761->Frame376_1Link.CtlField.PRM = 0;	//�Ӷ�վ
	snframe3761->Frame376_1Link.CtlField.FCV = 0;	//FCBλ��Ч
	snframe3761->Frame376_1Link.CtlField.FCB = 0;
	snframe3761->Frame376_1Link.CtlField.FUNC_CODE = 0;	//������
	//��ַ��
	memcpy(snframe3761->Frame376_1Link.AddrField.WardCode,\
			rvframe3761->Frame376_1Link.AddrField.WardCode, 2);
	memcpy(snframe3761->Frame376_1Link.AddrField.Addr, \
			rvframe3761->Frame376_1Link.AddrField.Addr, 2);
	snframe3761->Frame376_1Link.AddrField.MSA = 0;

	//������
	snframe3761->Frame376_1App.AFN = AFN3761_EXTEND16;
	//����ȷ�ϱ�־
	snframe3761->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	snframe3761->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	snframe3761->Frame376_1App.SEQ.PSEQ_RSEQ = rvframe3761->Frame376_1App.SEQ.PSEQ_RSEQ;
	//ʱ���ǩ
	snframe3761->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	snframe3761->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	snframe3761->Frame376_1App.AUX.AUXEC.flag = 0;

	//���ݱ�ʶ
	snframe3761->Frame376_1App.AppBuf[0] = rvframe3761->Frame376_1App.AppBuf[0];
	snframe3761->Frame376_1App.AppBuf[1] = rvframe3761->Frame376_1App.AppBuf[1];
	snframe3761->Frame376_1App.AppBuf[2] = rvframe3761->Frame376_1App.AppBuf[2];
	snframe3761->Frame376_1App.AppBuf[3] = rvframe3761->Frame376_1App.AppBuf[3];

	//��������֡
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);

	switch(Fn)
	{
		case 5:
			DL3761_AFN16_05(snframe3761);
			break;
		case 6:
			DL3761_AFN16_06(snframe3761);
			break;
		case 7:
			DL3761_AFN16_07(rvframe3761, snframe3761);
			break;
		case 8:
			DL3761_AFN16_08(snframe3761);
			break;
		case 9:
			DL3761_AFN16_09(snframe3761);
			break;
		case 10:
			DL3761_AFN16_10(rvframe3761, snframe3761);
			break;
		case 11:
			DL3761_AFN16_11(rvframe3761, snframe3761);
			break;
		case 12:
			DL3761_AFN16_12(rvframe3761, snframe3761);
			break;
		case 13:
			DL3761_AFN16_13(rvframe3761, snframe3761);
			break;
		case 14:
			DL3761_AFN16_14(rvframe3761, snframe3761);
			break;
		case 15:
			DL3761_AFN16_15(rvframe3761, snframe3761);
			break;
		case 16:
			DL3761_AFN16_16(rvframe3761, snframe3761);
			break;
		case 20:
			DL3761_AFN16_20(rvframe3761, snframe3761);
			break;
		case 21:
			DL3761_AFN16_21(rvframe3761, snframe3761);
			break;
		case 22:
			DL3761_AFN16_22(rvframe3761, snframe3761);
			break;
		case 23:
			DL3761_AFN16_23(rvframe3761, snframe3761);
			break;
		case 24:
			DL3761_AFN16_24(snframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
