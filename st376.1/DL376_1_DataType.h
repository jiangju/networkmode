/*
 * DL376_1_DataType.h
 *
 *  Created on: 2016��7��14��
 *      Author: j
 */

#ifndef ST376_1_DL376_1_DATATYPE_H_
#define ST376_1_DL376_1_DATATYPE_H_

#include <stdbool.h>
#define FRMBUFF_LEN 2048

typedef enum
{
	FRMLEN_AFN   = 1, 	//������ĳ���
	FRMLEN_SEQ   = 1, 	//֡������ĳ���
	FRMLEN_EC    = 2, 	//EC�ĳ���
	FRMLEN_TP    = 6, 	//ʱ���ǩTp�ĳ���
	FRMLEN_PW    = 16,	//PW�ĳ���
	FRMLEN_DA    = 2, 	//���ݱ�ʶ��Ԫ__��Ϣ��ĳ���
	FRMLEN_DT    = 2, 	//���ݱ�ʶ��Ԫ__��Ϣ��ĳ���
	FRMLEN_HEAD  = 6,	//֡ͷ�ĳ���
	FRMLEN_CON   = 1,	//������ĳ���
	FRMLEN_ADD   = 5,	//��ַ��ĳ���
	FRMLEN_CS    = 1,	//У��͵ĳ���
	FRMLEN_END   = 1,	//֡β�ĳ���
	FRMLEN_EMPTY1 = 16,	//FRMLEN_HEAD+FRMLEN_CON+FRMLEN_ADD+FRMLEN_CS+FRMLEN_END	//��֡�ĳ���
}enFRMLEN;				//֡����ö��

typedef enum
{
	AFN3761_AFFI		= 0x00,	//ȷ��֡
	AFN3761_REST		= 0x01,	//��λ����
	AFN3761_INTFA		= 0x02,	//��·�ӿڼ��
	AFN3761_RELAY		= 0x03,	//�м̼��
	AFN3761_SETPARA		= 0x04,	//��������
	AFN3761_CTRL		= 0x05,	//��������
	AFN3761_CAPASS		= 0x06,	//�����֤������Э��
	AFN3761_TERMCONFIG	= 0x09,	//�����ն�����
	AFN3761_QURPARA		= 0x0a,	//������ѯ
	AFN3761_REPORT		= 0x0b,	//��ʱ�ϱ�
	AFN3761_QUR1CLASS	= 0x0c,	//����1������
	AFN3761_QUR2CLASS	= 0x0d,	//����2������
	AFN3761_QUR3CLASS	= 0x0e,	//����3������
	AFN3761_FILETRA		= 0x0f,	//�ļ�����
	AFN3761_DATASEND	= 0x10,	//����ת��
	AFN3761_EXTEND12	= 0x12,	//��չAFN=12
	AFN3761_EXTEND13	= 0x13,	//��չAFN=13
	AFN3761_EXTEND15	= 0x15,	//��չAFN=15
	AFN3761_EXTEND16	= 0x16,	//��չAFN=16
}enAFN3761;					   		//AFN����

typedef struct
{
	unsigned char DIR;			//DIR=0���б���  DIR=1���б���
	unsigned char PRM;			//PRM=0�������ԴӶ�վ  PRM=1������������վ

	//���б�����Ч
	unsigned char FCV;			//1-FCB��Ч  0-FCB��Ч
	unsigned char FCB;

	//���б�����Ч
	unsigned char ACD;			//1-������Ҫ�¼�  0-��������Ҫ�¼�

	//������
	unsigned char FUNC_CODE;
}tpCtlField1;	//376.1������

typedef struct
{
	unsigned char  WardCode[2];		//BCD��������
	unsigned char  Addr[2];			//BIN�ն˵�ַ
	unsigned char  MSA;				//BIN��վ��ַ�����ַ��־
}tpAddrField;	//376.1��ַ��

typedef struct
{
	unsigned char 	BeginChar0;	//��ʼ�ַ� 0x68
	unsigned short	L1;			//����1
	unsigned short	L2;			//����2
	unsigned char 	BeginChar1;	//��ʼ�ַ� 0x68
	tpCtlField1		CtlField;	//������
	tpAddrField		AddrField;	//��ַ��

	unsigned char	CS;			//Ч���
	unsigned char	EndChar;	//�����ַ�( 0x16 )

}tpFrame376_1Link;

typedef enum
{
	FRM_FIRFIN_MID		= 0x00,	//��֡/�м�֡
	FRM_FIRFIN_END		= 0x20,	//��֡/����֡
	FRM_FIRFIN_BEGIN	= 0x40,	//��֡/��1֡��������֡
	FRM_FIRFIN_SING		= 0x60,	//��֡
}enFRM_FIRFIN;					//֡���Ͷ���

typedef struct
{
	unsigned char TPV;					//0-��ʱ��   1-ʱ����Ч
	enFRM_FIRFIN  FIR_FIN;				//֡����
	unsigned char CON;					//0-����Ҫ�Ը�֡����ȷ��, 1-��Ҫ�Ը�֡����ȷ��
	unsigned char PSEQ_RSEQ;			//����/��Ӧ֡���
}tpSEQ;

typedef struct
{
	unsigned char  PW[FRMLEN_PW];	//����--16�ֽ�
	unsigned char  flag;			//0-��Ч 1-��Ч
}tpAUXPW;							//����

typedef struct
{
	unsigned char EC[2];//�¼�������
	unsigned char flag;	//0-��Ч 1-��Ч
}tpAUXEC;				//EC

typedef struct
{
	unsigned char	PFC;		//BIN����֡֡��ż�����
	unsigned char	TPFlag[4];	//����֡����ʱ��(��/��/ʱ/��)
	unsigned char	RTS;		//BIN�����ʹ�����ʱʱ��min
	unsigned char	flag;		//0-��Ч 1-��Ч
}stAUXTP;						//ʱ��

typedef struct
{
	tpAUXPW AUXPW;//����
	tpAUXEC AUXEC;//EC
	stAUXTP AUXTP;//ʱ��
}tpAUX;			  //������Ϣ��----AUX

typedef struct
{
	unsigned char	AFN;					//������
	tpSEQ			SEQ;					//SEQ֡������

	unsigned short	Len;					//Ӧ�ò�֡��---������AFN/SEQ
	unsigned char	AppBuf[FRMBUFF_LEN];	//���ݻ���

	tpAUX	AUX;							//������
}tpFrame376_1App;							//376.1Ӧ�ò�

typedef struct
{
	tpFrame376_1Link	Frame376_1Link;
	tpFrame376_1App		Frame376_1App;
	bool				IsHaving;
}tpFrame376_1;	//376.1���ݽṹ

#endif /* ST376_1_DL376_1_DATATYPE_H_ */
