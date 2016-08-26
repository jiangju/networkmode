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
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port / 256;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
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
		case 7:
			DL3761_AFN16_07(rvframe3761, snframe3761);
			break;
		default:
			break;
	}
}
