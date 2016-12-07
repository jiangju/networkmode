/*
 * DL3761_AFN12.c
 *
 *  Created on: 2016年8月11日
 *      Author: j
 */

#include "DL376_1.h"
#include "DL3761_AFN15.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CommLib.h"
#include <stdlib.h>
#include <stddef.h>
#include "SysPara.h"
#include "log3762.h"

/*
 * 函数功能:设置ip及端口号
 * */
void DL3761_AFN15_07(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int fd;
	volatile int i = 3;	//打开文件失败重试次数
	int res = -1;
	int len = 0;
	int outdex = 0;

	tpIpPort ip_port;
	memset(&ip_port, 0, sizeof(tpIpPort));

	//打开系统配置参数文件
	while(i > 0)
	{

		i--;
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd < 0)
		{
			perror("open config file");
			res = -1;
		}
		else
		{
			//IP 地址
			memcpy(ip_port.Ip, rvframe3761->Frame376_1App.AppBuf + index, 4);
//			printf(" %d.%d.%d.%d\n",ip_port.Ip[0],ip_port.Ip[1],ip_port.Ip[2],ip_port.Ip[3]);
			index += 4;
			//子网掩码
			memcpy(ip_port.NetMask, rvframe3761->Frame376_1App.AppBuf + index, 4);
			index += 4;
			//网关
			memcpy(ip_port.GwIp, rvframe3761->Frame376_1App.AppBuf + index, 4);
			index += 4;
			//端口号0
			ip_port.Port = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;
			index += 2;
			//端口号1
			ip_port.TopPort = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;

			if(ip_port.TopPort == ip_port.Port)
			{
				res = -1;
				close(fd);
				break;
			}

			len = offsetof(tpIpPort, CS);
			ip_port.CS = Func_CS(&ip_port, len);
			len = offsetof(tpConfiguration, IpPort);
			res = WriteFile(fd, len, &ip_port, sizeof(tpIpPort));
//			printf("res********** %d\n",res);
			close(fd);
			break;
		}
	}
	if(0 <= res)
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}
	else
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
	_RebootInfrared = 0x66;
}

/*
 * 函数功能:主站ip以及端口设置
 * */
void DL3761_AFN15_05(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int fd;
	volatile int i = 3;	//打开文件失败重试次数
	int res = -1;
	int len = 0;
	int outdex = 0;

	tpServer ip_port;
	memset(&ip_port, 0, sizeof(tpServer));

	//打开系统配置参数文件
	while(i > 0)
	{

		i--;
		fd = open(CONFIG_FILE, O_RDWR);
		if(fd < 0)
		{
			perror("open config file");
			res = -1;
		}
		else
		{
			//IP 地址
			memcpy(ip_port.ip, rvframe3761->Frame376_1App.AppBuf + index, 4);
//			printf(" %d.%d.%d.%d\n",ip_port.Ip[0],ip_port.Ip[1],ip_port.Ip[2],ip_port.Ip[3]);
			index += 4;
			//端口号0
			ip_port.port = rvframe3761->Frame376_1App.AppBuf[index] + rvframe3761->Frame376_1App.AppBuf[index + 1] * 256;
			index += 2;
			len = offsetof(tpServer, CS);
			ip_port.CS = Func_CS(&ip_port, len);
			len = offsetof(tpConfiguration, server);
			res = WriteFile(fd, len, &ip_port, sizeof(tpServer));
			close(fd);
			pthread_mutex_lock(&_RunPara.mutex);
			memcpy(&_RunPara.server, &ip_port, sizeof(tpServer));
			pthread_mutex_unlock(&_RunPara.mutex);
			break;
		}
	}
	if(0 <= res)
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}
	else
	{
		//数据标识
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x02;
		snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
	_RebootInfrared = 0x66;
}

/*
 * 函数功能:开启/关闭376.2报文记录
 * */
void DL3761_AFN15_20(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	int index = 4;
	int outdex = 0;
	//获取开关报文记录状态
	unsigned char flag = rvframe3761->Frame376_1App.AppBuf[index];

	if(0 == flag)
	{	//关闭
		close_log_3762();
	}
	else
	{	//打开
		open_log_3762();
	}
	//数据标识
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x01;
	snframe3761->Frame376_1App.AppBuf[outdex++] = 0x00;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = outdex;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;
}

/*..
 * 函数功能:参数设置
 * */
void DL3761_AFN15_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	if(NULL == rvframe3761 || NULL == snframe3761)
		return;
	/***********************************填充链路层**************************/
	//起始符
	snframe3761->Frame376_1Link.BeginChar0 = 0x68;
	snframe3761->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	snframe3761->Frame376_1Link.EndChar = 0x16;
	//控制域
	snframe3761->Frame376_1Link.CtlField.DIR = 0;	//下行
	snframe3761->Frame376_1Link.CtlField.PRM = 0;	//从动站
	snframe3761->Frame376_1Link.CtlField.FCV = 0;	//FCB位无效
	snframe3761->Frame376_1Link.CtlField.FCB = 0;
	snframe3761->Frame376_1Link.CtlField.FUNC_CODE = 0;	//功能码
	//地址域
	memcpy(snframe3761->Frame376_1Link.AddrField.WardCode,\
			rvframe3761->Frame376_1Link.AddrField.WardCode, 2);
	memcpy(snframe3761->Frame376_1Link.AddrField.Addr, \
			rvframe3761->Frame376_1Link.AddrField.Addr, 2);
	snframe3761->Frame376_1Link.AddrField.MSA = 0;

	/*************************************应用层********************************/
	//功能码
	snframe3761->Frame376_1App.AFN = AFN3761_AFFI;
	//请求确认标志
	snframe3761->Frame376_1App.SEQ.CON = 0;	//不需要确认
	//帧类型
	snframe3761->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//帧序号
	snframe3761->Frame376_1App.SEQ.PSEQ_RSEQ = rvframe3761->Frame376_1App.SEQ.PSEQ_RSEQ;
	//时间标签
	snframe3761->Frame376_1App.SEQ.TPV = 0;	//不要时标
	//附加域 时标
	snframe3761->Frame376_1App.AUX.AUXTP.flag = 0;	//无效
	snframe3761->Frame376_1App.AUX.AUXEC.flag = 0;

	//解析接受帧
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);
//	printf("AFN 15  %d\n",Fn);
	switch(Fn)
	{
		case 7:
			DL3761_AFN15_07(rvframe3761, snframe3761);
			break;
		case 5:
			DL3761_AFN15_05(rvframe3761, snframe3761);
			break;
		case 20:
			DL3761_AFN15_20(rvframe3761, snframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0,  sizeof(tpFrame376_1));
}
