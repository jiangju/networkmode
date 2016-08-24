/*
 * DL3762_AFN05.c
 *
 *  Created on: 2016��6��29��
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <DL3762_AFN05.h>
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "SysPara.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>

/*
 * ��������:�������ڵ��ַ
 * ����:	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN05_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	int fd;
	char i = 3;	//���ļ�ʧ�����Դ���
	int len = 0;
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	tpAFN05_1 afn05_1;
	unsigned short outIndex = 0;
	unsigned short inIndex = 0;
	inIndex += 2;	//ǰ2λΪ���ݵ�Ԫ��ʶ

	int res = -1;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));

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
			memcpy(&(afn05_1.HostNode), (rvframe3762->Frame376_2App.AppData.Buffer+inIndex), NODE_ADDR_LEN);
			inIndex += NODE_ADDR_LEN;
			memcpy(&_RunPara.AFN05_1.HostNode, &(afn05_1.HostNode), NODE_ADDR_LEN);
			len = offsetof(tpAFN05_1, CS);
			afn05_1.CS = Func_CS(&afn05_1, len);
			len = offsetof(tpConfiguration, AFN05_1);
			res = WriteFile(fd, len, &afn05_1, sizeof(tpAFN05_1));
			close(fd);
			break;
		}
	}
	if(0 > res)
	{
		Fn = 2;		//����
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//����״̬��
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xEE;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
	else
	{
		Fn = 1;		//ȷ��
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//��������
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}

}

/*
 * ��������:����ӽڵ��ϱ�
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN05_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	int fd;
	char i = 3;
	int len = 0;
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	tpAFN05_2 afn05_2;
	unsigned short outIndex = 0;
	unsigned short inIndex = 0;
	inIndex += 2;	//ǰ2λΪ���ݵ�Ԫ��ʶ

	int res = -1;
	memset(&afn05_2, 0, sizeof(tpAFN05_2));

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
			memcpy(&(afn05_2.IsAppera), (rvframe3762->Frame376_2App.AppData.Buffer+inIndex), 1);
			inIndex += 1;
			memcpy(&_RunPara.AFN05_2.IsAppera, &(afn05_2.IsAppera), 1);
			len = offsetof(tpAFN05_2, CS);
			afn05_2.CS = Func_CS(&afn05_2, len);
			len = offsetof(tpConfiguration, AFN05_1);
			res = WriteFile(fd, len, &afn05_2, sizeof(tpAFN05_1));
			close(fd);
			break;
		}
	}
	if(0 > res)
	{
		Fn = 2;		//����
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//����״̬��
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xEE;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
	else
	{
		Fn = 1;		//ȷ��
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//��������
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}

}

/*
 * ��������:�����㲥
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN05_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * ��������:���ôӽڵ������ʱʱ��
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN05_04(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	int fd;
	char i = 3;
	int len = 0;
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	tpAFN05_4 afn05_4;
	unsigned short outIndex = 0;
	unsigned short inIndex = 0;
	inIndex += 2;	//ǰ2λΪ���ݵ�Ԫ��ʶ

	int res = -1;
	memset(&afn05_4, 0, sizeof(tpAFN05_4));

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
			afn05_4.TimeOut = rvframe3762->Frame376_2App.AppData.Buffer[inIndex];
			_RunPara.AFN05_4.TimeOut = afn05_4.TimeOut;

			len = offsetof(tpAFN05_4, CS);
			afn05_4.CS = Func_CS((void*)&afn05_4, len);
			res = WriteFile(fd, 0, (void*)&afn05_4, sizeof(tpConfiguration));
			close(fd);
			break;
		}
	}
	if(0 > res)
	{
		Fn = 2;		//����
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//����״̬��
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xEE;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
	else
	{
		Fn = 1;		//ȷ��
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//��������
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;

		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
	}
}

/*
 * ��������:��֧�ֵ�AFN����Ӧ
 * */
void AFN05_Other(tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	unsigned char Fn = 0;
	unsigned short outIndex = 0;

	Fn = 2;		//����
	FNtoDT(Fn, DT);

	//���ݱ�ʶ
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

	//����״̬��
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x10;
	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:��������
 * ����: rvframe3762 ��������376.2����
 * 		snframe3762	���������ķ���376.2
 * ����ֵ:��
 * */
void DL3762_AFN05_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;
	unsigned short temp = 0;
	if(NULL == snframe3762)
	{
		return;
	}

	memset(snframe3762, 0, sizeof(tpFrame376_2));

	/**********************************�������·��*****************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	if(0 == rvframe3762->Frame376_2Link.CtlField.PRM ||
			1 == rvframe3762->Frame376_2Link.CtlField.DIR)
	{
		return;
	}
	snframe3762->Frame376_2Link.CtlField.PRM = 0;
	snframe3762->Frame376_2Link.CtlField.DIR = 1;

	snframe3762->Frame376_2Link.CtlField.CommMode = rvframe3762->Frame376_2Link.CtlField.CommMode;
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;

	/**************************************Ӧ�ò�************************************************/
	//�޵�ַ��
	memset(&snframe3762->Frame376_2App.R, 0, sizeof(tpR));
	snframe3762->Frame376_2App.R.MessageSerialNumber = rvframe3762->Frame376_2App.R.MessageSerialNumber;
	snframe3762->Frame376_2Link.Len += R_LEN;
	snframe3762->Frame376_2App.AppData.AFN = AFN_AFFI;	//ȷ��֡
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}

	temp = snframe3762->Frame376_2App.AppData.Len;
//	printf("AFN5  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN05_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN05_02(rvframe3762, snframe3762);
			break;
		case 4:
			AFN05_04(rvframe3762, snframe3762);
			break;
		default :
			AFN05_Other(snframe3762);
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}
