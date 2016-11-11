/*
 * AFN03.c
 *
 *  Created on: 2016��6��27��
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <DL3762_AFN03.h>
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
#include "StandBook.h"

/*
 *��������:��ѯ���̴���Ͱ汾��Ϣ
 *����:	snframe3762	�����ķ���֡�ṹ
 *����ֵ:	��
 * */
void AFN03_01(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	//���̴���
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE1;

	//оƬ����
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE1;

	//�汾����  ��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE0;

	//�汾����  ��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE1;

	//�汾����  ��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE2;

	//�汾
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM1;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
//	printf("L1  %d\n",snframe3762->Frame376_2App.AppData.Len);
//	printf("L2  %d\n",snframe3762->Frame376_2Link.Len);
}

/*
 * ��������:��ѯ����ֵ
 * ����snframe3762	�����ķ���֡�ṹ
 * */
void AFN03_02(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 * ��������:�ӽڵ�������Ϣ
 * */
void AFN03_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char p = 0;
	unsigned char num = 0;
	unsigned short inIndex = 2;
	unsigned short outIndex = 2;
	StandNode node;
	//��ȡ��ʼ�ڵ�����
	p = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	//��ȡ����
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	if(p == 0)
	{
		p = 1;
	}
	else if(p > _StandNodeNum)
	{
		num = 0;
	}
	else if((p + num) > _StandNodeNum)
	{
		num = p + num - _StandNodeNum + 1;
	}

	//֡�����ӽ��ܵ�����
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = num;
	//�������ı�֡����Ĵӽڵ�����
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = num;
	while(num > 0)
	{
		if(0 > GetStandNode(p + num - 1, &node))
		{
			num--;
			continue;
		}
		//�ӽڵ��ַ
		memcpy(snframe3762->Frame376_2App.AppData.Buffer + outIndex, node.Amm, AMM_ADDR_LEN);
		outIndex += AMM_ADDR_LEN;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;
		num--;
	}

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * ��������:��ѯ���ڵ��ַ
 * ����:snframe3762	�����ķ���֡�ṹ
 * */
void AFN03_04(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	tpAFN05_1 afn05_1;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));
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

	}
	else
	{
		len = offsetof(tpConfiguration, AFN05_1);
		if(0 == ReadFile(fd, len, (void *)(&afn05_1), sizeof(tpAFN05_1)))
		{
			len = offsetof(tpAFN05_1, CS);
			if(afn05_1.CS != Func_CS((void*)&afn05_1, len))
			{
				//�����в��������ڵ��ַ�������ò��������ڵ��ַ
				memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
				afn05_1.CS = Func_CS((void*)&afn05_1, len);
				len = offsetof(tpConfiguration, AFN05_1);
				WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
			}
		}
		else
		{
			close(fd);
		}
	}

	memcpy((snframe3762->Frame376_2App.AppData.Buffer + index), &afn05_1.HostNode, NODE_ADDR_LEN);
	index += NODE_ADDR_LEN;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;

	close(fd);
}

/*
 * ��������:���ڵ�״̬�ֺ�ͨ������
 * */
void AFN03_05(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	//״̬��(���ڳ���ģʽ + ���ڵ��ŵ����� + �������� + ���� + �ŵ�����)
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x30;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0F;
	//
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x04;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x64;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x32;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_06(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 * ��������:��ȡ�ӽڵ������ʱʱ��
 * */
void AFN03_07(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	//�ӽڵ������ʱʱ�䣨��λS��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x5A;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFF;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_08(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_09(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 *��������:����ͨ��ģ������ģʽ��Ϣ
 **/
void AFN03_10(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	tpAFN05_1 afn05_1;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));
	int fd;
	int len = 0;
	int i = 3;
	unsigned char temp = 0;
	//ͨ�ŷ�ʽ + ·�ɹ���ʽ + ��������Ϣģʽ + ���ڳ���ģʽ
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xF1;
	//������ʱ����֧�� + ʧ�ܽڵ��л�����ʽ + �㲥����ȷ�Ϸ�ʽ + �㲥�����ŵ�ִ�з�ʽ
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//�ŵ����� + ��������n
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x60;
	//��ѹ����������Ϣ
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//����
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//�ӽڵ������ʱʱ�䣨��λS��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x5A;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFF;
	//�㲥�������ʱʱ�䣨��λS��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFA;
	//���֧�ֵ�376.2���ĳ���
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xE2;
	//�ļ�����֧�ֵ���󵥰�����
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x28;
	//���������ȴ�ʱ��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x22;
	//���ڵ��ַ
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
	}
	else
	{
		len = offsetof(tpConfiguration, AFN05_1);
		if(0 == ReadFile(fd, len, (void *)(&afn05_1), sizeof(tpAFN05_1)))
		{
			len = offsetof(tpAFN05_1, CS);
			if(afn05_1.CS != Func_CS((void*)&afn05_1, len))
			{
				//�����в��������ڵ��ַ�������ò��������ڵ��ַ
				memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
				afn05_1.CS = Func_CS((void*)&afn05_1, len);
				len = offsetof(tpConfiguration, AFN05_1);
				WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
			}
			close(fd);
		}
		else
		{
			close(fd);
		}
	}
	memcpy((snframe3762->Frame376_2App.AppData.Buffer + index), &afn05_1.HostNode, NODE_ADDR_LEN);
	index += NODE_ADDR_LEN;
	//���֧�ִӽڵ�����
	temp = AMM_MAX_NUM % 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	temp = AMM_MAX_NUM / 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	//�ӽڵ�����
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	//ͨ��ģ��ʹ�õ�376.2Э�鷢�����ڣ�BCD��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x16;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x07;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x27;
	//ͨ��ģ��ʹ�õ�376.2Э����󱸰����ڣ�BCD��
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x09;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x13;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x43;
	//ͨ��ģ�鳧�̴��뼰�汾��Ϣ
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM1;
	//
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x04;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x64;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x32;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 *��������:����ͨ��ģ��376.2����֧����Ϣ
 **/
void AFN03_11(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short inIndex = 2;
	unsigned short outInidex = 2;
	unsigned char AFN = 0;

	//��ȡ����ѯ�Ĺ�����
	AFN = rvframe3762->Frame376_2App.AppData.Buffer[inIndex];

	//�������
	switch (AFN)
	{
		case 0:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 1:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 3:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x5B;
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 30);
			outInidex += 30;
			break;
		case 5:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x0D;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 6:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x06;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 10:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x1B;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 11:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 12:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 13:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x01;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 14:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		default:
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 32);
			outInidex += 32;
			break;
	}
	snframe3762->Frame376_2App.AppData.Len = outInidex;
	snframe3762->Frame376_2Link.Len += outInidex;
}

/*
 * ��������:ִ�в�ѯ����
 * ����: rvframe3762 ��������376.2����
 * 		snframe3762	���������ķ���376.2
 * ����ֵ:��
 * */
void DL3762_AFN03_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
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

	/************************************Ӧ�ò�**********************************************/
	//�޵�ַ��
	memset(&snframe3762->Frame376_2App.R, 0, sizeof(tpR));
	snframe3762->Frame376_2App.R.MessageSerialNumber = rvframe3762->Frame376_2App.R.MessageSerialNumber;
	snframe3762->Frame376_2Link.Len += R_LEN;
	snframe3762->Frame376_2App.AppData.AFN = AFN_QUERDATA;
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}

	snframe3762->Frame376_2App.AppData.Buffer[0] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[1] = DT[1];
	snframe3762->Frame376_2App.AppData.Len = 2;
	temp = snframe3762->Frame376_2App.AppData.Len;
	switch(Fn)
	{
		case 1:
			AFN03_01(snframe3762);
			break;
		case 2:
			AFN03_02(snframe3762);
			break;
		case 4:
			AFN03_04(snframe3762);
			break;
		case 5:
			AFN03_05(snframe3762);
			break;
		case 6:
			AFN03_06(snframe3762);
			break;
		case 7:
			AFN03_07(snframe3762);
			break;
		case 8:
			AFN03_08(snframe3762);
			break;
		case 9:
			AFN03_09(snframe3762);
			break;
		case 10:
			AFN03_10(snframe3762);
			break;
		case 11:
			AFN03_11(rvframe3762, snframe3762);
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

