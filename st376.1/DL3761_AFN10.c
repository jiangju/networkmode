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

	StandNode *p;
	struct topup_node *pp;
//	struct task_b *b_p;
//	struct task_e *e_p;
	unsigned char ter[4] = {0};
	int ret = 0;
	int i = 3;
	int fd;
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
				return;
			//�Ƚ�̨���е���Ӧ���ն˵�ַ������ն˵�ַ��һ��������ն˵�ַ
			ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

			//�ն˵�ַ��̨���е��ն˵�ַ��ͬ
			if(1 != CompareUcharArray(_SortNode[ret]->Ter, ter, TER_ADDR_LEN))
			{
				//���ļ�
				while(i--)
				{
					fd = open(STAND_BOOK_FILE, O_RDWR);
					if(fd >=0 )
						break;
				}
				if(fd >= 0)
				{
					memcpy(_SortNode[ret]->Ter, ter, TER_ADDR_LEN);
					AlterNodeStandFile(fd, _SortNode[ret]);
					close(fd);
				}
			}

			//�Ƚϱ��
			if(1 == CompareUcharArray(_Collect.amm, frame645.Address, AMM_ADDR_LEN))
			{
				p = _SortNode[ret];
				if(0x02 == p->type)
				{
					if((frame645.CtlField & 0x40) != 0)	//�쳣Ӧ��֡
					{
						is_true = 0;
					}
					else if(1 == CompareUcharArray(_Collect.dadt, frame645.Datas, 4))
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
					else if(1 == CompareUcharArray(_Collect.dadt, frame645.Datas, 2))	//����֡  �Ƚ����ݱ�ʶ
					{
						is_true = 0;
					}
				}

				if(0 == is_true)
				{
					switch(_Collect.runingtask)
					{
						case 'a':
							Create3761AFN10_01(_Collect.taska.next->ter, buf, len,  0, &snframe3761);
							//��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
							DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
							pthread_mutex_lock(&(topup_node_mutex));
							pp = AccordTerFind(_Collect.taska.next->ter);
							if(p != NULL)
							{
								pthread_mutex_lock(&(pp->write_mutex));
								while(1)
								{
									ret = write(pp->s, ttpbuffer.Data, ttpbuffer.Len);
									if(ret < 0)
									{
										break;
									}
									ttpbuffer.Len -= ret;
									if(0 == ttpbuffer.Len)
										break;
								}
								pthread_mutex_unlock(&(pp->write_mutex));
							}
							else
							{
								pthread_mutex_unlock(&(topup_node_mutex));
								break;
							}
							pthread_mutex_unlock(&(topup_node_mutex));
							_Collect.a_isend = 1;
							memset(_Collect.amm, 0, AMM_ADDR_LEN);
							memset(_Collect.dadt, 0, DADT_LEN);
							_Collect.conut = 0;
//								_Collect.flag = 0;
							_Collect.runingtask = 0;
							pthread_mutex_lock(&_Collect.taska.taska_mutex);
							DeleNearTaskA();
							pthread_mutex_unlock(&_Collect.taska.taska_mutex);
							break;
						case 'b':
							//����376.2 AFN13�ϱ�����
							Create3762AFN13_01(_Collect.amm, _Collect.taskb.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								_Collect.b_isend = 1;
								memset(_Collect.amm, 0, AMM_ADDR_LEN);
								memset(_Collect.dadt, 0, DADT_LEN);
								_Collect.conut = 0;
//								_Collect.flag = 0;
								_Collect.runingtask = 0;
								pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
								DeleNearTaskB();
								pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
//								//��ȡ��һ������ɾ����ǰ����
//								_Collect.taskb.num--;
//								b_p = _Collect.taskb.next;
//								_Collect.taskb.next = b_p->next;
//								//ɾ����һ������
//								if(b_p != NULL)
//								{
//									free(b_p);
//									b_p = NULL;
//								}
							}
							break;
						case 'c':
							//���� 376.2 AFN06�ϱ�����
							Create3762AFN06_02((unsigned short)ret, p->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								_Collect.c_isend = 1;
								memset(_Collect.amm, 0, AMM_ADDR_LEN);
								memset(_Collect.dadt, 0, DADT_LEN);
								_Collect.conut = 0;
//								_Collect.flag = 0;
								_Collect.taskc.isok = 0;
								_Collect.runingtask = 0;
							}
							break;
						case 'd':
							break;
						case 'e':
							//����376.2 AFN02�ϱ�����
							Create3762AFN02_01(_Collect.amm, _Collect.taske.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								_Collect.e_isend = 1;
								memset(_Collect.amm, 0, AMM_ADDR_LEN);
								memset(_Collect.dadt, 0, DADT_LEN);
								_Collect.conut = 0;
//								_Collect.flag = 0;
								_Collect.runingtask = 0;
								pthread_mutex_lock(&_Collect.taske.taske_mutex);
								DeleNearTaskE();
								pthread_mutex_unlock(&_Collect.taske.taske_mutex);
							}
							break;
					}
				}
			}
		}
	}
	else												//����վ����ֵ�նˣ�
	{
		struct task_a task;
		memset(&task, 0, sizeof(struct task_a));
		//ǰ4�ֽڲ�����
		index += 4;
		//͸��ת�������ֽ���
		len = rvframe3761->Frame376_1App.AppBuf[index + 1] * 256 + rvframe3761->Frame376_1App.AppBuf[index];
		index += 2;
		memcpy(buf, (rvframe3761->Frame376_1App.AppBuf + index), len);
		if(0 == ProtoAnaly645BufFromCycBuf(buf, len, &frame645))
		{
			memcpy(task.amm, frame645.Address, AMM_ADDR_LEN);
			memcpy(task.dadt, frame645.Datas, 4);
			task.next = NULL;
			memcpy(task.buf, buf, len);
			task.len = len;
			task.ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			task.ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			task.ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			task.ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];
			pthread_mutex_lock(&_Collect.taska.taska_mutex);
			AddTaskA(&task);
			pthread_mutex_unlock(&_Collect.taska.taska_mutex);
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
}
