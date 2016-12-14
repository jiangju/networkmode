/*
 * DL3761_AFN10.c
 *
 *  Created on: 2016年7月25日
 *      Author: j
 */
#include "DL376_1.h"
#include <stdio.h>
#include <string.h>
#include "SysPara.h"
#include <pthread.h>
#include "DL3762_AFN06.h"
#include "DL376_2_DataType.h"
#include "CommLib.h"
#include "DL376_2.h"
#include "StandBook.h"
#include "Collect.h"
#include "DL645.h"
#include "HLDUsart.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "NetWork1.h"
#include <stdlib.h>
#include "DL3762_AFN02.h"
#include "DL3762_AFN13.h"
#include "TopBuf.h"
#include "TopRoute.h"
#include "hld_ac.h"

//#include <sys/time.h>
/*
 * 函数功能:构造透明转发格式帧
 * 参数:	ter		终端地址
 * 		inbuf	待转发的数据
 * 		len		待转发的数据长度
 * 		outbuf	输出的格式帧
 * 		flag	从动站/启动站
 * */
void Create3761AFN10_01(unsigned char *ter, unsigned char *inbuf, int len,  unsigned char flag, tpFrame376_1 *outbuf)
{
	int index = 0;
	int i = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));
	/************************************链路层************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	outbuf->Frame376_1Link.EndChar = 0x16;
	//控制域
	if(flag == 0)
		outbuf->Frame376_1Link.CtlField.DIR = 1;	//上行
	else
		outbuf->Frame376_1Link.CtlField.DIR = 0;	//下行

	outbuf->Frame376_1Link.CtlField.PRM = flag;	//从动站
	outbuf->Frame376_1Link.CtlField.FCV = 0;	//FCB位无效
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	if(flag == 0)
		outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x08;	//功能码
	else
		outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//地址域
	outbuf->Frame376_1Link.AddrField.WardCode[0] = ter[0];
	outbuf->Frame376_1Link.AddrField.WardCode[1] = ter[1];
	outbuf->Frame376_1Link.AddrField.Addr[0] = ter[2];
	outbuf->Frame376_1Link.AddrField.Addr[1] = ter[3];
	if(flag == 0)
		outbuf->Frame376_1Link.AddrField.MSA = 0;
	else
		outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************应用层****************************************/
	//功能码
	outbuf->Frame376_1App.AFN = AFN3761_DATASEND;
	//请求确认标志
	outbuf->Frame376_1App.SEQ.CON = 0;	//不需要确认
	//帧类型
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//帧序号
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//时间标签
	outbuf->Frame376_1App.SEQ.TPV = 0;	//不要时标
	//附加域 时标
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//无效
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//数据标识
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;

	if(flag == 0)	//从动站(响应充值终端)
	{
		//通讯端口号
		outbuf->Frame376_1App.AppBuf[index++] = 0x01;
		//透明转发内容字节数
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)((unsigned int)len & 0x000000FF);
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)(((unsigned int)len & 0x0000FF00) >> 8);
		//透明转发内容
		for(i = 0; i < len; i++)
		{
			outbuf->Frame376_1App.AppBuf[index++] = inbuf[i];
		}
	}
	else			//主动站(发送给三网合一模块采集数据)
	{
		//终端通信端口号
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//透明转发通信控制字
		outbuf->Frame376_1App.AppBuf[index++] = 0x6B;
		//透明转发接收等待报文超时时间
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//透明转发接收等待字节超时时间
		outbuf->Frame376_1App.AppBuf[index++] = 0x02;
		//透明转发内容字节数
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)((unsigned int)len & 0x000000FF);
		outbuf->Frame376_1App.AppBuf[index++] = (unsigned char)(((unsigned int)len & 0x0000FF00) >> 8);
		//透明转发内容
		for(i = 0; i < len; i++)
		{
			outbuf->Frame376_1App.AppBuf[index++] = inbuf[i];
		}
	}
	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * 函数功能:广播数据给充值终端
 * 参数:	inbuf	数据
 * 		len		长度
 * 返回值: 0 成功  -1  失败
 * */
int broadcast_buf_topup(unsigned char *inbuf, int len)
{
	if(0 >= get_hld_top_node_num())
	{
		return -1;
	}

	if(0 != _hld_ac.get_status())	//未授权时，不进行数据抄收
	{
		return 0;
	}
	unsigned char ter[TER_ADDR_LEN] = {0};
	tpFrame376_1 outbuf;
	tp3761Buffer snbuf;
	int i = 1;
	while(0 == get_hld_top_node_ter(i, ter))
	{
		usleep(1);
		i++;
		//构造透明转发帧结构
		Create3761AFN10_01(ter, inbuf, len, 0, &outbuf);
		//将3761格式数据转换为可发送二进制数据
		DL3761_Protocol_LinkPack(&outbuf, &snbuf);
		send_hld_top_node_ter_data(ter, snbuf.Data, snbuf.Len);
	}

	return 0;
}

///*
// * 函数功能:判断是否是充值终端需要的数据标识
// * 参数:	dadt	数据标识
// * 返回值: 0 需要  -1  不需要
// * */
//int top_jude_dadt(unsigned char *dadt)
//{
//	int i = 0;
//	for(i = 0; i < TOP_FIX_DADT_NUM; i++)
//	{
//		if(1 == CompareUcharArray((unsigned char *)_top_fix_dadt[i], dadt, DADT_LEN))
//		{
//			return 0;
//		}
//	}
//
//	return -1;
//}

/*
 * 函数功能:透明转发
 * */
void DL3761_AFN10_01(tpFrame376_1 *rvframe3761)
{
	int len = 0;
	int index = 4;	//rvframe3761->Frame376_1App.AppBuf前4个字节是数据标识
	unsigned char buf[300] = {0};
	tpFrame376_2 snframe3762;
	tp3762Buffer tpbuffer;
	tpFrame645 frame645;
	tpFrame376_1 snframe3761;
	tp3761Buffer ttpbuffer;

	CollcetStatus c_status;
	StandNode node;
	unsigned char ter[4] = {0};
	int ret = 0;
	int is_true = -1;
	//判断来自充值终端还是电表的三网合一
	if(rvframe3761->Frame376_1Link.CtlField.PRM == 0)	//从动站（电表）
	{
		index++;
		len = rvframe3761->Frame376_1App.AppBuf[index + 1] * 256 + rvframe3761->Frame376_1App.AppBuf[index];
		index += 2;

		memcpy(buf, (rvframe3761->Frame376_1App.AppBuf + index), len);
		if(0 == ProtoAnaly645BufFromCycBuf(buf, len, &frame645))
		{
			//查看内存中的台账是否有该电表
			ret = SeekAmmAddr(frame645.Address, AMM_ADDR_LEN);
			if(ret < 0)
			{
				return;
			}

			//更改电表最后通信时间
			UpdateNodeDate(ret);

			//获取电表内容
			if(0 > GetStandNode(ret, &node))
			{
				return;
			}
			//比较台账中电表对应的终端地址，如果终端地址不一致则更新终端地址
			ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

			//终端地址与台账中的终端地址不同
			if(1 != CompareUcharArray(node.Ter, ter, TER_ADDR_LEN))
			{
				memcpy(node.Ter, ter, TER_ADDR_LEN);
				UpdateStandNode(ret, &node);
				AlterNodeStandFile(&node);
			}

			//判断充值任务
			if(1 == GetTaskaKey())
			{
				//根据表号获取节点
				struct task_a_node *node_p = SeekTaskANode1(frame645.Address);
				if(NULL == node_p)
					return;
				if(1 == CompareUcharArray(node_p->task_status.dadt, frame645.Datas, 4) || (frame645.CtlField & 0x40) != 0)
				{
					Create3761AFN10_01(node_p->task_status.top_ter, buf, len,  0, &snframe3761);
				}
				else
				{
					return;
				}
				//将3761格式数据转换为可发送二进制数据
				DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
				send_hld_top_node_ter_data(node_p->task_status.top_ter, ttpbuffer.Data, ttpbuffer.Len);
//				struct timeval tv;
//				gettimeofday(&tv, NULL);
//				printf("send top up :  %ld;  ms:  %ld\n",tv.tv_sec, (tv.tv_usec / 1000));
				//任务结束
				DeleTaskA(node_p->amm, 0);
				return;
			}

			GetCollectorStatus(&c_status);
			//比较表号  实时任务  周期任务 需要比较表号
			if(1 == CompareUcharArray(c_status.amm, frame645.Address, AMM_ADDR_LEN))
			{
				if(0x02 == node.type)
				{
					if((frame645.CtlField & 0x40) != 0)	//异常应答帧
					{
						is_true = 0;
					}
					else if(1 == CompareUcharArray(c_status.dadt, frame645.Datas, 4))
					{
						is_true = 0;
					}
				}
				else
				{
					if((frame645.CtlField & 0x40) != 0)	//异常应答帧
					{
						is_true = 0;
					}
					else if(1 == CompareUcharArray(c_status.dadt, frame645.Datas, 2))	//正常帧  比较数据标识
					{
						is_true = 0;
					}
				}

				if(0 == is_true)
				{
					switch(c_status.runingtask)
					{
						case 'a':

							break;
						case 'b':
							//构造376.2 AFN13上报数据
							Create3762AFN13_01(c_status.amm, _Collect.taskb.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);

								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
//								pthread_mutex_lock(&_Collect.taskb.taskb_mutex);
								DeleNearTaskB();
//								pthread_mutex_unlock(&_Collect.taskb.taskb_mutex);
							}
							break;
						case 'c':
							//构造 376.2 AFN06上报数据
							Create3762AFN06_02((unsigned short)ret, node.type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
//								_Collect.flag = 0;
//								_Collect.taskc.isok = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
							}
							break;
						case 'd':
							break;
						case 'e':
							//构造376.2 AFN02上报数据
							Create3762AFN02_01(c_status.amm, _Collect.taske.next->type, buf, len, &snframe3762);
							if(0 == DL3762_Protocol_LinkPack(&snframe3762, &tpbuffer))
							{
								pthread_mutex_lock(&writelock);
								UsartSend(Usart0Fd, tpbuffer.Data, tpbuffer.Len);
								pthread_mutex_unlock(&writelock);
								c_status.isend = 1;
								memset(c_status.amm, 0, AMM_ADDR_LEN);
								memset(c_status.dadt, 0, DADT_LEN);
								c_status.conut = 0;
//								_Collect.flag = 0;
								c_status.runingtask = 0;
								SetCollectorStatus(&c_status);
//								pthread_mutex_lock(&_Collect.taske.taske_mutex);
								DeleNearTaskE();
//								pthread_mutex_unlock(&_Collect.taske.taske_mutex);
							}
							break;
					}

//					//抄收到充值终端需要的数据时，广播给所有在线的充值终端
//					if(0 == top_jude_dadt(c_status.dadt))
//					{
//						broadcast_buf_topup(buf, len);
//					}
				}
			}
		}
	}
	else												//启动站（充值终端）
	{
		unsigned char top_ter[TER_ADDR_LEN] = {0};
		//前4字节不解析
		index += 4;
		//透明转发内容字节数
		len = rvframe3761->Frame376_1App.AppBuf[index + 1] * 256 + rvframe3761->Frame376_1App.AppBuf[index];
		index += 2;
		memcpy(buf, (rvframe3761->Frame376_1App.AppBuf + index), len);
		if(0 == ProtoAnaly645BufFromCycBuf(buf, len, &frame645))
		{
			top_ter[0] = rvframe3761->Frame376_1Link.AddrField.WardCode[0];
			top_ter[1] = rvframe3761->Frame376_1Link.AddrField.WardCode[1];
			top_ter[2] = rvframe3761->Frame376_1Link.AddrField.Addr[0];
			top_ter[3] = rvframe3761->Frame376_1Link.AddrField.Addr[1];

			ret = AddTaskA(frame645.Address, frame645.Datas, buf, len, top_ter);
			if(0 != ret)
			{
				tpFrame645 f645;
				Buff645 b645;

				memcpy(f645.Address, frame645.Address, AMM_ADDR_LEN);
				f645.CtlField = 0xD1;
				printf("add top up task err %d\n", ret);
				switch (ret)
				{
					case -1:
						f645.Datas[0] = 0xFD;
						break;
					case -2:
						f645.Datas[0] = 0xFF;
						break;
					case -3:
						f645.Datas[0] = 0xFC;
						break;
					default:
						break;
				}
				f645.Datas[0] += 0x33;
				f645.Length = 1;

				if(0 == Create645From(&f645, &b645))
				{
					Create3761AFN10_01(top_ter, b645.buf, b645.len, 0, &snframe3761);
					//将3761格式数据转换为可发送二进制数据
					DL3761_Protocol_LinkPack(&snframe3761, &ttpbuffer);
					send_hld_top_node_ter_data(top_ter, ttpbuffer.Data, ttpbuffer.Len);
				}
			}
		}
	}
}

/*
 * 函数功能:数据转发(接受抄收电表的数据)
 * 参数:	rvframe3761	接受的数据
 * */
void DL3761_AFN10_Analy(tpFrame376_1 *rvframe3761)
{
	unsigned char DT[2] = {0};
	char Fn = 0;
	DT[0] = rvframe3761->Frame376_1App.AppBuf[2];
	DT[1] = rvframe3761->Frame376_1App.AppBuf[3];
	Fn = DTtoFN(DT);
	switch (Fn)
	{
		case 1:
			DL3761_AFN10_01(rvframe3761);
			break;
		case 9:
			break;
		case 12:
			break;
		case 13:
			break;
		default:
			break;
	}
	memset(rvframe3761, 0, sizeof(tpFrame376_1));
}
