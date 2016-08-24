/*
 * DL3762_AFN10.c
 *
 *  Created on: 2016��7��4��
 *      Author: j
 */
#include "DL3762_AFN10.h"
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
#include <stdio.h>
#include <string.h>
#include "StandBook.h"

/*
 * ��������:��ѯ�ӽڵ�����
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	unsigned char temp = 0;

	//�ӽڵ�����
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//���֧�ִӽڵ�����
	temp = AMM_MAX_NUM % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = AMM_MAX_NUM / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:��ѯ�ӽڵ���Ϣ
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char temp = 0;
	unsigned short serialNum = 0;
	unsigned char count0 = 0;
	unsigned char count1 = 0;
	unsigned short inIndex = 2;
	unsigned short outIndex = 2;
	int	i = 0;

	//�жϽ������ݵĳ����Ƿ����	���ݱ�ʶ+�ӽڵ���ʼ���+�ӽڵ�����
	if(rvframe3762->Frame376_2App.AppData.Len < (2 + 2 + 1))
	{
//		printf("AFN10 len 70 erro\n");
		return;
	}

	//��ȡ��ʼ���
	temp = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	serialNum = temp + 256 * rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	if(0 == serialNum)
	{
		serialNum = 1;
	}

	//��ȡ����
	count0 = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//��䷢������
	//�ӽڵ�������
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//����Ӧ��������	�������·�����ʼ���1Ϊ��С
	if(_StandNodeNum < serialNum)
	{
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++]  = 0;
		snframe3762->Frame376_2App.AppData.Len = outIndex;
		snframe3762->Frame376_2Link.Len += outIndex;
		return;
	}

	if((_StandNodeNum - serialNum + 1) < count0)
	{
		count1 = _StandNodeNum - serialNum + 1;
	}
	else
	{
		count1 = count0;
	}
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = count1;

	//�ӽڵ���Ϣ
	for(i = 0; i < count1; i++)
	{
		//�ӽڵ��ַ
		memcpy((snframe3762->Frame376_2App.AppData.Buffer + outIndex),
				_SortNode[serialNum - 1 + i]->Amm, AMM_ADDR_LEN);
		outIndex += AMM_ADDR_LEN;

		//�ӽڵ���Ϣ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0xFF;
		temp = (_SortNode[serialNum - 1 + i]->type << 3)  & 0x38;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp | 0x4;
	}

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:ָ���ӽڵ����һ���м�·����Ϣ
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;

	//�ṩ·�ɵĴӽڵ�������n
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:·������״̬
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_04(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	int	i = 0;
	unsigned char temp = 0;
	unsigned short num = 0;
	//����״̬��(·��ѧϰ���)
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x01;

	//�ӽڵ�����
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//�ѳ��սڵ�����
	for(i = 0; i < _StandNodeNum; i++)
	{
		if(_SortNode[i]->cyFlag == _RunPara.CyFlag)
		{
			num++;
		}
	}
	temp = num % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = num / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//�м̳����ӽڵ�����
	temp = num % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = num / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//��������
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x01;

	//ͨ������
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x02;

	//��һ���м̼���
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//�ڶ����м̼���
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//�������м̼���
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
	//��һ�������
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;
	//�ڶ��������
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;
	//�����������
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x08;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:δ�����ɹ��Ĵӽڵ���Ϣ
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_05(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short sernum = 0;	//��ʼ���
	unsigned char  num = 0;		//����
	unsigned short outIndex = 2;
	unsigned short tempout = 0;
	unsigned short inIndex = 2;
	unsigned short temp = 0;
	unsigned short tempnum = 0;
	int i = 0;

	//�����ʼ���
	sernum = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	sernum = sernum + 256 * rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//��ȡ����
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	if(sernum < 0 || num <= 0)
		return;
	if(sernum == 0)
		sernum = 1;

	//�ӽڵ�����
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//����Ӧ������
	outIndex++;
	tempout = outIndex;	//���ݱ���Ӧ����������

	for(i = 0; i < num && ((sernum + i) < _StandNodeNum); i++)
	{
		//�����ɹ�
		if(_SortNode[sernum - 1 + i]->cyFlag != _RunPara.CyFlag)
		{
			tempnum++;
			//�ӽڵ��ַ
			memcpy(snframe3762->Frame376_2App.AppData.Buffer + outIndex, _SortNode[sernum - 1 + i], AMM_ADDR_LEN);
			outIndex += outIndex;
			//�ӽڵ���Ϣ
			snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0x00;
			temp = _SortNode[sernum - 1 + i]->type;
			temp &= 0x7;
			temp = temp << 3;
			temp = temp || 0x01;
			snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
		}
	}

	snframe3762->Frame376_2App.AppData.Buffer[tempout] = tempnum;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:����ע��Ĵӽڵ���Ϣ
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN10_06(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 2;
	int	i = 0;
	unsigned char temp = 0;

	//�ӽڵ�����
	temp = (unsigned short)_StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;
	temp = (unsigned short)_StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = temp;

	//����Ӧ��Ĵӽڵ�����
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:·�ɲ�ѯ
 * ����: rvframe3762 ��������376.2����
 * 		snframe3762	���������ķ���376.2
 * ����ֵ:��
 * */
void DL3762_AFN10_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
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
	snframe3762->Frame376_2App.AppData.AFN = AFN_ROUTEQUERY;	//
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	snframe3762->Frame376_2App.AppData.Buffer[0] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[1] = DT[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}
	snframe3762->Frame376_2App.AppData.Len = 2;
	temp = snframe3762->Frame376_2App.AppData.Len;
//	printf("AFN10  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN10_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN10_02(rvframe3762, snframe3762);
			break;
		case 3:
			AFN10_03(rvframe3762, snframe3762);
			break;
		case 4:
			AFN10_04(rvframe3762, snframe3762);
			break;
		case 5:
			AFN10_05(rvframe3762, snframe3762);
			break;
		case 6:
			AFN10_06(rvframe3762, snframe3762);
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

