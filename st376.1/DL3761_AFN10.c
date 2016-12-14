/*
 * DL3761_AFN10.c
 *
 *  Created on: 2016��7��25��
 *      Author: j
 */
#include "DL376_1.h"
#include <stdio.h>
#include <string.h>
#include "SysPara.h"
#include <pthread.h>
#include "DL3762_AFN06.h"
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "DL376_2.h"
#include "StandBook.h"
#include "Collect.h"
#include "DL645.h"
#include "HLDUsart.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "NetWork1.h"
#include <stdlib.h>
#include "DL3762_AFN02.h"
#include "DL3762_AFN13.h"
#include "TopBuf.h"
#include "TopRoute.h"
#include "hld_ac.h"

//#include <sys/time.h>
/*
 * ��������:����͸��ת����ʽ֡
 * ����:	ter		�ն˵�ַ
 * 		inbuf	��ת��������
 * 		len		��ת�������ݳ���
 * 		outbuf	����ĸ�ʽ֡
 * 		flag	�Ӷ�վ/����վ
 * */
void Create3761AFN10_01(unsigned char *ter, unsigned char *inbuf, int len,  unsigned char flag, tpFrame376_1 *outbuf)
{
	int index = 0;
	int i = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));
	/************************************��·��************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//������
	outbuf->Frame376_1Link.EndChar = 0x16;
	//������
	if(flag == 0)
		outbuf->Frame376_1Link.CtlField.DIR = 1;	//����
	else
		outbuf->Frame376_1Link.CtlField.DIR = 0;	//����

	outbuf->Frame376_1Link.CtlField.PRM = flag;	//�Ӷ�վ
	outbuf->Frame376_1Link.CtlField.FCV = 0;	//FCBλ��Ч
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	if(flag == 0)
		outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x08;	//������
	else
		outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//��ַ��
	outbuf->Frame376_1Link.AddrField.WardCode[0] = ter[0];
	outbuf->Frame376_1Link.AddrField.WardCode[1] = ter[1];
	outbuf->Frame376_1Link.AddrField.Addr[0] = ter[2];
	outbuf->Frame376_1Link.AddrField.Addr[1] = ter[3];
	if(flag == 0)
		outbuf->Frame376_1Link.AddrField.MSA = 0;
	else
		outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************Ӧ�ò�****************************************/
	//������
	outbuf->Frame376_1App.AFN = AFN3761_DATASEND;
	//����ȷ�ϱ�־
	outbuf->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//ʱ���ǩ
	outbuf->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//���ݱ�ʶ
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;

	if(flag == 0)	//�Ӷ�վ(��Ӧ��ֵ�ն�)
	{
		//ͨѶ�˿ں�
		outbuf->Frame376_1App.AppBuf[index++] = 0x01;
		//͸��ת�������ֽ���
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)((unsigned int)len & 0x000000FF);
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)(((unsigned int)len & 0x0000FF00) >> 8);
		//͸��ת������
		for(i = 0; i < len; i++)
		{
			outbuf->Frame376_1App.AppBuf[index++] = inbuf[i];
		}
	}
	else			//����վ(���͸�������һģ��ɼ�����)
	{
		//�ն�ͨ�Ŷ˿ں�
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//͸��ת��ͨ�ſ�����
		outbuf->Frame376_1App.AppBuf[index++] = 0x6B;
		//͸��ת�����յȴ����ĳ�ʱʱ��
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//͸��ת�����յȴ��ֽڳ�ʱʱ��
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//͸��ת�������ֽ���
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)((unsigned int)len & 0x000000FF);
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)(((unsigned int)len & 0x0000FF00) >> 8);
		//͸��ת������
		for(i = 0; i < len; i++)
		{
			outbuf->Frame376_1App.AppBuf[index++] = inbuf[i];
		}
	}
	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * ��������:�㲥���ݸ���ֵ�ն�
 * ����:	inbuf	����
 * 		len		����
 * ����ֵ: 0 �ɹ�  -1  ʧ��
 * */
int broadcast_buf_topup(unsigned char *inbuf, int len)
{
	if(0 >= get_hld_top_node_num())
	{
		return -1;
	}

	if(0 != _hld_ac.get_status())	//δ��Ȩʱ�����������ݳ���
	{
		return 0;
	}
	unsigned char ter[TER_ADDR_LEN] = {0};
	tpFrame376_1 outbuf;
	tp3761Buffer snbuf;
	int i = 1;
	while(0 == get_hld_top_node_ter(i, ter))
	{
		usleep(1);
		i++;
		//����͸��ת��֡�ṹ
		Create3761AFN10_01(ter, inbuf, len, 0, &outbuf);
		//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
		DL3761_Protocol_LinkPack(&outbuf, &snbuf);
		send_hld_top_node_ter_data(ter, snbuf.Data, snbuf.Len);
	}

	return 0;
}

///*
// * ��������:�ж��Ƿ��ǳ�ֵ�ն���Ҫ�����ݱ�ʶ
// * ����:	dadt	���ݱ�ʶ
// * ����ֵ: 0 ��Ҫ  -1  ����Ҫ
// * */
//int top_jude_dadt(unsigned char *dadt)
//{
//	int i = 0;
//	for(i = 0; i < TOP_FIX_DADT_NUM; i++)
//	{
//		if(1 == CompareUcharArray((unsigned char *)_top_fix_dadt[i], dadt, DADT_LEN))
//		{
//			return 0;
//		}
//	}
//
//	return -1;
//}

/*
 * ��������:͸��ת��
 * */
void DL3761_AFN10_01(tpFrame376_1 *rvframe3761)
{
	int len = 0;
	int index = 4;	//rvframe3761->Frame376_1App.AppBufǰ4���ֽ������ݱ�ʶ
	unsigned char buf[300] = {0};
	tpFrame376_2 snframe3762;
	tp3762Buffer tpbuffer;
	tpFrame645 frame645;
	tpFrame376_1 snframe3761;
	tp3761Buffer ttpbuffer;

	CollcetStatus c_status;
	StandNode node;
	unsigned char ter[4] = {0};
	int ret = 0;
	int is_true = -1;
	//�ж����Գ�ֵ�ն˻��ǵ���������һ
	if(rvframe3761->Frame376_1Link.CtlField.PRM == 0)	//�Ӷ�վ�����
	{
		index++;
		len = rvframe3761->Frame376_1App.AppBuf[index + 1] * 256 + rvframe3761->Frame376_1App.AppBuf[index];
		index += 2;

		memcpy(buf, (rvframe3761->Frame376_1App.AppBuf + index), len);
		if(0 == ProtoAnaly645BufFromCycBuf(buf, len, &frame645))
		{
			//�鿴�ڴ��е�̨���Ƿ��иõ��
			ret = SeekAmmAddr(frame645.Address, AMM_ADDR_LEN);
			if(ret < 0)
			{
				return;
			}

			//���ĵ�����ͨ��ʱ��
			UpdateNodeDate(ret);

			//��ȡ�������
			if(0 > GetStandNode(ret, &node))
			{
				return;
			}
			//�Ƚ�̨���е���Ӧ���ն˵�ַ������ն˵�ַ��һ��������ն˵�ַ
			ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

			//�ն˵�ַ��̨���е��ն˵�ַ��ͬ
			if(1 != CompareUcharArray(node.Ter, ter, TER_ADDR_LEN))
			{
				memcpy(node.Ter, ter, TER_ADDR_LEN);
				UpdateStandNode(ret, &node);
				AlterNodeStandFile(&node);
			}

			//�жϳ�ֵ����
			if(1 == GetTaskaKey())
			{
				//���ݱ�Ż�ȡ�ڵ�
				struct task_a_node *node_p = SeekTaskANode1(frame645.Address);
				if(NULL == node_p)
					return;
				if(1 == CompareUcharArray(node_p->task_status.dadt, frame645.Datas, 4) || (frame645.CtlField & 0x40) != 0)
				{
					Create3761AFN10_01(node_p->task_status.top_ter, buf, len,  0, &snframe3761);
				}
				else
				{
					return;
				}
				//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
				DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
				send_hld_top_node_ter_data(node_p->task_status.top_ter, ttpbuffer.Data, ttpbuffer.Len);
//				struct timeval tv;
//				gettimeofday(&tv, NULL);
//				printf("send top up :  %ld;  ms:  %ld\n",tv.tv_sec, (tv.tv_usec / 1000));
				//�������
				DeleTaskA(node_p->amm, 0);
				return;
			}

			GetCollectorStatus(&c_status);
			//�Ƚϱ��  ʵʱ����  �������� ��Ҫ�Ƚϱ��
			if(1 == CompareUcharArray(c_status.amm, frame645.Address, AMM_ADDR_LEN))
			{
				if(0x02 == node.type)
				{
					if((frame645.CtlField & 0x40) != 0)	//�쳣Ӧ��֡
					{
						is_true = 0;
					}
					else if(1 == CompareUcharArray(c_status.dadt, frame645.Datas, 4))
					{
						is_true = 0;
					}
				}
				else
				{
					if((frame645.CtlField & 0x40) != 0)	//�쳣Ӧ��֡
					{
						is_true = 0;
					}
					else if(1 == CompareUcharArray(c_status.dadt, frame645.Datas, 2))	//����֡  �Ƚ����ݱ�ʶ
					{
						is_true = 0;
					}
				}

				if(0 == is_true)
				{
					switch(c_status.runingtask)
					{
						case 'a':

							break;
						case 'b':
							//����376.2 AFN13�ϱ�����
							Create3762AFN13_01(c_status.amm, _Collect.taskb.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);

								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
//								pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
								DeleNearTaskB();
//								pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
							}
							break;
						case 'c':
							//���� 376.2 AFN06�ϱ�����
							Create3762AFN06_02((unsigned short)ret, node.type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
//								_Collect.flag = 0;
//								_Collect.taskc.isok = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
							}
							break;
						case 'd':
							break;
						case 'e':
							//����376.2 AFN02�ϱ�����
							Create3762AFN02_01(c_status.amm, _Collect.taske.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
//								_Collect.flag = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
//								pthread_mutex_lock(&_Collect.taske.taske_mutex);
								DeleNearTaskE();
//								pthread_mutex_unlock(&_Collect.taske.taske_mutex);
							}
							break;
					}

//					//���յ���ֵ�ն���Ҫ������ʱ���㲥���������ߵĳ�ֵ�ն�
//					if(0 == top_jude_dadt(c_status.dadt))
//					{
//						broadcast_buf_topup(buf, len);
//					}
				}
			}
		}
	}
	else												//����վ����ֵ�նˣ�
	{
		unsigned char top_ter[TER_ADDR_LEN] = {0};
		//ǰ4�ֽڲ�����
		index += 4;
		//͸��ת�������ֽ���
		len = rvframe3761->Frame376_1App.AppBuf[index + 1] * 256 + rvframe3761->Frame376_1App.AppBuf[index];
		index += 2;
		memcpy(buf, (rvframe3761->Frame376_1App.AppBuf + index), len);
		if(0 == ProtoAnaly645BufFromCycBuf(buf, len, &frame645))
		{
			top_ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			top_ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			top_ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			top_ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

			ret = AddTaskA(frame645.Address, frame645.Datas, buf, len, top_ter);
			if(0 != ret)
			{
				tpFrame645 f645;
				Buff645 b645;

				memcpy(f645.Address, frame645.Address, AMM_ADDR_LEN);
				f645.CtlField = 0xD1;
				printf("add top up task err %d\n", ret);
				switch (ret)
				{
					case -1:
						f645.Datas[0] = 0xFD;
						break;
					case -2:
						f645.Datas[0] = 0xFF;
						break;
					case -3:
						f645.Datas[0] = 0xFC;
						break;
					default:
						break;
				}
				f645.Datas[0] += 0x33;
				f645.Length = 1;

				if(0 == Create645From(&f645, &b645))
				{
					Create3761AFN10_01(top_ter, b645.buf, b645.len, 0, &snframe3761);
					//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
					DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
					send_hld_top_node_ter_data(top_ter, ttpbuffer.Data, ttpbuffer.Len);
				}
			}
		}
	}
}

/*
 * ��������:����ת��(���ܳ��յ�������)
 * ����:	rvframe3761	���ܵ�����
 * */
void DL3761_AFN10_Analy(tpFrame376_1 *rvframe3761)
{
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);
	switch (Fn)
	{
		case 1:
			DL3761_AFN10_01(rvframe3761);
			break;
		case 9:
			break;
		case 12:
			break;
		case 13:
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
