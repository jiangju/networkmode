/*
 * DL376_2_DataType.h
 *
 *  Created on: 2016��6��21��
 *      Author: Administrator
 */

#ifndef ST376_2_DL376_2_DATATYPE_H_
#define ST376_2_DL376_2_DATATYPE_H_

#include <stdbool.h>

#define	USER_DATA_LEN 	5000	//�û�������󳤶�
#define	ADDR_LEN		6		//��ַ����
#define	DT_LEN			2		//���ݵ�Ԫ��ʶ����

#define MAX_RELAY		15		//�м����

#define BEGINCHAR		0x68	//��ʼ�ַ�
#define ENDCHAR			0x16	//�����ַ�

#define	FRMLEN_EMPTY2	12		//��֡�ĳ���

enum
{
	BEGIN_LEN = 1,	//��ʼ�ַ�����
	END_LEN = 1,	//�����ַ�����
	L_LEN = 2,		//���ݳ��ȳ���
	CTL_LEN = 1,	//�����򳤶�
	CS_LEN = 1,		//CSУ���볤��
	R_LEN = 6,		//��Ϣ�򳤶�
	AFN_LEN = 1,	//�����볤��
};

typedef enum
{
	FRMPRM_PRM_SLAVE	= 0,	//�Ӷ�վ
	FRMPRM_PRM_PRIMARY  = 1		//����վ
}enFRMPRM;						//����վ�Ӷ�վ����ö��

typedef enum
{
	AFN_AFFI		= 0x00,	//ȷ��֡
	AFN_REST		= 0x01,	//��λ����
	AFN_DATASEND	= 0x02,	//����ת��
	AFN_QUERDATA	= 0x03,	//��ѯ����
	AFN_PORTDETE	= 0x04,	//��·�ӿڼ��
	AFN_CTRL		= 0x05,	//��������
	AFN_ACCORD		= 0x06,	//�����ϱ�

	AFN_ROUTEQUERY	= 0x10,	//·�ɲ�ѯ
	AFN_ROUTESET	= 0x11,	//·������
	AFN_ROUTECTRL	= 0x12,	//·�ɿ���
	AFN_ROUTESEND	= 0x13,	//·������ת��
	AFN_ROUTERCV	= 0x14, //·�����ݳ���
	AFN_FILETRAN    = 0x15,	//�ļ�����
}enAFN3762;						//AFN����

typedef struct
{
	unsigned char DIR;			//���䷽��(0 ���б��� 1 ���б���)
	unsigned char PRM;			//������־(1 ����վ     0 �Ӷ�վ)
	unsigned char CommMode;		//ͨѶ��ʽ   0 ����
								//1 ����ʽ·���ز�
								//2 �ֲ�ʽ·���ز�
								//3 ����ز�
								//4~9 ����
								//10 ΢��������ͨѶ
								//11~19 ����
								//20 ��̫��
								//21~63����
}tpCtlField2;					//376.2֡�п�����ṹ

typedef struct
{
	unsigned char 	BeginChar;	//��ʼ�ַ� 0x68
	unsigned short 	Len;		//���� ���û����ݳ���+6�� ������65535
	tpCtlField2		CtlField;	//������
	unsigned char 	CS;			//У���
	unsigned char 	Endchar;	//�����ַ�0x16
}tpFrame376_2Link;				//376.2������·��

typedef struct
{
	unsigned char	RelayRank;				//�м̼���(ֻ���ø�4λ)
	unsigned char	ConflictDetection;		//��ͻ��� 0/1
	unsigned char 	CommModeIdentifying;	//ͨѶģ���ʶ0/1
	unsigned char	AffiliateIdentifying;	//�����ڵ��ʶ0/1
	unsigned char 	RouteIdentifying;		//·�ɱ�ʶ
	unsigned char 	ErrEecoveryIdentifying;	//��������ʶ(ֻ�ø�4λ)
	unsigned char   ChannelIdentifying;		//�ŵ���ʶ(ֻ�õ�4λ)

	unsigned char 	PredictAnswerLen;		//Ԥ��Ӧ���ֽ���,���и�ֵ0x11

	unsigned char	SpeedIdentifying;		//�ٶȵ�λ��ʶ
	unsigned short	CommSpeed;				//ͨѶ����(���ʵ�λ��ʶ��ͨѶ�������2�ֽڳ��ȣ�����ʱ��ֵ0x99990000)

	unsigned char	MessageSerialNumber;	//�������к�
}tpR;										//��Ϣ��R

typedef	struct
{
	unsigned char 	SourceAddr[ADDR_LEN];				//Դ��ַ
	unsigned char	RelayAddr[ADDR_LEN *MAX_RELAY];		//�м̵�ַ
	unsigned char	DestinationAddr[ADDR_LEN];			//Ŀ�ĵ�ַ
}tpAddr;												//��ַ��

typedef struct
{
	unsigned char	AFN;			//������
	unsigned short  Len;			//�û����ݳ���
	unsigned char	Buffer[USER_DATA_LEN];	//�û�����
}tpAppData;

typedef struct
{
	tpR		R;			//��Ϣ��R
	tpAddr	Addr;		//��ַ��
	tpAppData	AppData;//�û�����
}tpFrame376_2App;				//376.2�û����ݲ�

typedef struct
{
	tpFrame376_2Link	Frame376_2Link;
	tpFrame376_2App 	Frame376_2App;
	bool				IsHaving;
}tpFrame376_2;		//376_2���ݽṹ

#endif /* ST376_2_DL376_2_DATATYPE_H_ */
