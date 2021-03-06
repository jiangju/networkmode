/*
 * SysPara.h
 *	系统参数文件
 *  Created on: 2016年6月29日
 *      Author: j
 */

#ifndef COMMONALITY_SYSPARA_H_
#define COMMONALITY_SYSPARA_H_

#define VENDOR_CODE0	'H'		//厂商代码
#define VENDOR_CODE1	'H'		//厂商代码

#define CHIP_CODE0		'H'		//芯片代码
#define	CHIP_CODE1		'H'		//芯片代码

#define	VERSION_DATE0	0x28	//版本日期 	日
#define	VERSION_DATE1	0x06	//版本日期	月
#define	VERSION_DATE2	0x16	//版本日期	年

#define	VERSION_NUM0	0x01	//版本号
#define	VERSION_NUM1	0x00	//版本号

#define	CONFIG_FILE		"/opt/configuration"	//配置文件名
#define	STAND_BOOK_FILE	"/opt/stand_book"	//台账文件

#define	NODE_ADDR_LEN	6		//节点地址长度

#define TER_ADDR_LEN	4		//终端地址长度

#define DADT_LEN		4		//数据标识长度

#define	AMM_ADDR_LEN	6//电表地址长度
#define AMM_MAX_NUM		2040//电表最大数量

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
	tpAFN05_1 	AFN05_1;
	tpAFN05_2	AFN05_2;
	tpAFN05_4	AFN05_4;
	tpIpPort	IpPort;
	tpCyFlag	cyFlag;
}tpConfiguration;	//系统配置参数

typedef struct
{
	tpAFN05_1 		AFN05_1;
	tpAFN05_2		AFN05_2;
	tpAFN05_4		AFN05_4;
	tpIpPort		IpPort;
	unsigned char 	CyFlag;	//抄收成功的标志
}RunPara;	//运行参数

#ifdef	_SYS_PARA_C_
RunPara _RunPara;
int _RebootUsart0;	//执行某些指令后需要重启标志(例如:执行硬件复位命令,将该标志设置为0x66系统重启)这个只是串口0的标志
int _RebootInfrared;	//同上作用(这个是红外接的需重启的命令)
#endif

#ifndef	_SYS_PARA_C_
extern	RunPara _RunPara;
extern int _RebootUsart0;	//执行某些指令后需要重启标志(例如:执行硬件复位命令,将该标志设置为0x66系统重启)
extern int _RebootInfrared;	//同上作用(这个是红外接的需重启的命令)
#endif

#endif /* COMMONALITY_SYSPARA_H_ */
