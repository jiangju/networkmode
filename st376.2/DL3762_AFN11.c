/*
 * DL3762_AFN11.c
 *
 *  Created on: 2016��6��30��
 *      Author: j
 */
#include "DL3762_AFN11.h"
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
 * ��������:��Ӵӽڵ�
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_01(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	StandNode node;
	int index = 2;	//������������
	unsigned char num = 0;
	int ret = 0;
	unsigned char flag = 0;	//�����־
	int fd;
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	unsigned short outIndex = 0;
	int i = 3;

	//�õ�����ӵĴӽڵ����
	num = rvframe3762->Frame376_2App.AppData.Buffer[index++];

	if(rvframe3762->Frame376_2App.AppData.Len < (num * 7 + 3))
	{
		ret = -1;
		flag = 0;	//��ʽ����
		goto Afn0;
	}
	else
	{
		while(num--)
		{
			memset(&node, 0xFF, sizeof(StandNode));
			//����ַ
			memcpy(node.Amm, (rvframe3762->Frame376_2App.AppData.Buffer + index), AMM_ADDR_LEN);
			index += AMM_ADDR_LEN;
			//��Լ����
			node.type = rvframe3762->Frame376_2App.AppData.Buffer[index++];
			//��ȡ�洢���
			node.num = GetNearestSurplus();
			//�����־(_RunPara.CyFlagΪ����ɹ���־�������ĳ����־������ʱ������ɹ���ÿ����������ʱ��_RunPara.CyFlag - 1)
			//�������־����Ϊ��_Runpara.CyFlag�����
			pthread_mutex_lock(&_RunPara.mutex);
			node.cyFlag = _RunPara.CyFlag + 1;
			pthread_mutex_unlock(&_RunPara.mutex);
			//���/�޸ăȴ��е�̨�˽ڵ�
			AddNodeStand(&node);
			//���ڴ��л�ȡ�ýڵ�д���ļ���
			ret = SeekAmmAddr(node.Amm, AMM_ADDR_LEN);
			if(-1 != ret)
			{
//				printf("add node stand\n");
				GetStandNode(ret, &node);
				ret = AlterNodeStandFile(&node);
			}
		}

		pthread_mutex_lock(&_RunPara.mutex);
		if(0x01 != _RunPara.StandFlag)
		{
			_RunPara.StandFlag = 0x1;
			//���ļ�
			while(i--)
			{
				fd = open(CONFIG_FILE, O_RDWR | O_CREAT, 0666);
				if(fd >=0 )
					break;
			}

			if(fd < 0)
			{
				pthread_mutex_unlock(&_RunPara.mutex);
				goto Afn0;
			}

			tpIsStand stand;
			int len;
			len = offsetof(tpConfiguration, StandFlag);
			memcpy(&stand.flag, &_RunPara.StandFlag, 1);
			len = offsetof(tpIsStand, CS);
			stand.CS = Func_CS((void*)&stand, len);
			len = offsetof(tpConfiguration, StandFlag);
			WriteFile(fd, len, (void*)&stand, sizeof(tpCyFlag));
			close(fd);
		}
		pthread_mutex_unlock(&_RunPara.mutex);
	}

Afn0:		//Ӧ��
	if(0 > ret)
	{
		Fn = 2;		//����
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//����״̬��
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = flag;
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

	return;
}

/*
 * ��������:ɾ���ӽڵ�
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_02(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	AmmAttribute Amm;
	unsigned short inIndex = 2;	//������������
	unsigned short outIndex = 0;	//�����������
	unsigned char num = 0;	//ɾ��������
	int	ret = -1;
	unsigned char flag = 0;
	int i = 3;
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};
	//��ô�ɾ��������
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	//�ж�ɾ��֡�ĳ����Ƿ���ȷ	���ݱ�ʶ+�ӽڵ�����+ÿ���ӽڵ������
	if(rvframe3762->Frame376_2App.AppData.Len < (2 + 1 + AMM_ADDR_LEN * num))
	{
		ret = -1;
		flag = 0;
	}
	else
	{
		for(i = 0; i < num; i++)
		{
			memset(&Amm, 0, sizeof(AmmAttribute));
			memcpy(Amm.Amm, (rvframe3762->Frame376_2App.AppData.Buffer+inIndex), AMM_ADDR_LEN);
			inIndex += AMM_ADDR_LEN;
			DeleNodeStand(Amm.Amm);
		}
		ret = 0;
	}

	//����֡
	if(0 > ret)
	{
		Fn = 2;		//����
		FNtoDT(Fn, DT);

		//���ݱ�ʶ
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[0];
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = DT[1];

		//����״̬��
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = flag;
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
 * ��������:���ôӽڵ�̶��м�·��
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * ��������:����·�ɹ���ģʽ
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_04(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * ��������:����ӽڵ�����ע��
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_05(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * ��������:��ֹ�ӽڵ�����ע��
 * ����	rvframe3762	��ִ�е�376.2
 * 		snframe3762	�����ص�376.2
 * ����ֵ:��
 * */
void AFN11_06(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{

}

/*
 * ��������:��֧�ֵ�AFN����Ӧ
 * */
void AFN11_Other(tpFrame376_2 *snframe3762)
{
	unsigned short outIndex = 0;	//�����������
	unsigned char Fn = 0;
	unsigned char DT[2] = {0};

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
 * ��������:·������
 * ����: rvframe3762 ��������376.2����
 * 		snframe3762	���������ķ���376.2
 * ����ֵ:��
 * */
void DL3762_AFN11_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
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
	printf("AFN11  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN11_01(rvframe3762, snframe3762);
			break;
		case 2:
			AFN11_02(rvframe3762, snframe3762);
			break;
//		case 3:
//			AFN11_03(rvframe3762, snframe3762);
//			break;
//		case 4:
//			AFN11_04(rvframe3762, snframe3762);
//			break;
//		case 5:
//			AFN11_05(rvframe3762, snframe3762);
//			break;
//		case 6:
//			AFN11_06(rvframe3762, snframe3762);
//			break;
		default :
			AFN11_Other(snframe3762);
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}

	snframe3762->IsHaving = true;
}

