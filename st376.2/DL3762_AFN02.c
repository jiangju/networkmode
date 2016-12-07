/*
 * DL3762_AFN2.c
 *
 *  Created on: 2016��8��8��
 *      Author: j
 */
#include <DL3762_AFN02.h>
#include "Collect.h"
#include "DL645.h"
#include <string.h>
#include <stdio.h>
#include "CommLib.h"

/*
 * ��������:�������е�AFN02_01����
 * */
void Create3762AFN02_01(unsigned char *amm, unsigned char type, unsigned char *inbuf, unsigned char len, tpFrame376_2 *snframe3762)
{
	unsigned short inext = 0;
	memset(snframe3762, 0, sizeof(tpFrame376_2));
	/*******************************��·��********************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	snframe3762->Frame376_2Link.CtlField.PRM = 0;	//�Ӷ�վ

	snframe3762->Frame376_2Link.CtlField.DIR = 1;	//����
	snframe3762->Frame376_2Link.CtlField.CommMode = 1;//ͨ��1
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;
	/*********************************Ӧ�ò�***********************************/
	//��Ϣ��
	snframe3762->Frame376_2App.R.PredictAnswerLen = 0x11;
	snframe3762->Frame376_2App.R.CommModeIdentifying = 0x01;	//ͨ��ģ���ʾ
	snframe3762->Frame376_2Link.Len += R_LEN;
	//��ַ��
	memcpy(snframe3762->Frame376_2App.Addr.SourceAddr, amm, AMM_ADDR_LEN);	//Դ��ַ

	pthread_mutex_lock(&_RunPara.mutex);
	memcpy(snframe3762->Frame376_2App.Addr.DestinationAddr, _RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);//Ŀ�ĵ�ַ
	pthread_mutex_unlock(&_RunPara.mutex);

	snframe3762->Frame376_2Link.Len += (6 + 6);
	//������
	snframe3762->Frame376_2App.AppData.AFN = AFN_DATASEND;	//�ϱ�
	snframe3762->Frame376_2Link.Len += AFN_LEN;
	//���ݱ�ʶ
	snframe3762->Frame376_2App.AppData.Buffer[inext++] = 0x01;
	snframe3762->Frame376_2App.AppData.Buffer[inext++] = 0x00;
	//ͨ��Э������
	snframe3762->Frame376_2App.AppData.Buffer[inext++] = type;
	//���ĳ���L
	snframe3762->Frame376_2App.AppData.Buffer[inext++] = len;
	//��������
	memcpy((snframe3762->Frame376_2App.AppData.Buffer + inext), inbuf, len);
	inext += len;
	snframe3762->Frame376_2App.AppData.Len += inext;
	snframe3762->Frame376_2Link.Len += inext;
	snframe3762->IsHaving = true;
}

/*
 * ��������:ת��ͨ��Э������֡
 * */
void AFN02_01(tpFrame376_2 *rvframe3762)
{
	unsigned short index = 2;
	struct task_e task;
	unsigned char len = 0;
	tpFrame645 frame645;

	//ͨ��Э������
	task.type = rvframe3762->Frame376_2App.AppData.Buffer[index++];
	//���ĳ���
	len  = rvframe3762->Frame376_2App.AppData.Buffer[index++];
	task.len = len;
	//��������
	memcpy(task.buf, (rvframe3762->Frame376_2App.AppData.Buffer + index), len);

	if(0 == ProtoAnaly645BufFromCycBuf(task.buf, len, &frame645))
	{
		memcpy(task.amm, frame645.Address, AMM_ADDR_LEN);
		memcpy(task.dadt, frame645.Datas, 4);
		task.next = NULL;
		pthread_mutex_lock(&_Collect.taske.taske_mutex);
		AddTashE(&task);
		pthread_mutex_unlock(&_Collect.taske.taske_mutex);
	}
}

/*
 * ��������:·������ת������
 * */
void DL3762_AFN02_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}
//	printf("AFN02  FN 	%d\n",Fn);
	switch(Fn)
	{
		case 1:
			AFN02_01(rvframe3762);
			break;
		default :
			break;
	}
	memset(rvframe3762, 0, sizeof(tpFrame376_2));
}
