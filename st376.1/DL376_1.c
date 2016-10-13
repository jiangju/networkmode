#define _DL3761_C_
#include "DL376_1.h"
#undef	_DL3761_C_
#include <stdbool.h>
#include "DL376_1_DataType.h"
#include <stdio.h>
#include <string.h>
#include "Response_AFN02.h"
#include "Response_AFN0E.h"
#include "DL3761_AFN10.h"
#include "DL3761_AFN15.h"
#include "DL3761_AFN16.h"

/*
 *��������:�����ܵ�376.1����ת��Ϊ�����376.1��ʽ
 *����:	inBuffer		��ת�������ݻ���
 *		tpFrame376_1	ת�������ݵĴ洢����
 *����ֵ:��
 **/
void DL376_1_LinkFrame(tp3761Buffer *inBuffer, tpFrame376_1 *frame3761)
{
	memset(frame3761, 0, sizeof(tpFrame376_1));

	unsigned short i,j;

	if ((inBuffer->Len <= FRMLEN_EMPTY1))
	{
		return;//С��֡��С����
	}

	//-----------------------------------------------------------
	//��·��
	frame3761->Frame376_1Link.BeginChar0 = inBuffer->Data[0];
	if(frame3761->Frame376_1Link.BeginChar0 != 0x68)
	{
		return;
	}

	frame3761->Frame376_1Link.L1 = (inBuffer->Data[1]+(inBuffer->Data[2]*256))>>2;
	frame3761->Frame376_1Link.L2 = (inBuffer->Data[3]+(inBuffer->Data[4]*256))>>2;
	if(frame3761->Frame376_1Link.L1 != frame3761->Frame376_1Link.L2)
	{
		return;
	}

	frame3761->Frame376_1Link.BeginChar1 = inBuffer->Data[5];
	if(frame3761->Frame376_1Link.BeginChar1 != 0x68)
	{
		return;
	}

	//������
	frame3761->Frame376_1Link.CtlField.DIR = ((inBuffer->Data[6]&0x80)>>7);
	frame3761->Frame376_1Link.CtlField.PRM = ((inBuffer->Data[6]&0x40)>>6);
	if(frame3761->Frame376_1Link.CtlField.DIR == 0)//����
	{
		frame3761->Frame376_1Link.CtlField.FCB = ((inBuffer->Data[6]&0x20)>>5);
		frame3761->Frame376_1Link.CtlField.FCV = ((inBuffer->Data[6]&0x20)>>4);
	}
	else if(frame3761->Frame376_1Link.CtlField.DIR == 1)//����
	{
		frame3761->Frame376_1Link.CtlField.ACD = ((inBuffer->Data[6]&0x20)>>5);
	}
	frame3761->Frame376_1Link.CtlField.FUNC_CODE = (inBuffer->Data[6]&0x0f);

	frame3761->Frame376_1Link.AddrField.WardCode[0] = inBuffer->Data[7];//buf[7]+(buf[8]*256);
	frame3761->Frame376_1Link.AddrField.WardCode[1] = inBuffer->Data[8];
	frame3761->Frame376_1Link.AddrField.Addr[0]		= inBuffer->Data[9];//buf[9]+(buf[10]*256);
	frame3761->Frame376_1Link.AddrField.Addr[1]		= inBuffer->Data[10];
	frame3761->Frame376_1Link.AddrField.MSA			= inBuffer->Data[11];

	//У�顢�����ַ�
	frame3761->Frame376_1Link.CS = inBuffer->Data[inBuffer->Len-2];
	frame3761->Frame376_1Link.EndChar = inBuffer->Data[inBuffer->Len-1];
	if(frame3761->Frame376_1Link.EndChar != 0x16)
	{
		return;
	}

	//-----------------------------------------------------------
	//Ӧ�ò�
	frame3761->Frame376_1App.AFN = inBuffer->Data[12];
	frame3761->Frame376_1App.SEQ.TPV		= ((inBuffer->Data[13]&0x80)>>7);
	frame3761->Frame376_1App.SEQ.FIR_FIN	= ((inBuffer->Data[13]&0x60)>>5);
	frame3761->Frame376_1App.SEQ.CON		= ((inBuffer->Data[13]&0x10)>>4);
	frame3761->Frame376_1App.SEQ.PSEQ_RSEQ	= (inBuffer->Data[13]&0x0f);
	frame3761->Frame376_1App.Len = (frame3761->Frame376_1Link.L1)-FRMLEN_CON-FRMLEN_ADD-FRMLEN_AFN-FRMLEN_SEQ;//L1-C-A-AFN-SEQ

	//���ݱ�ʶ+���ݵ�Ԫ+������PW��+ʱ��TP(������AFN+SEQ)
	for (j=0,i=14;j<(frame3761->Frame376_1App.Len);j++,i++)
	{
		frame3761->Frame376_1App.AppBuf[j] = inBuffer->Data[i];
	}

	//����
	if(frame3761->Frame376_1App.Len > 8)
	{
		if((frame3761->Frame376_1App.AFN		==	AFN3761_REST)		//��λ
			|| (frame3761->Frame376_1App.AFN	==	AFN3761_SETPARA)	//��������
			|| (frame3761->Frame376_1App.AFN	==	AFN3761_CTRL)		//��������
			|| (frame3761->Frame376_1App.AFN	==	AFN3761_CAPASS)		//������֤
			|| (frame3761->Frame376_1App.AFN	==	AFN3761_FILETRA)	//�ļ�����
			|| (frame3761->Frame376_1App.AFN	==	AFN3761_DATASEND))	//����ת��
		{
			frame3761->Frame376_1App.AUX.AUXPW.flag = 1;
			int i = 0;
			for(i=0; i<FRMLEN_PW; i++)
			{
				frame3761->Frame376_1App.AUX.AUXPW.PW[i] = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-6-(FRMLEN_PW - i)];
			}
		}
	}//if

	//ʱ��
	if(frame3761->Frame376_1App.Len > 6)
	{
		if(frame3761->Frame376_1App.SEQ.TPV == 1)
		{
			frame3761->Frame376_1App.AUX.AUXTP.flag		 = 1;
			frame3761->Frame376_1App.AUX.AUXTP.PFC		 = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-6];
			frame3761->Frame376_1App.AUX.AUXTP.TPFlag[0] = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-5];
			frame3761->Frame376_1App.AUX.AUXTP.TPFlag[1] = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-4];
			frame3761->Frame376_1App.AUX.AUXTP.TPFlag[2] = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-3];
			frame3761->Frame376_1App.AUX.AUXTP.TPFlag[3] = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-2];
			frame3761->Frame376_1App.AUX.AUXTP.RTS		 = frame3761->Frame376_1App.AppBuf[frame3761->Frame376_1App.Len-1];
		}
	}

	//��ʶ�յ�����֡
	frame3761->IsHaving = true;
}

/*
 * ��������:���������
 * ����:	CtlField ������
 * ����ֵ: �����õ���ֵ
 * */
unsigned char DL3761_Protocol_CtlFieldToChar(tpCtlField1 *CtlField)
{
	unsigned char cResult;

	cResult = 0;
	//---
	if(CtlField->DIR == 1)
	{
		cResult |= 0x80;
	}
	else
	{
		cResult &= ~(0x80);
	}

	//---
	if(CtlField->PRM == 1)
	{
		cResult |= 0x40;
	}
	else
	{
		cResult &= ~(0x40);
	}

	//���б���
	if(CtlField->DIR == 0)
	{
		if(CtlField->FCV == 1)
		{
			cResult |= 0x10;

			if(CtlField->FCB == 1)
			{
				cResult |= 0x20;
			}
			else
			{
				cResult &= ~(0x20);
			}
		}
		else
		{
			cResult &= ~(0x10);
			cResult &= ~(0x20);
		}
	}

	//���б���
	if(CtlField->DIR == 1)
	{
		if(CtlField->ACD == 1)
		{
			cResult |= 0x20;
		}
		else
		{
			cResult &= ~(0x20);
		}
	}

	cResult += (CtlField->FUNC_CODE & 0x0f);

	return cResult;
}

/*
 * ��������:����seq
 * ����:seq	seq
 * ����ֵ:�����õ���ֵ
 * */
unsigned char DL3761_Protocol_SEQToChar(tpSEQ *SEQ)
{
	unsigned char cResult = 0;

	if(SEQ->TPV == 1)
	{
		cResult |= 0x80;
	}
	else
	{
		cResult &= ~(0x80);
	}

	cResult &= ~(FRM_FIRFIN_SING);
	cResult |= SEQ->FIR_FIN;
	if(SEQ->CON == 1)//1-��Ҫ�Ը�֡����ȷ��
	{
		cResult |= 0x10;
	}
	else//0-����Ҫ�Ը�֡����ȷ��
	{
		cResult &= ~(0x10);
	}
	cResult += (SEQ->PSEQ_RSEQ & 0x0f);
	return cResult;
}


/*
 * ��������:��376.1��ʽ����ת��Ϊ���Է��͵Ķ���������
 * ����:	snframe3761	��ת��������
 * 		outbuffer	ת���������
 * ����ֵ: -1 ʧ��  0�ɹ�
 * */
int DL3761_Protocol_LinkPack(tpFrame376_1 *snframe3761, tp3761Buffer *outbuffer)
{
	int index = 0;
	unsigned short Len = 0;
	unsigned char *pcTemp = NULL;
	unsigned char temp = 0;
	int j = 0;
	memset(outbuffer, 0, sizeof(tp3761Buffer));
	if(true == snframe3761->IsHaving)
	{
		//��ʼ��
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.BeginChar0;

		//L1 L2
		Len = snframe3761->Frame376_1App.Len;
		Len += FRMLEN_AFN;
		Len += FRMLEN_SEQ;
		Len += FRMLEN_CON;
		Len += FRMLEN_ADD;
		if(snframe3761->Frame376_1App.AUX.AUXEC.flag == 1)
		{
			Len += FRMLEN_EC;
		}
		if(snframe3761->Frame376_1App.AUX.AUXTP.flag == 1)
		{
			Len += FRMLEN_TP;
		}
		Len <<= 2;
		Len &= 0xfffc;
		Len |= 0x0002;
		pcTemp = (unsigned char*)&Len;
		outbuffer->Data[index++] = pcTemp[0];
		outbuffer->Data[index++] = pcTemp[1];
		outbuffer->Data[index++] = pcTemp[0];
		outbuffer->Data[index++] = pcTemp[1];

		//��ʼ��
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.BeginChar1;

		//������
		temp = DL3761_Protocol_CtlFieldToChar(&(snframe3761->Frame376_1Link.CtlField));
		outbuffer->Data[index++] = temp;

		//��ַ��
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.AddrField.WardCode[0];
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.AddrField.WardCode[1];
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.AddrField.Addr[0];
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.AddrField.Addr[1];
		outbuffer->Data[index++] = snframe3761->Frame376_1Link.AddrField.MSA;

		//������
		outbuffer->Data[index++] = snframe3761->Frame376_1App.AFN;

		//seq
		temp = DL3761_Protocol_SEQToChar(&(snframe3761->Frame376_1App.SEQ));
		outbuffer->Data[index++] = temp;

		//��·�û�����
		for (j=0;j<(snframe3761->Frame376_1App.Len);j++)
		{
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AppBuf[j];
		}

		//EC
		if(snframe3761->Frame376_1App.AUX.AUXEC.flag == 1)
		{
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXEC.EC[0];
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXEC.EC[1];
		}

		//TP
		if(snframe3761->Frame376_1App.AUX.AUXTP.flag == 1)
		{
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.PFC;
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.TPFlag[0];
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.TPFlag[1];
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.TPFlag[2];
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.TPFlag[3];
			outbuffer->Data[index++] = snframe3761->Frame376_1App.AUX.AUXTP.RTS;
		}

		//��β
		outbuffer->Data[index] = 0;
		for(j=6;j<index;j++)
		{
			outbuffer->Data[index] += outbuffer->Data[j];//CS
		}
		index++;

		outbuffer->Data[index++] = snframe3761->Frame376_1Link.EndChar;//������ʶ
		outbuffer->Len = index;

		//���������֡�ṹ
		memset((snframe3761), 0, sizeof(tpFrame376_1));
		return 0;
	}
	return -1;
}

/*
 * ��������:У��376.1���ܱ����Ƿ���ȷ
 * ����:�ھ�����376.1����
 * ����ֵ: 0 �ɹ� -1 ʧ��
 * */
int ProtoAnaly_Effic376_1Frame(tp3761Buffer *buf)
{
	unsigned short len1,len2;
	unsigned short i;
	unsigned char  CS;

	//֡����L1��L2
	if (((buf->Data[1]&0x03) == 0x01)  & ((buf->Data[3]&0x03) == 0x01))//��ʶ����Ч
	{
		len1 = buf->Data[1]+(buf->Data[2]*256);
		len2 = buf->Data[3]+(buf->Data[4]*256);
		len1 = (len1 >> 2) & 0x3fff;
		len2 = (len2 >> 2) & 0x3fff;

		if ((len1) != (len2))//L1!=L2
		{
			return -1;
		}
	}

	//Ч���CS
	i=6;
	CS=0;
	while(i<(buf->Len-2))
	{
		CS = CS + buf->Data[i];
		i++;
	}

	if (CS != buf->Data[buf->Len-2])
	{
		return -1;
	}

	//���ķ���
	//ControlField = buf->Data[6];
	//if ((ControlField & 0x80) != 0x80) return -1;//���б���

	return 0;
}

/*
 * ��������:�������Ļ����У�����Ƿ���������376.1֡
 * ����:	inbuf	�������Ļ���
 * 		maxlen	�������󳤶�
 * 		r		�����Ŀ�ʼλ��(������rֵ�践��)
 * 		w		�����Ľ���λ��+1(������wֵ�践��)
 * 		outbuf	�����ɹ�������
 * ����ֵ:0 �ɹ� -1 ʧ��
 * */
int ProtoAnaly_Get376_1BufFromCycBuf(unsigned char *inbuf, unsigned short maxlen,\
		unsigned short *r, unsigned short *w, tp3761Buffer *outbuf)
{
	unsigned short L1 = 0;
	unsigned short L2 = 0;
	unsigned short l = 0;
	unsigned short rr = *r;
	unsigned short ww = *w;
	int i = 0;
	if(rr == ww)
	{
		return -1;
	}
	memset(outbuf, 0x00, sizeof(tp3761Buffer));

	while(rr != ww)
	{
		if(inbuf[rr] != 0x68)
		{
			rr++;
			rr %= maxlen;
			continue;
		}
		else
		{
			(*r) = rr;
			if(ww > rr)
			{
				l = ww - rr;
			}
			else if(rr > ww)
			{
				l = ww + maxlen - rr;
			}
			else
			{
				*r = rr;
//				printf("a rr  %d\n", rr);
				return -1;
			}
			if(l < FRMLEN_EMPTY1)
			{

				return -1;
			}

			if(0x68 != inbuf[(rr+5) % maxlen])
			{
				(*r) += 1;
				(*r) %= maxlen;
				rr = (*r);
//				printf("b rr  %d\n", rr);
//				return -1;
				continue;
			}

			L1 = inbuf[(rr+1) % maxlen];
			L1 = 256 * inbuf[(rr+2) % maxlen] + L1;

			L2 = inbuf[(rr+3) % maxlen];
			L2 = 256 * inbuf[(rr+4) % maxlen] + L2;

			L1 = L1 >> 2;
			L1 = L1 & 0x3fff;
			L2 = L2 >> 2;
			L2 = L2 & 0x3fff;
			if(L1 != L2)
			{
				(*r) += 1;
				(*r) %= maxlen;
				rr = (*r);
//				printf("c rr  %d\n", rr);
//				return -1;
				continue;
			}

			if(0 == L1)
			{
				(*r) += 1;
				(*r) %= maxlen;
				rr = (*r);
//				printf("d rr  %d\n", rr);
//				return -1;
				continue;
			}

			//���ܵ����ݳ��ȴ��ڻ�������ݳ���
			if((L1 + 2 + 6) > maxlen)
			{
				(*r) = *w;
//				printf("e rr  %d\n", rr);
				return -1;
			}

			//�ѽ��ܵ�����С�ڽ�Ҫ���ܵ����ݳ���
			if((L1 + 2 + 6) > l)
			{
//				printf("376 rev need delay\n");
				return -1;
			}

			for(i = 0; i < (L1 + 2 + 6); i++)
			{
				outbuf->Data[i] = inbuf[rr];
				rr++;
				rr %= maxlen;
			}

			//�жϽ�β�Ƿ���0x16
			if(0x16 != outbuf->Data[L1 + 2 + 6 - 1])
			{
				(*r) += 1;
				(*r) %= maxlen;
				rr = (*r);
//				printf("f rr  %d\n", rr);
//				return -1;
				continue;
			}

			outbuf->Len = (L1 + 2 + 6);
			if(0 == ProtoAnaly_Effic376_1Frame(outbuf))
			{
				(*r) = rr;
				return 0;
			}
			(*r) = rr;
//			printf("g rr  %d\n", rr);
			return -1;
		}
	}

	(*r) = rr;
	(*r) = ww;
//	printf("no find bigen char\n");
	return -1;
}

/*
 * ��������:����376.1��Ӧ
 * */
void DL3761_Process_Response(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	//printf("rev 376.1 ok  AFN  %x\n",rvframe3761->Frame376_1App.AFN);
	switch(rvframe3761->Frame376_1App.AFN)
	{
		case AFN3761_AFFI:
			break;
		case AFN3761_INTFA:
			ResponseAFN02_Analy(rvframe3761, snframe3761);
			break;
		case AFN3761_RELAY:
			break;
		case AFN3761_SETPARA:
			break;
		case AFN3761_CAPASS:
			break;
		case AFN3761_TERMCONFIG:
			break;
		case AFN3761_QURPARA:
			break;
		case AFN3761_QUR1CLASS:
			break;
		case AFN3761_QUR2CLASS:
			break;
		case AFN3761_QUR3CLASS:
			ResponseAFN0E_Analy(rvframe3761, snframe3761);
			break;
		case AFN3761_FILETRA:
			break;
		case AFN3761_DATASEND:
			DL3761_AFN10_Analy(rvframe3761);
			break;
		case AFN3761_EXTEND12:
			break;
		case AFN3761_EXTEND13:
			break;
		case AFN3761_EXTEND15:
			DL3761_AFN15_Analy(rvframe3761, snframe3761);
			break;
		case AFN3761_EXTEND16:
			DL3761_AFN16_Analy(rvframe3761, snframe3761);
			break;
		default :
			memset(rvframe3761, 0, sizeof(tpFrame376_1));
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
