/*
 * DL3762_AFN12.c
 *
 *  Created on: 2016��7��22��
 *      Author: j
 */

#include "DL3762_AFN12.h"
#include <stdio.h>
#include "Collect.h"
#include "CommLib.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "SysPara.h"
#include "stddef.h"
/*
 * ��������:��������
 * */
void AFN12_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	_Collect.taskc.key = 1;
	_Collect.taskc.index = 0;

	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	unsigned short outIndex = 0;
	int fd;
	int i = 3;

	//�޸ĳ�����״̬
	TaskcReset();

	//�޸ĳ���ɹ���־
	while(i--)
	{
		fd = open(CONFIG_FILE, O_RDWR, 0666);
		if(fd >= 0)
			break;
	}
	if(fd >= 0)
	{
		//���³���ɹ���־
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.CyFlag--;
		tpCyFlag  cyFlag;
		memset(&cyFlag, 0, sizeof(tpCyFlag));
		cyFlag.flag = _RunPara.CyFlag;
		pthread_mutex_unlock(&_RunPara.mutex);
		int len = 0;

		len = offsetof(tpCyFlag, CS);
		cyFlag.CS = Func_CS((void*)&cyFlag, len);

		len = offsetof(tpConfiguration, cyFlag);
		WriteFile(fd, len, (void*)&cyFlag, sizeof(tpCyFlag));
		close(fd);
	}
	else
	{
		pthread_mutex_lock(&_RunPara.mutex);
		_RunPara.CyFlag--;
		pthread_mutex_unlock(&_RunPara.mutex);
	}
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

/*
 * ��������:������ͣ
 * */
void AFN12_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	_Collect.taskc.key = 0;

	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	unsigned short outIndex = 0;
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

/*
 * ��������:��������
 * */
void AFN12_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	_Collect.taskc.key = 1;

	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	unsigned short outIndex = 0;
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

void DL3762_AFN12_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;
	unsigned short temp = 0;
	if(NULL == snframe3762)
	{
		return;
	}
	memset(snframe3762, 0, sizeof(tpFrame376_2));

	/**************************************��·��***************************************/
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

	/****************************************Ӧ�ò�******************************************/
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
	printf("AFN 12  FN  %d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN12_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN12_02(rvframe3762, snframe3762);
			break;
		case 3:
			AFN12_03(rvframe3762, snframe3762);
			break;
		default :
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}
