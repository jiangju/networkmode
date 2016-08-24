/*
 * SysPara.h
 *	ϵͳ�����ļ�
 *  Created on: 2016��6��29��
 *      Author: j
 */

#ifndef COMMONALITY_SYSPARA_H_
#define COMMONALITY_SYSPARA_H_

#define VENDOR_CODE0	'H'		//���̴���
#define VENDOR_CODE1	'H'		//���̴���

#define CHIP_CODE0		'H'		//оƬ����
#define	CHIP_CODE1		'H'		//оƬ����

#define	VERSION_DATE0	0x28	//�汾���� 	��
#define	VERSION_DATE1	0x06	//�汾����	��
#define	VERSION_DATE2	0x16	//�汾����	��

#define	VERSION_NUM0	0x01	//�汾��
#define	VERSION_NUM1	0x00	//�汾��

#define	CONFIG_FILE		"/opt/configuration"	//�����ļ���
#define	STAND_BOOK_FILE	"/opt/stand_book"	//̨���ļ�

#define	NODE_ADDR_LEN	6		//�ڵ��ַ����

#define TER_ADDR_LEN	4		//�ն˵�ַ����

#define DADT_LEN		4		//���ݱ�ʶ����

#define	AMM_ADDR_LEN	6//����ַ����
#define AMM_MAX_NUM		2040//����������

typedef struct
{
	unsigned char Amm[AMM_ADDR_LEN];	//����
	unsigned char type;					//ͨѶ��Լ����
	unsigned char Ter[TER_ADDR_LEN];	//�ն˵�ַ
	unsigned char cyFlag;				//�����־  (���������в����еĳ���ɹ��ı�־ʱ������ɹ�)

	unsigned char CS;					//CSУ��
}AmmAttribute;

typedef struct
{
	unsigned char HostNode[NODE_ADDR_LEN];
	unsigned char CS;
}tpAFN05_1;		//���ڵ��ַ

typedef struct
{
	unsigned char IsAppera;	//�Ƿ�����ӽڵ��¼��ϱ�		0��ֹ  1����
	unsigned char CS;
}tpAFN05_2;

typedef struct
{
	unsigned char TimeOut;	//�ӽڵ������ʱʱ��
	unsigned char CS;
}tpAFN05_4;

typedef struct
{
	unsigned char 	Ip[4];			//ip
	unsigned char	NetMask[4];		//��������
	unsigned char	GwIp[4];		//����
	unsigned short 	Port;			//TCP Server�̶��˿�
	unsigned short	TopPort;		//��ֵ�ն�Server�̶��˿�
	unsigned char 	CS;
}tpIpPort;	//��ģ��Ӧ�ö�Ӧ��  IP �˿�

typedef struct
{
	unsigned char flag;	//0 - 0xFF ÿ���յ�������������� flag - 1
						//���ճɹ�����ĳ��ձ�־����Ϊflag
	unsigned char CS;	//CSУ��
}tpCyFlag;	//���ճɹ���ı�־

typedef struct
{
	tpAFN05_1 	AFN05_1;
	tpAFN05_2	AFN05_2;
	tpAFN05_4	AFN05_4;
	tpIpPort	IpPort;
	tpCyFlag	cyFlag;
}tpConfiguration;	//ϵͳ���ò���

typedef struct
{
	tpAFN05_1 		AFN05_1;
	tpAFN05_2		AFN05_2;
	tpAFN05_4		AFN05_4;
	tpIpPort		IpPort;
	unsigned char 	CyFlag;	//���ճɹ��ı�־
}RunPara;	//���в���

#ifdef	_SYS_PARA_C_
RunPara _RunPara;
int _RebootUsart0;	//ִ��ĳЩָ�����Ҫ������־(����:ִ��Ӳ����λ����,���ñ�־����Ϊ0x66ϵͳ����)���ֻ�Ǵ���0�ı�־
int _RebootInfrared;	//ͬ������(����Ǻ���ӵ�������������)
#endif

#ifndef	_SYS_PARA_C_
extern	RunPara _RunPara;
extern int _RebootUsart0;	//ִ��ĳЩָ�����Ҫ������־(����:ִ��Ӳ����λ����,���ñ�־����Ϊ0x66ϵͳ����)
extern int _RebootInfrared;	//ͬ������(����Ǻ���ӵ�������������)
#endif

#endif /* COMMONALITY_SYSPARA_H_ */
