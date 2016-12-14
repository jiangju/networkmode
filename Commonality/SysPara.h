/*
 * SysPara.h
 *	系统参数文件
 *  Created on: 2016年6月29日
 *      Author: j
 */
#include <pthread.h>
#ifndef COMMONALITY_SYSPARA_H_
#define COMMONALITY_SYSPARA_H_

#define VENDOR_CODE0	'H'		//厂商代码
#define VENDOR_CODE1	'H'		//厂商代码

#define CHIP_CODE0		'H'		//芯片代码
#define	CHIP_CODE1		'H'		//芯片代码

#define	VERSION_DATE0	0x28	//版本日期 	日
#define	VERSION_DATE1	0x06	//版本日期	月
#define	VERSION_DATE2	0x16	//版本日期	年

#define	VERSION_NUM0	0x02	//版本号
#define	VERSION_NUM1	0x00	//版本号

#define TOP_FIX_DADT_NUM	15	//充值终端需要的数据标识个数

#define NET_MODE_V		"NetworkMode1.0.00"		//当前以太网模块版本名称
#define LL_LL			11						//产品名称长度
#define V_LL			6						//版本号长度x.x.xx

#define	CONFIG_FILE		"/opt/configuration"	//配置文件名
#define	STAND_BOOK_FILE	"/opt/stand_book"		//台账文件

#define	NODE_ADDR_LEN	6		//节点地址长度

#define TER_ADDR_LEN	4		//终端地址长度

#define DADT_LEN		4		//数据标识长度

#define TIME_FRA_LEN	6		//时间格式长度

#define	AMM_ADDR_LEN	6		//电表地址长度
#define AMM_MAX_NUM		2040	//电表最大数量

#define TER_UNDER_AMM_MAX		192	//终端下电表最大个数

#define SOCKET_RV_MAX	1024			//套接字接受最大字节数
#define SOCKET_TICKER	300				//套接字心跳倒计时
#define NETWORK_MAX_CONNCET	600			//socket最大连接数
typedef enum
{
	INFR 	= 0x11,	//红外
	THR 	= 0x22,	//三网合一
	TOPUP	= 0x33,	//充值终端
	UST		= 0x44,	//串口
	ZHU		= 0x55,	//主站
}enDEV;				//设备类型

typedef struct
{
	unsigned char Amm[AMM_ADDR_LEN];	//电表号
	unsigned char type;					//通讯规约类型
	unsigned char Ter[TER_ADDR_LEN];	//终端地址
	unsigned char cyFlag;				//抄表标志  (当等于运行参数中的抄表成功的标志时，抄表成功)

	unsigned char CS;					//CS校验
}AmmAttribute;

typedef struct
{
	unsigned char HostNode[NODE_ADDR_LEN];
	unsigned char CS;
}tpAFN05_1;		//主节点地址

typedef struct
{
	unsigned char IsAppera;	//是否允许从节点事件上报		0禁止  1允许
	unsigned char CS;
}tpAFN05_2;

typedef struct
{
	unsigned char TimeOut;	//从节点监控最大超时时间
	unsigned char CS;
}tpAFN05_4;

typedef struct
{
	unsigned char 	Ip[4];			//ip
	unsigned char	NetMask[4];		//子网掩码
	unsigned char	GwIp[4];		//网关
	unsigned short 	Port;			//TCP Server固定端口
	unsigned short	TopPort;		//充值终端Server固定端口
	unsigned char 	CS;
}tpIpPort;	//该模块应用对应的  IP 端口

typedef struct
{
	unsigned char flag;	//0 - 0xFF 每当收到抄收重启命令后 flag - 1
						//抄收成功后电表的抄收标志设置为flag
	unsigned char CS;	//CS校验
}tpCyFlag;	//抄收成功后的标志

typedef struct
{
	unsigned char flag;	//0x44 有下发过台账
	unsigned char CS;	//CS校验
}tpIsStand;

typedef struct
{
	unsigned char 	ip[4];
	unsigned short 	port;
	unsigned char 	CS;
}tpServer;				//授权服务端

typedef struct
{
	tpAFN05_1 	AFN05_1;
	tpAFN05_2	AFN05_2;
	tpAFN05_4	AFN05_4;
	tpIpPort	IpPort;
	tpCyFlag	cyFlag;
	tpIsStand 	StandFlag;	//是否有台账下发
	tpServer	server;
}tpConfiguration;	//系统配置参数

typedef struct
{
	tpAFN05_1 		AFN05_1;
	tpAFN05_2		AFN05_2;
	tpAFN05_4		AFN05_4;
	tpIpPort		IpPort;
	unsigned char 	CyFlag;		//抄收成功的标志
	unsigned char 	StandFlag;	//是否有台账下发
	tpServer		server;
	pthread_mutex_t mutex;
}RunPara;	//运行参数

#ifdef	_SYS_PARA_C_

RunPara _RunPara;
int 	_RebootUsart0;		//执行某些指令后需要重启标志(例如:执行硬件复位命令,将该标志设置为0x66系统重启)这个只是串口0的标志
int 	_RebootInfrared;	//同上作用(这个是红外接的需重启的命令)
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
extern int 		_RebootUsart0;		//执行某些指令后需要重启标志(例如:执行硬件复位命令,将该标志设置为0x66系统重启)
extern int 		_RebootInfrared;	//同上作用(这个是红外接的需重启的命令)
//extern const unsigned char _top_fix_dadt[][4];
#endif

#endif /* COMMONALITY_SYSPARA_H_ */
