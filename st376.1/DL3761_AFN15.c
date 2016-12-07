/*
 * DL3761_AFN12.c
 *
 *  Created on: 2016��8��11��
 *      Author: j
 */

#include "DL376_1.h"
#include "DL3761_AFN15.h"
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
#include "log3762.h"

/*
 * ��������:����ip���˿ں�
 * */
void DL3761_AFN15_07(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int fd;
	volatile int i = 3;	//���ļ�ʧ�����Դ���
	int res = -1;
	int len = 0;
	int outdex = 0;

	tpIpPort ip_port;
	memset(&ip_port, 0, sizeof(tpIpPort));

	//��ϵͳ���ò����ļ�
	while(i > 0)
	{

		i--;
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd < 0)
		{
			perror("open config file");
			res = -1;
		}
		else
		{
			//IP ��ַ
			memcpy(ip_port.Ip, rvframe3761->Frame376_1App.AppBuf + index, 4);
//			printf(" %d.%d.%d.%d\n",ip_port.Ip[0],ip_port.Ip[1],ip_port.Ip[2],ip_port.Ip[3]);
			index += 4;
			//��������
			memcpy(ip_port.NetMask, rvframe3761->Frame376_1App.AppBuf + index, 4);
			index += 4;
			//����
			memcpy(ip_port.GwIp, rvframe3761->Frame376_1App.AppBuf + index, 4);
			index += 4;
			//�˿ں�0
			ip_port.Port = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;
			index += 2;
			//�˿ں�1
			ip_port.TopPort = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;

			if(ip_port.TopPort == ip_port.Port)
			{
				res = -1;
				close(fd);
				break;
			}

			len = offsetof(tpIpPort, CS);
			ip_port.CS = Func_CS(&ip_port, len);
			len = offsetof(tpConfiguration, IpPort);
			res = WriteFile(fd, len, &ip_port, sizeof(tpIpPort));
//			printf("res********** %d\n",res);
			close(fd);
			break;
		}
	}
	if(0 <= res)
	{
		//���ݱ�ʶ
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}
	else
	{
		//���ݱ�ʶ
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
	_RebootInfrared = 0x66;
}

/*
 * ��������:��վip�Լ��˿�����
 * */
void DL3761_AFN15_05(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int fd;
	volatile int i = 3;	//���ļ�ʧ�����Դ���
	int res = -1;
	int len = 0;
	int outdex = 0;

	tpServer ip_port;
	memset(&ip_port, 0, sizeof(tpServer));

	//��ϵͳ���ò����ļ�
	while(i > 0)
	{

		i--;
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd < 0)
		{
			perror("open config file");
			res = -1;
		}
		else
		{
			//IP ��ַ
			memcpy(ip_port.ip, rvframe3761->Frame376_1App.AppBuf + index, 4);
//			printf(" %d.%d.%d.%d\n",ip_port.Ip[0],ip_port.Ip[1],ip_port.Ip[2],ip_port.Ip[3]);
			index += 4;
			//�˿ں�0
			ip_port.port = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;
			index += 2;
			len = offsetof(tpServer, CS);
			ip_port.CS = Func_CS(&ip_port, len);
			len = offsetof(tpConfiguration, server);
			res = WriteFile(fd, len, &ip_port, sizeof(tpServer));
			close(fd);
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.server, &ip_port, sizeof(tpServer));
			pthread_mutex_unlock(&_RunPara.mutex);
			break;
		}
	}
	if(0 <= res)
	{
		//���ݱ�ʶ
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}
	else
	{
		//���ݱ�ʶ
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
	_RebootInfrared = 0x66;
}

/*
 * ��������:����/�ر�376.2���ļ�¼
 * */
void DL3761_AFN15_20(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int outdex = 0;
	//��ȡ���ر��ļ�¼״̬
	unsigned char flag = rvframe3761->Frame376_1App.AppBuf[index];

	if(0 == flag)
	{	//�ر�
		close_log_3762();
	}
	else
	{	//��
		open_log_3762();
	}
	//���ݱ�ʶ
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}

/*..
 * ��������:��������
 * */
void DL3761_AFN15_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
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
	snframe3761->Frame376_1Link.CtlField.DIR = 0;	//����
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

	/*************************************Ӧ�ò�********************************/
	//������
	snframe3761->Frame376_1App.AFN = AFN3761_AFFI;
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

	//��������֡
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);
//	printf("AFN 15  %d\n",Fn);
	switch(Fn)
	{
		case 7:
			DL3761_AFN15_07(rvframe3761, snframe3761);
			break;
		case 5:
			DL3761_AFN15_05(rvframe3761, snframe3761);
			break;
		case 20:
			DL3761_AFN15_20(rvframe3761, snframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0,  sizeof(tpFrame376_1));
}
