/*
 * DL3762_AFN04.c
 *
 *  Created on: 2016��8��16��
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <DL3762_AFN04.h>
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
 * ��������:��֧�ֵ�AFN����Ӧ
 * */
void AFN04_Other(tpFrame376_2 *snframe3762)
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
 * ��������:��·���
 * */
void DL3762_AFN04_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
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

//	switch(Fn)
//	{
//		case 1:
//			AFN05_01(rvframe3762, snframe3762);
//			break;
//		case 2:
//			AFN05_02(rvframe3762, snframe3762);
//			break;
//		case 4:
//			AFN05_04(rvframe3762, snframe3762);
//			break;
//		default :
//			AFN05_Other(snframe3762);
//			break;
//	}

	AFN04_Other(snframe3762);

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}