/*
 * DL376_2_DataType.h
 *
 *  Created on: 2016年6月21日
 *      Author: Administrator
 */

#ifndef ST376_2_DL376_2_DATATYPE_H_
#define ST376_2_DL376_2_DATATYPE_H_

#include <stdbool.h>

#define	USER_DATA_LEN 	5000	//用户数据最大长度
#define	ADDR_LEN		6		//地址长度
#define	DT_LEN			2		//数据单元标识长度

#define MAX_RELAY		15		//中继最级别

#define BEGINCHAR		0x68	//起始字符
#define ENDCHAR			0x16	//结束字符

#define	FRMLEN_EMPTY2	12		//空帧的长度

enum
{
	BEGIN_LEN = 1,	//起始字符长度
	END_LEN = 1,	//结束字符长度
	L_LEN = 2,		//数据长度长度
	CTL_LEN = 1,	//控制域长度
	CS_LEN = 1,		//CS校验码长度
	R_LEN = 6,		//信息域长度
	AFN_LEN = 1,	//功能码长度
};

typedef enum
{
	FRMPRM_PRM_SLAVE	= 0,	//从动站
	FRMPRM_PRM_PRIMARY  = 1		//启动站
}enFRMPRM;						//启动站从动站类型枚举

typedef enum
{
	AFN_AFFI		= 0x00,	//确认帧
	AFN_REST		= 0x01,	//复位命令
	AFN_DATASEND	= 0x02,	//数据转发
	AFN_QUERDATA	= 0x03,	//查询数据
	AFN_PORTDETE	= 0x04,	//链路接口检测
	AFN_CTRL		= 0x05,	//控制命令
	AFN_ACCORD		= 0x06,	//主动上报

	AFN_ROUTEQUERY	= 0x10,	//路由查询
	AFN_ROUTESET	= 0x11,	//路由设置
	AFN_ROUTECTRL	= 0x12,	//路由控制
	AFN_ROUTESEND	= 0x13,	//路由数据转发
	AFN_ROUTERCV	= 0x14, //路由数据抄读
	AFN_FILETRAN    = 0x15,	//文件传输
}enAFN3762;						//AFN类型

typedef struct
{
	unsigned char DIR;			//传输方向(0 下行报文 1 上行报文)
	unsigned char PRM;			//启动标志(1 启动站     0 从动站)
	unsigned char CommMode;		//通讯方式   0 备用
								//1 集中式路由载波
								//2 分布式路由载波
								//3 宽带载波
								//4~9 备用
								//10 微功率无线通讯
								//11~19 备用
								//20 以太网
								//21~63备用
}tpCtlField2;					//376.2帧中控制域结构

typedef struct
{
	unsigned char 	BeginChar;	//开始字符 0x68
	unsigned short 	Len;		//长度 （用户数据长度+6） 不大于65535
	tpCtlField2		CtlField;	//控制域
	unsigned char 	CS;			//校验和
	unsigned char 	Endchar;	//结束字符0x16
}tpFrame376_2Link;				//376.2数据链路层

typedef struct
{
	unsigned char	RelayRank;				//中继级别(只是用高4位)
	unsigned char	ConflictDetection;		//冲突检测 0/1
	unsigned char 	CommModeIdentifying;	//通讯模块标识0/1
	unsigned char	AffiliateIdentifying;	//附属节点标识0/1
	unsigned char 	RouteIdentifying;		//路由标识
	unsigned char 	ErrEecoveryIdentifying;	//纠错编码标识(只用高4位)
	unsigned char   ChannelIdentifying;		//信道标识(只用低4位)

	unsigned char 	PredictAnswerLen;		//预计应答字节数,上行赋值0x11

	unsigned char	SpeedIdentifying;		//速度单位标识
	unsigned short	CommSpeed;				//通讯速率(速率单位标识与通讯速率组成2字节长度，上行时赋值0x99990000)

	unsigned char	MessageSerialNumber;	//报文序列号
}tpR;										//信息域R

typedef	struct
{
	unsigned char 	SourceAddr[ADDR_LEN];				//源地址
	unsigned char	RelayAddr[ADDR_LEN *MAX_RELAY];		//中继地址
	unsigned char	DestinationAddr[ADDR_LEN];			//目的地址
}tpAddr;												//地址阈

typedef struct
{
	unsigned char	AFN;			//功能码
	unsigned short  Len;			//用户数据长度
	unsigned char	Buffer[USER_DATA_LEN];	//用户数据
}tpAppData;

typedef struct
{
	tpR		R;			//信息域R
	tpAddr	Addr;		//地址阈
	tpAppData	AppData;//用户数据
}tpFrame376_2App;				//376.2用户数据层

typedef struct
{
	tpFrame376_2Link	Frame376_2Link;
	tpFrame376_2App 	Frame376_2App;
	bool				IsHaving;
}tpFrame376_2;		//376_2数据结构

#endif /* ST376_2_DL376_2_DATATYPE_H_ */
