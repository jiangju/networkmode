/*
 * DL3762_AFN06.c
 *
 *  Created on: 2016��7��25��
 *      Author: j
 */
#include "DL3762_AFN06.h"
#include "StandBook.h"
#include <string.h>
#include <stdio.h>
/*
 * ��������:�����ϱ��������ݱ���
 * ����:	index	�����̨���е����
 * 		type	ͨ������
 * 		inbuf	���뻺��
 * 		len		���ݳ���
 * 		snframe3762 �������
 * */
void Create3762AFN06_02(unsigned short index,unsigned char type,
		unsigned char *inbuf, unsigned char len, tpFrame376_2 *snframe3762)
{
	unsigned short outindex = 0;
	memset(snframe3762, 0, sizeof(tpFrame376_2));
	StandNode node;
	/************************************��·��*************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	snframe3762->Frame376_2Link.CtlField.PRM = 0;	//�Ӷ�վ

	snframe3762->Frame376_2Link.CtlField.DIR = 1;	//����
	snframe3762->Frame376_2Link.CtlField.CommMode = 1;//ͨ��1
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;
	/*************************************Ӧ�ò�*************************************/
	//��Ϣ��
	snframe3762->Frame376_2App.R.PredictAnswerLen = 0x11;
	snframe3762->Frame376_2App.R.CommModeIdentifying = 0x01;	//ͨ��ģ���ʾ
	snframe3762->Frame376_2Link.Len += R_LEN;
	//��ַ��
	GetStandNode(index, &node);
	memcpy(snframe3762->Frame376_2App.Addr.SourceAddr, node.Amm, AMM_ADDR_LEN);	//Դ��ַ
	memcpy(snframe3762->Frame376_2App.Addr.DestinationAddr, _RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);//Ŀ�ĵ�ַ
	snframe3762->Frame376_2Link.Len += (6 + 6);
	//������
	snframe3762->Frame376_2App.AppData.AFN = AFN_ACCORD;	//�ϱ�
	snframe3762->Frame376_2Link.Len += AFN_LEN;
	//���ݱ�ʶ
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x00;
	//�ӽڵ����
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = (unsigned char)((index+1) % 256);
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = (unsigned char)((index+1) / 256);
	//ͨ��Э������
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = type;
	//��ǰ����ͨ������ʱ��
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x01;
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = 0x00;
	//���ĳ���
	snframe3762->Frame376_2App.AppData.Buffer[outindex++] = len;
	//��������
	memcpy((snframe3762->Frame376_2App.AppData.Buffer + outindex), inbuf, len);
	outindex += len;
	snframe3762->Frame376_2App.AppData.Len += outindex;
	snframe3762->Frame376_2Link.Len += outindex;
	snframe3762->IsHaving = true;
}

