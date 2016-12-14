/*
 * DL3761_AFN13.c
 *
 *  Created on: 2016年8月11日
 *      Author: j
 */

#include "DL376_1.h"
#include "DL3761_AFN16.h"
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
#include "Route.h"
#include "StandBook.h"
#include <sys/time.h>
#include "SeekAmm.h"
#include "log3762.h"
#include "hld_ac.h"
#include "HLDUsb.h"

/*
 * 函数功能；查询授权主站ip及端口
 * */
void DL3761_AFN16_05(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	tpServer ip_port;
	memset(&ip_port, 0, sizeof(tpServer));
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
		printf("AFN 16 open CONFIG_FILE erro\n");
		return;
	}
	else
	{
		len = offsetof(tpConfiguration, server);
		if(0 == ReadFile(fd, len, (void *)(&ip_port), sizeof(tpServer)))
		{
			len = offsetof(tpServer, CS);
			if(ip_port.CS != Func_CS((void*)&ip_port, len))
			{
				memcpy(&ip_port, &_RunPara.server, sizeof(tpServer));
				ip_port.CS = Func_CS((void*)&ip_port, len);
				len = offsetof(tpConfiguration, server);
				WriteFile(fd, len, (void*)&ip_port, sizeof(tpServer));
			}
			close(fd);
		}
		else
		{
			printf("AFN 16 read CONFIG_FILE erro\n");
			close(fd);
			return;
		}
	}
	//ip
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.ip, 4);
	index += 4;

	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.port % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.port / 256;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;

}

/*
 * 函数功能:查询版本号
 * */
void DL3761_AFN16_06(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;

	//厂商代码
	snframe3761->Frame376_1App.AppBuf[index++] = VENDOR_CODE0;
	snframe3761->Frame376_1App.AppBuf[index++] = VENDOR_CODE1;

	//芯片代码
	snframe3761->Frame376_1App.AppBuf[index++] = CHIP_CODE0;
	snframe3761->Frame376_1App.AppBuf[index++] = CHIP_CODE1;

	//版本日期 	日
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE0;

	//版本日期	月
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE1;

	//版本日期	年
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_DATE2;

	//版本
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_NUM0;
	snframe3761->Frame376_1App.AppBuf[index++] = VERSION_NUM1;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能；查询ip 端口
 * */
void DL3761_AFN16_07(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	tpIpPort ip_port;
	memset(&ip_port, 0, sizeof(tpIpPort));
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
		printf("AFN 16 open CONFIG_FILE erro\n");
		return;
	}
	else
	{
		len = offsetof(tpConfiguration, IpPort);
		if(0 == ReadFile(fd, len, (void *)(&ip_port), sizeof(tpIpPort)))
		{
			len = offsetof(tpIpPort, CS);
			if(ip_port.CS != Func_CS((void*)&ip_port, len))
			{
				memcpy(&ip_port, &_RunPara.IpPort, sizeof(tpIpPort));
				ip_port.CS = Func_CS((void*)&ip_port, len);
				len = offsetof(tpConfiguration, IpPort);
				WriteFile(fd, len, (void*)&ip_port, sizeof(tpIpPort));
			}
			close(fd);
		}
		else
		{
			printf("AFN 16 read CONFIG_FILE erro\n");
			close(fd);
			return;
		}
	}
	//ip
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.Ip, 4);
	index += 4;
	//子网掩码
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.NetMask, 4);
	index += 4;
	//网关
	memcpy(snframe3761->Frame376_1App.AppBuf + index, ip_port.GwIp, 4);
	index += 4;
	//端口号0
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.Port / 256;
	//端口号1
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.TopPort % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = ip_port.TopPort / 256;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	memset(rvframe3761, 0, sizeof(tpFrame376_1));
	snframe3761->IsHaving = true;

}

/*
 * 函数功能:查询主节点地址
 * */
void DL3761_AFN16_08(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	memcpy(snframe3761->Frame376_1App.AppBuf + index, _RunPara.AFN05_1.HostNode, 6);
	index += 6;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:在线终端数量查询
 * */
void DL3761_AFN16_09(tpFrame376_1 *snframe3761)
{
	unsigned short index = 4;
	unsigned short num = 0;
	num = get_hld_route_node_num();

	snframe3761->Frame376_1App.AppBuf[index++] = num % 256;
	snframe3761->Frame376_1App.AppBuf[index++] = num / 256;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:在线终端查询
 * */
void DL3761_AFN16_10(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;

	unsigned short all_num = 0;
	unsigned short start_num;
	unsigned char num = 0;
	unsigned char temp_num = 0;

	//获取在线终端总数量
	all_num = get_hld_route_node_num();

	//获取待查询的起始序号
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++];
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++] * 256 + start_num;
	if(0 == start_num)
	{
		start_num = 1;
	}

	//获取查询个数
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//终端总数量
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num % 256;
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num / 256;

	if(start_num > all_num)
	{
		//本次应答数量
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//计算本次回复终端数量
		if(all_num - start_num + 1 >= num)
		{
			temp_num = num;
		}
		else
		{
			temp_num = all_num - start_num + 1;
		}

		//本次应答终端数量
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp_num;

		//填充终端地址
		while(temp_num > 0)
		{
			//根据序号获得相应路由节点
			if(0 != get_hld_route_node_ter_lstime(start_num,
					(snframe3761->Frame376_1App.AppBuf + out_inidex),
					(snframe3761->Frame376_1App.AppBuf + out_inidex+TER_ADDR_LEN)))
			{
				memset((snframe3761->Frame376_1App.AppBuf + out_inidex), 0xFF, TER_ADDR_LEN);
				memset((snframe3761->Frame376_1App.AppBuf + out_inidex + TER_ADDR_LEN), 0xFF, TIME_FRA_LEN);
			}
			out_inidex += TER_ADDR_LEN;
			out_inidex += TIME_FRA_LEN;
			start_num++;
			temp_num--;
		}
	}
	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:终端下属电表数量查询
 * */
void DL3761_AFN16_11(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char all_num = 0;

	//获取待查询终端地址
	memcpy(ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);

	//查询终端是否在线
	if(0 != check_hld_route_node_ter(ter))
	{
		//不在线
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//在线
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	}

	//获取终端下电表数量
	all_num = StatTerAmmNum(ter);

	//终端下电表数量
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:终端下属电表查询
 * */
void DL3761_AFN16_12(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char all_num = 0;
	unsigned short start_num;
	unsigned char num = 0;
	unsigned char temp_num = 0;
	int index = 0;
	StandNode node;

	//获取待查询终端地址
	memcpy(ter, rvframe3761->Frame376_1App.AppBuf + in_index, TER_ADDR_LEN);
	in_index += TER_ADDR_LEN;

	//获取电表起始序号
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//获取待读取电表数量
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	//查询终端是否在线
	if(0 != check_hld_route_node_ter(ter))
	{
		//不在线
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//在线
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	}

	//获取终端下电表数量
	all_num = StatTerAmmNum(ter);

	//终端下电表数量
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = all_num;

	if(start_num > all_num)
	{
		//本次应答数量
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//计算本次回复电表数量
		if(all_num - start_num + 1 >= num)
		{
			temp_num = num;
		}
		else
		{
			temp_num = all_num - start_num + 1;
		}

		//本次应答电表数量
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp_num;

		while(temp_num > 0)
		{
			index = SeekTerOfAmm(ter, start_num);
			if(index == -1)
			{
				memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, AMM_ADDR_LEN);
				out_inidex += AMM_ADDR_LEN;
				memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, TIME_FRA_LEN);
				out_inidex += TIME_FRA_LEN;
			}
			else
			{
				if(-1 == GetStandNode(index, &node))
				{
					memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, AMM_ADDR_LEN);
					out_inidex += AMM_ADDR_LEN;
					memset(snframe3761->Frame376_1App.AppBuf + out_inidex, 0xFF, TIME_FRA_LEN);
					out_inidex += TIME_FRA_LEN;
				}
				else
				{
					memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.Amm, AMM_ADDR_LEN);
					out_inidex += AMM_ADDR_LEN;
					memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.last_t, TIME_FRA_LEN);
					out_inidex += TIME_FRA_LEN;
				}
			}
			start_num++;
			temp_num--;
		}
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询电表信息
 * */
void DL3761_AFN16_13(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_inidex = 4;

	unsigned char amm[AMM_ADDR_LEN] = {0};
	int index = 0;
	StandNode node;
	//获取电表地址
	memcpy(amm, (rvframe3761->Frame376_1App.AppBuf + in_index), AMM_ADDR_LEN);
	in_index += AMM_ADDR_LEN;

	//获取电表索引
	index = SeekAmmAddr(amm, AMM_ADDR_LEN);
	//获取电表信息
	if(-1 == UpdateStandNode(index, &node))	//失败
	{
		//不存在
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	}
	else
	{
		//存在
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;

		//电表对应终端
		memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.Ter, TER_ADDR_LEN);
		out_inidex += TER_ADDR_LEN;

		//电表规约
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = node.type;

		//最后一次通信时间
		memcpy(snframe3761->Frame376_1App.AppBuf + out_inidex, node.last_t, TIME_FRA_LEN);
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询log日志开关状态
 * */
void DL3761_AFN16_14(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;

	//
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//打开log配置文件，获取当前记录状态
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_mutex_unlock(&_log_3762_cfg_mutex);
			return;
		}
	}
	//获取配置文件属性
	lseek(_cfg3762_fd, 0, SEEK_SET);
	read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(_log_cfg.flag == 0x00)
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 1;
	else
		snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询模块当前状态字
 * */
void DL3761_AFN16_15(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;
	unsigned char status = 0;

	//运行状态
	status = status | 0x01;

	//搜表状态  是否有终端正在搜表
	if(0 == seek_amm_task_empty())
		status = status & 0xFD;
	else
		status = status | 0x02;
	//
	pthread_mutex_lock(&_log_3762_cfg_mutex);
	if(_cfg3762_fd_flag != 0x66)
	{
		//打开log配置文件，获取当前记录状态
		_cfg3762_fd = open(LOG_3762_FILE_CFG, O_RDWR | O_CREAT, 0666);
		if(_cfg3762_fd < 0)
		{
			perror("open log config: ");
			pthread_mutex_unlock(&_log_3762_cfg_mutex);
			return;
		}
	}
	//获取配置文件属性
	lseek(_cfg3762_fd, 0, SEEK_SET);
	read(_cfg3762_fd, &_log_cfg, sizeof(struct log_config));
	if(_log_cfg.flag == 0x00)
		status = status | 0x04;
	else
		status = status & 0xFB;
	close(_cfg3762_fd);
	pthread_mutex_unlock(&_log_3762_cfg_mutex);

	//授权状态
	status = status | ((_hld_ac.get_status()) << 3);
	snframe3761->Frame376_1App.AppBuf[out_inidex++] = status;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = 0;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询模块当前USB状态
 * */
void DL3761_AFN16_16(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_inidex = 4;
	struct usb_status temp;

	get_usb_status(&temp);

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_in;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_log;

	snframe3761->Frame376_1App.AppBuf[out_inidex++] = temp.s_update;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_inidex;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:搜表结果
 * */
void DL3761_AFN16_20(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned short i = 0;
	unsigned short num = 0;
	struct seek_amm_result result;
	//获取终端总数量
	num = rvframe3761->Frame376_1App.AppBuf[in_index] + rvframe3761->Frame376_1App.AppBuf[in_index + 1] * 256;
	snframe3761->Frame376_1App.AppBuf[out_index] =  rvframe3761->Frame376_1App.AppBuf[in_index];
	snframe3761->Frame376_1App.AppBuf[out_index + 1] =  rvframe3761->Frame376_1App.AppBuf[in_index + 1];
	in_index += 2;
	out_index += 2;
	for(i = 0; i < num; i++)
	{
		//获取待查询搜表结果的终端
		memcpy(result.ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);
		in_index += TER_ADDR_LEN;
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), result.ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		//获取搜表结果
		if(0 == get_seek_amm_result(result.ter, &result))
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = (unsigned char)result.num;
		}
		else
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
		}
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:搜表结果详情
 * */
void DL3761_AFN16_21(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned char num = 0;
	unsigned char index = 0;
	unsigned char ter[TER_ADDR_LEN] = {0};
	unsigned char i = 0;
	struct seek_amm_result result;

	//获取终端地址
	memcpy(ter, (rvframe3761->Frame376_1App.AppBuf + in_index), TER_ADDR_LEN);
	in_index += TER_ADDR_LEN;
	//获取起始序号
	index = rvframe3761->Frame376_1App.AppBuf[in_index++];
	if(0 == index)
		index = 1;
	//获取个数
	num = rvframe3761->Frame376_1App.AppBuf[in_index++];

	if(0 == get_seek_amm_result(ter, &result))
	{
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		if((index + num - 1) > (unsigned char)result.num)
		{
			printf("index:    %d num:    %d NUM:   %d\n", index, num, result.num);
			if(index > (unsigned char)result.num)
			{
				snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
			}
			else
			{
				snframe3761->Frame376_1App.AppBuf[out_index++] = (unsigned char)result.num - index - 1;
				for(i = (index - 1); i < (unsigned char)result.num; i++)
				{
					memcpy((snframe3761->Frame376_1App.AppBuf + out_index),(unsigned char *)(result.amm + i), AMM_ADDR_LEN);
					out_index += AMM_ADDR_LEN;
				}
			}
		}
		else
		{
			snframe3761->Frame376_1App.AppBuf[out_index++] = num;
			for(i = (index - 1); i < (index - 1 + num); i++)
			{
				memcpy((snframe3761->Frame376_1App.AppBuf + out_index),(unsigned char *)(result.amm + i), AMM_ADDR_LEN);
				out_index += AMM_ADDR_LEN;
			}
		}

	}
	else
	{
		memcpy((snframe3761->Frame376_1App.AppBuf + out_index), ter, TER_ADDR_LEN);
		out_index += TER_ADDR_LEN;
		snframe3761->Frame376_1App.AppBuf[out_index++] = 0;
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:搜表终端数量查询
 * */
void DL3761_AFN16_22(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short out_index = 4;
	unsigned short num = 0;
	//获取搜表终端个数
	num = (unsigned short)get_seek_amm_task_num();
	printf("afn 16 22 num: %d\n",num);
	//
	snframe3761->Frame376_1App.AppBuf[out_index++] = num % 256;
	snframe3761->Frame376_1App.AppBuf[out_index++] = num / 256;

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能：搜表任务详细查询
 * */
void DL3761_AFN16_23(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
{
	unsigned short in_index = 4;
	unsigned short out_index = 4;
	unsigned short start_num = 0;
	unsigned short all_num = 0;
	unsigned char  num = 0;
	unsigned char  r_num;
	struct seek_amm_task task;
	//获取起始序号
	start_num = rvframe3761->Frame376_1App.AppBuf[in_index] + rvframe3761->Frame376_1App.AppBuf[in_index + 1] * 256;
	if(start_num == 0)
		start_num = 1;
	//获取读取数量
	num = rvframe3761->Frame376_1App.AppBuf[in_index + 2];

	//计算本次应答数量
	all_num = (unsigned short)get_seek_amm_task_num();
	if((num + start_num - 1) > all_num)
	{
		if(start_num > all_num)
		{
			r_num = 0;
		}
		else
		{
			r_num = all_num - start_num + 1;
		}
	}
	else
	{
		r_num = num;
	}

	//总数量
	snframe3761->Frame376_1App.AppBuf[out_index++] = all_num % 256;
	snframe3761->Frame376_1App.AppBuf[out_index++] = all_num / 256;

	//本次应答数量
	snframe3761->Frame376_1App.AppBuf[out_index++] = r_num;

	//终端地址
	while(0 != r_num)
	{
		if(0 != get_n_seek_amm_task(start_num, &task))
		{
			memset((snframe3761->Frame376_1App.AppBuf + out_index), 0xFF, TER_ADDR_LEN);
		}
		else
		{
			memcpy((snframe3761->Frame376_1App.AppBuf + out_index), task.ter, TER_ADDR_LEN);
		}
		out_index += TER_ADDR_LEN;
		start_num++;
		r_num--;
	}

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询授权失败原因
 * */
void DL3761_AFN16_24(tpFrame376_1 *snframe3761)
{
	unsigned short out_index = 4;
	//
	snframe3761->Frame376_1App.AppBuf[out_index++] = _hld_ac.get_status();

	//应用层帧长---不包括AFN/SEQ
	snframe3761->Frame376_1App.Len = out_index;

	snframe3761->IsHaving = true;
}

/*
 * 函数功能:查询参数
 * */
void DL3761_AFN16_Analy(tpFrame376_1 *rvframe3761, tpFrame376_1 *snframe3761)
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
	snframe3761->Frame376_1Link.CtlField.DIR = 1;	//上行
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

	//功能码
	snframe3761->Frame376_1App.AFN = AFN3761_EXTEND16;
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

	//数据标识
	snframe3761->Frame376_1App.AppBuf[0] = rvframe3761->Frame376_1App.AppBuf[0];
	snframe3761->Frame376_1App.AppBuf[1] = rvframe3761->Frame376_1App.AppBuf[1];
	snframe3761->Frame376_1App.AppBuf[2] = rvframe3761->Frame376_1App.AppBuf[2];
	snframe3761->Frame376_1App.AppBuf[3] = rvframe3761->Frame376_1App.AppBuf[3];

	//解析接受帧
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);

	switch(Fn)
	{
		case 5:
			DL3761_AFN16_05(snframe3761);
			break;
		case 6:
			DL3761_AFN16_06(snframe3761);
			break;
		case 7:
			DL3761_AFN16_07(rvframe3761, snframe3761);
			break;
		case 8:
			DL3761_AFN16_08(snframe3761);
			break;
		case 9:
			DL3761_AFN16_09(snframe3761);
			break;
		case 10:
			DL3761_AFN16_10(rvframe3761, snframe3761);
			break;
		case 11:
			DL3761_AFN16_11(rvframe3761, snframe3761);
			break;
		case 12:
			DL3761_AFN16_12(rvframe3761, snframe3761);
			break;
		case 13:
			DL3761_AFN16_13(rvframe3761, snframe3761);
			break;
		case 14:
			DL3761_AFN16_14(rvframe3761, snframe3761);
			break;
		case 15:
			DL3761_AFN16_15(rvframe3761, snframe3761);
			break;
		case 16:
			DL3761_AFN16_16(rvframe3761, snframe3761);
			break;
		case 20:
			DL3761_AFN16_20(rvframe3761, snframe3761);
			break;
		case 21:
			DL3761_AFN16_21(rvframe3761, snframe3761);
			break;
		case 22:
			DL3761_AFN16_22(rvframe3761, snframe3761);
			break;
		case 23:
			DL3761_AFN16_23(rvframe3761, snframe3761);
			break;
		case 24:
			DL3761_AFN16_24(snframe3761);
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
