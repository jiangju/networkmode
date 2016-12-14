/*
 * SysPara.h
 *	ϵͳ�����ļ�
 *  Created on: 2016��6��29��
 *      Author: j
 */
#include <pthread.h>
#ifndef COMMONALITY_SYSPARA_H_
#define COMMONALITY_SYSPARA_H_

#define VENDOR_CODE0	'H'		//���̴���
#define VENDOR_CODE1	'H'		//���̴���

#define CHIP_CODE0		'H'		//оƬ����
#define	CHIP_CODE1		'H'		//оƬ����

#define	VERSION_DATE0	0x28	//�汾���� 	��
#define	VERSION_DATE1	0x06	//�汾����	��
#define	VERSION_DATE2	0x16	//�汾����	��

#define	VERSION_NUM0	0x02	//�汾��
#define	VERSION_NUM1	0x00	//�汾��

#define TOP_FIX_DADT_NUM	15	//��ֵ�ն���Ҫ�����ݱ�ʶ����

#define NET_MODE_V		"NetworkMode1.0.00"		//��ǰ��̫��ģ��汾����
#define LL_LL			11						//��Ʒ���Ƴ���
#define V_LL			6						//�汾�ų���x.x.xx

#define	CONFIG_FILE		"/opt/configuration"	//�����ļ���
#define	STAND_BOOK_FILE	"/opt/stand_book"		//̨���ļ�

#define	NODE_ADDR_LEN	6		//�ڵ��ַ����

#define TER_ADDR_LEN	4		//�ն˵�ַ����

#define DADT_LEN		4		//���ݱ�ʶ����

#define TIME_FRA_LEN	6		//ʱ���ʽ����

#define	AMM_ADDR_LEN	6		//����ַ����
#define AMM_MAX_NUM		2040	//����������

#define TER_UNDER_AMM_MAX		192	//�ն��µ��������

#define SOCKET_RV_MAX	1024			//�׽��ֽ�������ֽ���
#define SOCKET_TICKER	300				//�׽�����������ʱ
#define NETWORK_MAX_CONNCET	600			//socket���������
typedef enum
{
	INFR 	= 0x11,	//����
	THR 	= 0x22,	//������һ
	TOPUP	= 0x33,	//��ֵ�ն�
	UST		= 0x44,	//����
	ZHU		= 0x55,	//��վ
}enDEV;				//�豸����

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
	unsigned char flag;	//0x44 ���·���̨��
	unsigned char CS;	//CSУ��
}tpIsStand;

typedef struct
{
	unsigned char 	ip[4];
	unsigned short 	port;
	unsigned char 	CS;
}tpServer;				//��Ȩ�����

typedef struct
{
	tpAFN05_1 	AFN05_1;
	tpAFN05_2	AFN05_2;
	tpAFN05_4	AFN05_4;
	tpIpPort	IpPort;
	tpCyFlag	cyFlag;
	tpIsStand 	StandFlag;	//�Ƿ���̨���·�
	tpServer	server;
}tpConfiguration;	//ϵͳ���ò���

typedef struct
{
	tpAFN05_1 		AFN05_1;
	tpAFN05_2		AFN05_2;
	tpAFN05_4		AFN05_4;
	tpIpPort		IpPort;
	unsigned char 	CyFlag;		//���ճɹ��ı�־
	unsigned char 	StandFlag;	//�Ƿ���̨���·�
	tpServer		server;
	pthread_mutex_t mutex;
}RunPara;	//���в���

#ifdef	_SYS_PARA_C_

RunPara _RunPara;
int 	_RebootUsart0;		//ִ��ĳЩָ�����Ҫ������־(����:ִ��Ӳ����λ����,���ñ�־����Ϊ0x66ϵͳ����)���ֻ�Ǵ���0�ı�־
int 	_RebootInfrared;	//ͬ������(����Ǻ���ӵ�������������)
//const unsigned char _top_fix_dadt[TOP_FIX_DADT_NUM][4] =
//											{
//											{0x36,0x38,0x33,0x37},{0x33,0x35,0xC3,0x33},{0x33,0x32,0x33,0x33},
//											{0x34,0x32,0x33,0x33},{0x35,0x32,0x33,0x33},{0x33,0x33,0x3E,0x33},
//											{0x3E,0x33,0xB3,0x35},{0x3F,0x33,0xB3,0x35},{0x53,0x33,0xB3,0x35},
//											{0x34,0x35,0x66,0x36},{0x33,0x34,0x34,0x35},{0x33,0x34,0x35,0x35},
//											{0x34,0x43,0x33,0x37},{0x34,0x35,0xC3,0x33},{0x32,0x33,0x33,0x3A}
//											};
#endif

#ifndef	_SYS_PARA_C_

extern RunPara  _RunPara;
extern int 		_RebootUsart0;		//ִ��ĳЩָ�����Ҫ������־(����:ִ��Ӳ����λ����,���ñ�־����Ϊ0x66ϵͳ����)
extern int 		_RebootInfrared;	//ͬ������(����Ǻ���ӵ�������������)
//extern const unsigned char _top_fix_dadt[][4];
#endif

#endif /* COMMONALITY_SYSPARA_H_ */
