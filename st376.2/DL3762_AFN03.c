/*
 * AFN03.c
 *
 *  Created on: 2016年6月27日
 *      Author: j
 */
#include <stdio.h>
#include <string.h>
#include <DL3762_AFN03.h>
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "SysPara.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include "StandBook.h"

/*
 *函数功能:查询厂商代码和版本信息
 *参数:	snframe3762	需填充的发送帧结构
 *返回值:	无
 * */
void AFN03_01(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	//厂商代码
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE1;

	//芯片代码
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE1;

	//版本日期  日
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE0;

	//版本日期  月
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE1;

	//版本日期  年
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE2;

	//版本
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM1;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
//	printf("L1  %d\n",snframe3762->Frame376_2App.AppData.Len);
//	printf("L2  %d\n",snframe3762->Frame376_2Link.Len);
}

/*
 * 函数功能:查询噪音值
 * 参数snframe3762	需填充的发送帧结构
 * */
void AFN03_02(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 * 函数功能:从节点侦听信息
 * */
void AFN03_03(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char p = 0;
	unsigned char num = 0;
	unsigned short inIndex = 2;
	unsigned short outIndex = 2;
	StandNode node;
	//获取开始节点索引
	p = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];
	//读取数量
	num = rvframe3762->Frame376_2App.AppData.Buffer[inIndex++];

	if(p == 0)
	{
		p = 1;
	}
	else if(p > _StandNodeNum)
	{
		num = 0;
	}
	else if((p + num) > _StandNodeNum)
	{
		num = p + num - _StandNodeNum + 1;
	}

	//帧听到从节总点数量
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = num;
	//侦听到的本帧传输的从节点数量
	snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = num;
	while(num > 0)
	{
		if(0 > GetStandNode(p + num - 1, &node))
		{
			num--;
			continue;
		}
		//从节点地址
		memcpy(snframe3762->Frame376_2App.AppData.Buffer + outIndex, node.Amm, AMM_ADDR_LEN);
		outIndex += AMM_ADDR_LEN;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;
		snframe3762->Frame376_2App.AppData.Buffer[outIndex++] = 0;
		num--;
	}

	snframe3762->Frame376_2App.AppData.Len = outIndex;
	snframe3762->Frame376_2Link.Len += outIndex;
}

/*
 * 函数功能:查询主节点地址
 * 参数:snframe3762	需填充的发送帧结构
 * */
void AFN03_04(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	tpAFN05_1 afn05_1;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));
	int fd;
	int len = 0;
	int i = 3;
	while(i--)
	{
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd >= 0)
		{
			break;
		}
	}

	if(fd < 0)
	{

	}
	else
	{
		len = offsetof(tpConfiguration, AFN05_1);
		if(0 == ReadFile(fd, len, (void *)(&afn05_1), sizeof(tpAFN05_1)))
		{
			len = offsetof(tpAFN05_1, CS);
			if(afn05_1.CS != Func_CS((void*)&afn05_1, len))
			{
				//将运行参数的主节点地址赋给配置参数的主节点地址
				memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
				afn05_1.CS = Func_CS((void*)&afn05_1, len);
				len = offsetof(tpConfiguration, AFN05_1);
				WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
			}
		}
		else
		{
			close(fd);
		}
	}

	memcpy((snframe3762->Frame376_2App.AppData.Buffer + index), &afn05_1.HostNode, NODE_ADDR_LEN);
	index += NODE_ADDR_LEN;

	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;

	close(fd);
}

/*
 * 函数功能:主节点状态字和通信速率
 * */
void AFN03_05(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;

	//状态字(周期抄表模式 + 主节点信道特征 + 速率数量 + 备用 + 信道数量)
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x30;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0F;
	//
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x04;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x64;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x32;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_06(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 * 函数功能:读取从节点监控最大超时时间
 * */
void AFN03_07(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	//从节点监控最大超时时间（单位S）
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x5A;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFF;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_08(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

void AFN03_09(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x0;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 *函数功能:本地通信模块运行模式信息
 **/
void AFN03_10(tpFrame376_2 *snframe3762)
{
	unsigned short index = 2;
	tpAFN05_1 afn05_1;
	memset(&afn05_1, 0, sizeof(tpAFN05_1));
	int fd;
	int len = 0;
	int i = 3;
	unsigned char temp = 0;
	//通信方式 + 路由管理方式 + 测量点信息模式 + 周期抄表模式
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xF1;
	//传输延时参数支持 + 失败节点切换发起方式 + 广播命令确认方式 + 广播命令信道执行方式
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//信道数量 + 速率数量n
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x60;
	//抵压电网掉电信息
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//保留
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	//从节点监控最大超时时间（单位S）
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x5A;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFF;
	//广播命令最大超时时间（单位S）
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xFA;
	//最大支持的376.2报文长度
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0xE2;
	//文件传输支持的最大单包长度
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x28;
	//升级操作等待时间
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x22;
	//主节点地址
	while(i--)
	{
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd >= 0)
		{
			break;
		}
	}

	if(fd < 0)
	{
	}
	else
	{
		len = offsetof(tpConfiguration, AFN05_1);
		if(0 == ReadFile(fd, len, (void *)(&afn05_1), sizeof(tpAFN05_1)))
		{
			len = offsetof(tpAFN05_1, CS);
			if(afn05_1.CS != Func_CS((void*)&afn05_1, len))
			{
				//将运行参数的主节点地址赋给配置参数的主节点地址
				memcpy(&afn05_1.HostNode, &_RunPara.AFN05_1.HostNode, NODE_ADDR_LEN);
				afn05_1.CS = Func_CS((void*)&afn05_1, len);
				len = offsetof(tpConfiguration, AFN05_1);
				WriteFile(fd, len, (void*)&afn05_1, sizeof(tpAFN05_1));
			}
			close(fd);
		}
		else
		{
			close(fd);
		}
	}
	memcpy((snframe3762->Frame376_2App.AppData.Buffer + index), &afn05_1.HostNode, NODE_ADDR_LEN);
	index += NODE_ADDR_LEN;
	//最大支持从节点数量
	temp = AMM_MAX_NUM % 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	temp = AMM_MAX_NUM / 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	//从节点数量
	temp = _StandNodeNum % 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	temp = _StandNodeNum / 256;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = temp;
	//通信模块使用的376.2协议发布日期（BCD）
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x16;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x07;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x27;
	//通信模块使用的376.2协议最后备案日期（BCD）
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x09;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x13;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x43;
	//通信模块厂商代码及版本信息
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VENDOR_CODE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = CHIP_CODE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE1;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_DATE2;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM0;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = VERSION_NUM1;
	//
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x04;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x58;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x02;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x64;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x00;
	snframe3762->Frame376_2App.AppData.Buffer[index++] = 0x32;
	snframe3762->Frame376_2App.AppData.Len = index;
	snframe3762->Frame376_2Link.Len += index;
}

/*
 *函数功能:本地通信模块376.2报文支持信息
 **/
void AFN03_11(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned short inIndex = 2;
	unsigned short outInidex = 2;
	unsigned char AFN = 0;

	//获取待查询的功能码
	AFN = rvframe3762->Frame376_2App.AppData.Buffer[inIndex];

	//填充数据
	switch (AFN)
	{
		case 0:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 1:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 3:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x5B;
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 30);
			outInidex += 30;
			break;
		case 5:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x0D;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 6:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x06;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 10:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x1B;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 11:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 12:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x07;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 13:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x01;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		case 14:
			snframe3762->Frame376_2App.AppData.Buffer[outInidex++] = 0x03;
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 31);
			outInidex += 31;
			break;
		default:
			memset((snframe3762->Frame376_2App.AppData.Buffer + outInidex), 0, 32);
			outInidex += 32;
			break;
	}
	snframe3762->Frame376_2App.AppData.Len = outInidex;
	snframe3762->Frame376_2Link.Len += outInidex;
}

/*
 * 函数功能:执行查询请求
 * 参数: rvframe3762 待解析的376.2数据
 * 		snframe3762	解析后填充的发送376.2
 * 返回值:无
 * */
void DL3762_AFN03_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
	unsigned char DT[2] = {0};
	char Fn = -1;
	unsigned short temp = 0;
	if(NULL == snframe3762)
	{
		return;
	}

	memset(snframe3762, 0, sizeof(tpFrame376_2));

	/**********************************先填充链路层*****************************************/
	snframe3762->Frame376_2Link.BeginChar = 0x68;
	snframe3762->Frame376_2Link.Endchar = 0x16;
	if(0 == rvframe3762->Frame376_2Link.CtlField.PRM ||
			1 == rvframe3762->Frame376_2Link.CtlField.DIR)
	{
		return;
	}
	snframe3762->Frame376_2Link.CtlField.PRM = 0;
	snframe3762->Frame376_2Link.CtlField.DIR = 1;

	snframe3762->Frame376_2Link.CtlField.CommMode = rvframe3762->Frame376_2Link.CtlField.CommMode;
	snframe3762->Frame376_2Link.Len = BEGIN_LEN + END_LEN + L_LEN + CS_LEN + CTL_LEN;

	/************************************应用层**********************************************/
	//无地址域
	memset(&snframe3762->Frame376_2App.R, 0, sizeof(tpR));
	snframe3762->Frame376_2App.R.MessageSerialNumber = rvframe3762->Frame376_2App.R.MessageSerialNumber;
	snframe3762->Frame376_2Link.Len += R_LEN;
	snframe3762->Frame376_2App.AppData.AFN = AFN_QUERDATA;
	snframe3762->Frame376_2Link.Len += AFN_LEN;

	DT[0] = rvframe3762->Frame376_2App.AppData.Buffer[0];
	DT[1] = rvframe3762->Frame376_2App.AppData.Buffer[1];

	Fn = DTtoFN(DT);
	if(Fn < 0)
	{
		return;
	}

	snframe3762->Frame376_2App.AppData.Buffer[0] = DT[0];
	snframe3762->Frame376_2App.AppData.Buffer[1] = DT[1];
	snframe3762->Frame376_2App.AppData.Len = 2;
	temp = snframe3762->Frame376_2App.AppData.Len;
	switch(Fn)
	{
		case 1:
			AFN03_01(snframe3762);
			break;
		case 2:
			AFN03_02(snframe3762);
			break;
		case 4:
			AFN03_04(snframe3762);
			break;
		case 5:
			AFN03_05(snframe3762);
			break;
		case 6:
			AFN03_06(snframe3762);
			break;
		case 7:
			AFN03_07(snframe3762);
			break;
		case 8:
			AFN03_08(snframe3762);
			break;
		case 9:
			AFN03_09(snframe3762);
			break;
		case 10:
			AFN03_10(snframe3762);
			break;
		case 11:
			AFN03_11(rvframe3762, snframe3762);
			break;
		default :
			break;
	}

	memset(rvframe3762, 0, sizeof(tpFrame376_2));
	if(temp == snframe3762->Frame376_2App.AppData.Len)
	{
		return;
	}
	snframe3762->IsHaving = true;
}

