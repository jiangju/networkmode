/*
 * DL376_1_DataType.h
 *
 *  Created on: 2016年7月14日
 *      Author: j
 */

#ifndef ST376_1_DL376_1_DATATYPE_H_
#define ST376_1_DL376_1_DATATYPE_H_

#include <stdbool.h>
#define FRMBUFF_LEN 2048

typedef enum
{
	FRMLEN_AFN   = 1, 	//功能码的长度
	FRMLEN_SEQ   = 1, 	//帧序列域的长度
	FRMLEN_EC    = 2, 	//EC的长度
	FRMLEN_TP    = 6, 	//时间标签Tp的长度
	FRMLEN_PW    = 16,	//PW的长度
	FRMLEN_DA    = 2, 	//数据标识单元__信息点的长度
	FRMLEN_DT    = 2, 	//数据标识单元__信息类的长度
	FRMLEN_HEAD  = 6,	//帧头的长度
	FRMLEN_CON   = 1,	//控制域的长度
	FRMLEN_ADD   = 5,	//地址域的长度
	FRMLEN_CS    = 1,	//校验和的长度
	FRMLEN_END   = 1,	//帧尾的长度
	FRMLEN_EMPTY1 = 16,	//FRMLEN_HEAD+FRMLEN_CON+FRMLEN_ADD+FRMLEN_CS+FRMLEN_END	//空帧的长度
}enFRMLEN;				//帧长度枚举

typedef enum
{
	AFN3761_AFFI		= 0x00,	//确认帧
	AFN3761_REST		= 0x01,	//复位命令
	AFN3761_INTFA		= 0x02,	//链路接口检测
	AFN3761_RELAY		= 0x03,	//中继检测
	AFN3761_SETPARA		= 0x04,	//参数设置
	AFN3761_CTRL		= 0x05,	//控制命令
	AFN3761_CAPASS		= 0x06,	//身份认证及密码协商
	AFN3761_TERMCONFIG	= 0x09,	//请求终端配置
	AFN3761_QURPARA		= 0x0a,	//参数查询
	AFN3761_REPORT		= 0x0b,	//定时上报
	AFN3761_QUR1CLASS	= 0x0c,	//请求1类数据
	AFN3761_QUR2CLASS	= 0x0d,	//请求2类数据
	AFN3761_QUR3CLASS	= 0x0e,	//请求3类数据
	AFN3761_FILETRA		= 0x0f,	//文件传输
	AFN3761_DATASEND	= 0x10,	//数据转发
	AFN3761_EXTEND12	= 0x12,	//扩展AFN=12
	AFN3761_EXTEND13	= 0x13,	//扩展AFN=13
	AFN3761_EXTEND15	= 0x15,	//扩展AFN=15
	AFN3761_EXTEND16	= 0x16,	//扩展AFN=16
}enAFN3761;					   		//AFN类型

typedef struct
{
	unsigned char DIR;			//DIR=0下行报文  DIR=1上行报文
	unsigned char PRM;			//PRM=0报文来自从动站  PRM=1报文来自启动站

	//下行报文有效
	unsigned char FCV;			//1-FCB有效  0-FCB无效
	unsigned char FCB;

	//上行报文有效
	unsigned char ACD;			//1-存在重要事件  0-不存在重要事件

	//功能码
	unsigned char FUNC_CODE;
}tpCtlField1;	//376.1控制域

typedef struct
{
	unsigned char  WardCode[2];		//BCD行政区码
	unsigned char  Addr[2];			//BIN终端地址
	unsigned char  MSA;				//BIN主站地址和组地址标志
}tpAddrField;	//376.1地址域

typedef struct
{
	unsigned char 	BeginChar0;	//开始字符 0x68
	unsigned short	L1;			//长度1
	unsigned short	L2;			//长度2
	unsigned char 	BeginChar1;	//开始字符 0x68
	tpCtlField1		CtlField;	//控制域
	tpAddrField		AddrField;	//地址域

	unsigned char	CS;			//效验和
	unsigned char	EndChar;	//结束字符( 0x16 )

}tpFrame376_1Link;

typedef enum
{
	FRM_FIRFIN_MID		= 0x00,	//多帧/中间帧
	FRM_FIRFIN_END		= 0x20,	//多帧/结束帧
	FRM_FIRFIN_BEGIN	= 0x40,	//多帧/第1帧，后续有帧
	FRM_FIRFIN_SING		= 0x60,	//单帧
}enFRM_FIRFIN;					//帧类型定义

typedef struct
{
	unsigned char TPV;					//0-无时标   1-时标有效
	enFRM_FIRFIN  FIR_FIN;				//帧类型
	unsigned char CON;					//0-不需要对该帧报文确认, 1-需要对该帧报文确认
	unsigned char PSEQ_RSEQ;			//启动/响应帧序号
}tpSEQ;

typedef struct
{
	unsigned char  PW[FRMLEN_PW];	//密码--16字节
	unsigned char  flag;			//0-无效 1-有效
}tpAUXPW;							//密码

typedef struct
{
	unsigned char EC[2];//事件计数器
	unsigned char flag;	//0-无效 1-有效
}tpAUXEC;				//EC

typedef struct
{
	unsigned char	PFC;		//BIN启动帧帧序号计数器
	unsigned char	TPFlag[4];	//启动帧发送时标(秒/分/时/日)
	unsigned char	RTS;		//BIN允许发送传输延时时间min
	unsigned char	flag;		//0-无效 1-有效
}stAUXTP;						//时标

typedef struct
{
	tpAUXPW AUXPW;//密码
	tpAUXEC AUXEC;//EC
	stAUXTP AUXTP;//时标
}tpAUX;			  //附加信息域----AUX

typedef struct
{
	unsigned char	AFN;					//功能码
	tpSEQ			SEQ;					//SEQ帧序列域

	unsigned short	Len;					//应用层帧长---不包括AFN/SEQ
	unsigned char	AppBuf[FRMBUFF_LEN];	//数据缓存

	tpAUX	AUX;							//附加域
}tpFrame376_1App;							//376.1应用层

typedef struct
{
	tpFrame376_1Link	Frame376_1Link;
	tpFrame376_1App		Frame376_1App;
	bool				IsHaving;
}tpFrame376_1;	//376.1数据结构

#endif /* ST376_1_DL376_1_DATATYPE_H_ */
