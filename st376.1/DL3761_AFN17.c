/*
 * DL3761_AFN17.c
 *
 *  Created on: 2016��10��21��
 *      Author: j
 */
#include "DL376_1.h"
#include "DL3761_AFN17.h"
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
#include "Route.h"
#include "SeekAmm.h"
#include "StandBook.h"
#include <sys/time.h>

/*
 * ��������:�ѱ�����̨��ƥ��
 * ����:		amm		���
 * 			ter		�ն�
 * ����ֵ 0  -1
 * */
int seek_amm_synchroniza(unsigned char *amm, unsigned char *ter)
{
	if(NULL == amm || NULL == ter)
		return -1;
	StandNode node;
	int i = 3;
	int fd, ret;
	//���ļ�
	while(i--)
	{
		fd = open(STAND_BOOK_FILE, O_RDWR);
		if(fd >=0 )
			break;
	}

	if(fd < 0)
		return -1;

	if(0x01 == _RunPara.StandFlag)
	{
		//�鿴�Ƿ�����ͬ�ĵ��
		ret = SeekAmmAddr(node.Amm, AMM_ADDR_LEN);
		if(ret >= 0)
		{
			GetStandNode(ret, &node);
			memcpy(node.Ter, ter, TER_ADDR_LEN);
			AddNodeStand(&node);
			AlterNodeStandFile(fd, &node);
		}
	}
	else
	{
		memset(&node, 0xFF, sizeof(StandNode));
		//����ַ
		memcpy(node.Amm, (amm), AMM_ADDR_LEN);
		//�ն˵�ַ
		memcpy(node.Ter, ter, TER_ADDR_LEN);
		//��Լ����
		node.type = 0x02;
		//��ȡ�洢���
		node.num = GetNearestSurplus();
		//�����־(_RunPara.CyFlagΪ����ɹ���־�������ĳ����־������ʱ������ɹ���ÿ����������ʱ��_RunPara.CyFlag - 1)
		//�������־����Ϊ��_Runpara.CyFlag�����
		node.cyFlag = _RunPara.CyFlag + 1;
		//���/�޸ăȴ��е�̨�˽ڵ�
		AddNodeStand(&node);
		AlterNodeStandFile(fd, &node);
	}
	close(fd);
	return 0;
}

/*
 * ��������:�ѱ����������ϱ�����
 * */
void DL3761_AFN17_01(tpFrame376_1 *rvframe3761)
{
	unsigned short in_index = 4;
	unsigned char num = 0;
	int i = 0;
	struct seek_amm_result *result;

	result = (struct seek_amm_result *)malloc(sizeof(struct seek_amm_result));
	if(NULL == result)
		return;
	memset(result, 0, sizeof(struct seek_amm_result));

	result->ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
	result->ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
	result->ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
	result->ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

	//��������̨��
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];	//��ȡ����
	result->num = num;
	result->index = initiative_stand_min_not_using_index();
	if(result->index < 0)
		return;

	for(i = 0; i < num; i++)
	{
		memcpy((unsigned char *)(result->amm + i), (rvframe3761->Frame376_1App.AppBuf + in_index), AMM_ADDR_LEN);
		in_index += AMM_ADDR_LEN;
		//û���·�����̨��ʱ��������ͬ��������̨�ˣ����·�����̨��ʱ�������ѱ������±���̨��·��
		seek_amm_synchroniza((unsigned char *)(result->amm + i), result->ter);
	}

	if(0 == add_seek_amm_result(result))
	{
		write_seek_amm_result(result);
	}
}

/*
 * ��������:��չ
 * */
void DL3761_AFN17_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
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
		case 1:
			DL3761_AFN17_01(rvframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
