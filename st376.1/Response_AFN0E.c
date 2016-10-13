/*
 * DL3761_AFN0E.c
 *
 *  Created on: 2016��8��16��
 *      Author: j
 */

#include "DL376_1_DataType.h"
#include "DL376_1.h"
#include "Response_AFN0E.h"
#include <stdio.h>
#include <string.h>

/*
 * ��������:��Ӧ�¼��ϱ�
 * */
void ResponseAFN0E_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	if(NULL == snframe3761)
			return;
	int index = 0;
	/**********************************�����·��*******************************/
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
	//snframe3761->Frame376_1Link.CtlField.ACD  = 0;
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

	//���ݱ�ʶ
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x01;
	snframe3761->Frame376_1App.AppBuf[index++] = 0x00;

	//Ӧ�ò�֡��---������AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}
