/*
 * DL376_2.c
 *
 *  Created on: 2016年6月21日
 *      Author: Administrator
 */
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <HLDUsart.h>
#include <limits.h>
#include <pthread.h>
#include "DL376_2.h"
#include "DL376_2_DataType.h"
#include "DL3762_AFN01.h"
#include "DL3762_AFN02.h"
#include "DL3762_AFN03.h"
#include "DL3762_AFN04.h"
#include "DL3762_AFN05.h"
#include "DL3762_AFN10.h"
#include "DL3762_AFN11.h"
#include "DL3762_AFN12.h"
#include "DL3762_AFN13.h"
#include "DL3762_AFN14.h"
#include "CommLib.h"
/*
 * 函数功能:校验376.2接受报文是否正确
 * 参数:挖掘过后的376.2数据
 * 返回值:0 成功 -1 失败
 * */
char ProtoAnaly_Effic376_2Frame(tp3762Buffer *buffer)
{
	unsigned short i = 0;
	unsigned char CS = 0;
	unsigned char C = 0;
	//校验CS
	for(i = 3; i < buffer->Len - 3; i++)
	{
		CS += buffer->Data[i];
	}

	if(CS != buffer->Data[buffer->Len - 2])
	{
		return -1;
	}

	//报文传输方向
	C = buffer->Data[3];
	if(C & 0x80)
	{
		return -1;
	}
	return 0;
}

/*
 * 函数功能:解析收到的数据是否有完整的376.2数据帧
 * 参数:Usart0RvBuffer 串口1的接受缓存
 * 		tpBuffer  解析后得到的376.2数据帧
 * 返回值:0成功解析 -1解析失败
 * */
char ProtoAnaly_Get376_2BufFromCycBuf(Usart0RvBuffer *rvbuffer, tp3762Buffer *buffer)
{
	unsigned short r = 0;
	unsigned short w = 0;
	unsigned short l = 0;
	unsigned short Len = 0;
	unsigned short i = 0;
	if(rvbuffer->ReadIndex == rvbuffer->WriteIndex)
	{
		return -1;
	}
	memset(buffer, 0x00, sizeof(tp3762Buffer));

	while(rvbuffer->ReadIndex != rvbuffer->WriteIndex)
	{
		if(rvbuffer->DataBuffer[rvbuffer->ReadIndex] != 0x68)
		{
			rvbuffer->ReadIndex++;
			rvbuffer->ReadIndex %= USART0_RV_DATA_LEN;
			continue;
		}
		else
		{
			r = rvbuffer->ReadIndex;
			w = rvbuffer->WriteIndex;
			//判断可读帧的长度
			if(w > r)
			{
				l = w - r;
			}else if(r > w)
			{
				l = w + USART0_RV_DATA_LEN - r;
			}
			else
			{
				return -1;
			}
			if(l < 6)
			{
				return -1;
			}

			Len = rvbuffer->DataBuffer[(r+1)%USART0_RV_DATA_LEN];
			Len = 256 * rvbuffer->DataBuffer[(r+2)%USART0_RV_DATA_LEN] + Len;

			//接受的数据长度大于缓存数据长度
			if(Len > USART0_RV_DATA_LEN)
			{
				rvbuffer->ReadIndex = rvbuffer->WriteIndex;
//				printf("l1 %x\n",rvbuffer->DataBuffer[(r+1)%USART0_RV_DATA_LEN]);
//				printf("l2 %x\n",rvbuffer->DataBuffer[(r+2)%USART0_RV_DATA_LEN]);
//				printf("len more than max len\n");
				return -1;
			}

			//已接受的数据小于将要接受数据的长度
			if(Len > l)
			{
//				printf("rev need delay\n");
				return -1;
			}

			//判断结尾符是否为0x16
			if(0x16 != rvbuffer->DataBuffer[(Len + r - 1)%USART0_RV_DATA_LEN])
			{
				rvbuffer->ReadIndex++;
				rvbuffer->ReadIndex %= USART0_RV_DATA_LEN;
//				printf("end is not 0x68\n");
				return -1;
			}

			for(i = 0; i < Len; i++)
			{
				buffer->Data[i] = rvbuffer->DataBuffer[r];
				r++;
				r = r%USART0_RV_DATA_LEN;
			}
			buffer->Len = Len;

			if(-1 == ProtoAnaly_Effic376_2Frame(buffer))
			{
				rvbuffer->ReadIndex += Len;
				rvbuffer->ReadIndex %= USART0_RV_DATA_LEN;
//				printf("cs erro\n");
				return -1;
			}

			rvbuffer->ReadIndex += Len;
			rvbuffer->ReadIndex %= USART0_RV_DATA_LEN;
			return 0;
		}
	}

	return -1;
}

/*
 *函数功能:将接受的376.2数据转换为定义的376.2格式
 *参数:tpBuffer	待转换的数据缓存
 *		tpFrame376_2	转换后数据的存储缓存
 *返回值:无
 **/
void DL376_2_LinkFrame(tp3762Buffer *inBuffer, tpFrame376_2 *frame3762)
{
	unsigned short index = 0;
	unsigned short i = 0;
	if(inBuffer->Len < FRMLEN_EMPTY2)
	{
		return;
	}

	memset(frame3762, 0, sizeof(tpFrame376_2));

	/***************************链路层********************************/
	//起始字符
	frame3762->Frame376_2Link.BeginChar = inBuffer->Data[index++];
	if(frame3762->Frame376_2Link.BeginChar != BEGINCHAR)
	{
		return;
	}
//	printf("begin %x\n",frame3762->Frame376_2Link.BeginChar);

	//结尾字符
	frame3762->Frame376_2Link.Endchar = inBuffer->Data[inBuffer->Len - 1];
	if(frame3762->Frame376_2Link.Endchar != ENDCHAR)
	{
		return;
	}
//	printf("end %x\n",frame3762->Frame376_2Link.Endchar);

	//长度
	frame3762->Frame376_2Link.Len = inBuffer->Data[index] + inBuffer->Data[index + 1] * 256;
	if(frame3762->Frame376_2Link.Len != inBuffer->Len)
	{
		return;
	}
	index += 2;
//	printf("len %x\n",frame3762->Frame376_2Link.Len);

	//控制域
	frame3762->Frame376_2Link.CtlField.DIR = (inBuffer->Data[index] & 0x80) >> 7;
	if(0 != frame3762->Frame376_2Link.CtlField.DIR)	//非下行报文则退出
	{
		return;
	}
	frame3762->Frame376_2Link.CtlField.PRM = (inBuffer->Data[index] & 0x40) >> 6;
	frame3762->Frame376_2Link.CtlField.CommMode = inBuffer->Data[index] & 0x3F;
	index++;
//	printf("DIR %x\n",frame3762->Frame376_2Link.CtlField.DIR);
//	printf("PRM %x\n",frame3762->Frame376_2Link.CtlField.PRM);
//	printf("CommMode %x\n",frame3762->Frame376_2Link.CtlField.CommMode);

	//CS
	frame3762->Frame376_2Link.CS = inBuffer->Data[inBuffer->Len - 2];
//	printf("CS %x\n",frame3762->Frame376_2Link.CS);

	/****************************应用层***********************************/
	//R信息域
	frame3762->Frame376_2App.R.RelayRank = (inBuffer->Data[index] & 0xF0) >> 4;
	frame3762->Frame376_2App.R.ConflictDetection = (inBuffer->Data[index] & 0x08) >> 3;
	frame3762->Frame376_2App.R.CommModeIdentifying = (inBuffer->Data[index] & 0x04) >> 2;
	frame3762->Frame376_2App.R.AffiliateIdentifying = (inBuffer->Data[index] & 0x02) >> 1;
	frame3762->Frame376_2App.R.RouteIdentifying = (inBuffer->Data[index] & 0x01);
	index++;

	frame3762->Frame376_2App.R.ErrEecoveryIdentifying = (inBuffer->Data[index] & 0xF0) >> 4;
	frame3762->Frame376_2App.R.ChannelIdentifying = (inBuffer->Data[index] & 0x0F);
	index++;

	frame3762->Frame376_2App.R.PredictAnswerLen = inBuffer->Data[index++];

	frame3762->Frame376_2App.R.SpeedIdentifying = (inBuffer->Data[index] & 0x80) >> 7;
	frame3762->Frame376_2App.R.CommSpeed = (inBuffer->Data[index++] & 0x70);
	frame3762->Frame376_2App.R.CommSpeed = 256*frame3762->Frame376_2App.R.CommSpeed + inBuffer->Data[index++];

	frame3762->Frame376_2App.R.MessageSerialNumber = inBuffer->Data[index++];

//	printf("RelayRank %x\n",frame3762->Frame376_2App.R.RelayRank);
//	printf("ConflictDetection %x\n",frame3762->Frame376_2App.R.ConflictDetection);
//	printf("CommModeIdentifying %x\n",frame3762->Frame376_2App.R.CommModeIdentifying);
//	printf("AffiliateIdentifying %x\n",frame3762->Frame376_2App.R.AffiliateIdentifying);
//	printf("RouteIdentifying %x\n",frame3762->Frame376_2App.R.RouteIdentifying);
//	printf("ErrEecoveryIdentifying %x\n",frame3762->Frame376_2App.R.ErrEecoveryIdentifying);
//	printf("ChannelIdentifying %x\n",frame3762->Frame376_2App.R.ChannelIdentifying);
//	printf("PredictAnswerLen %x\n",frame3762->Frame376_2App.R.PredictAnswerLen);
//	printf("SpeedIdentifying %x\n",frame3762->Frame376_2App.R.SpeedIdentifying);
//	printf("CommSpeed %x\n",frame3762->Frame376_2App.R.CommSpeed);
//	printf("MessageSerialNumber %x\n",frame3762->Frame376_2App.R.MessageSerialNumber);

	//地址与A
	if(frame3762->Frame376_2App.R.CommModeIdentifying != 0)
	{
		memcpy(frame3762->Frame376_2App.Addr.SourceAddr, (inBuffer->Data + index), ADDR_LEN * sizeof(char));
		index += ADDR_LEN;
		memcpy(frame3762->Frame376_2App.Addr.RelayAddr, (inBuffer->Data + index),
				frame3762->Frame376_2App.R.RelayRank * ADDR_LEN * sizeof(char));
		index += (ADDR_LEN * frame3762->Frame376_2App.R.RelayRank);
		memcpy(frame3762->Frame376_2App.Addr.DestinationAddr, (inBuffer->Data + index), ADDR_LEN * sizeof(char));
		index += ADDR_LEN;
	}

	//功能码
	frame3762->Frame376_2App.AppData.AFN = inBuffer->Data[index++];
//	printf("AFN %x\n",frame3762->Frame376_2App.AppData.AFN);

	//数据长度
	if(frame3762->Frame376_2App.R.CommModeIdentifying == 0)//通讯模块标识为0时 无地址域
	{
		frame3762->Frame376_2App.AppData.Len = inBuffer->Len - 13;//总长度-链路长度-信息域长度 - AFN
	}
	else
	{
		frame3762->Frame376_2App.AppData.Len = inBuffer->Len - 13 - 12
		- frame3762->Frame376_2App.R.RelayRank * ADDR_LEN ;//
	}
//	printf("data Len %d\n",frame3762->Frame376_2App.AppData.Len);

	//数据内容
	for(i = 0; i < frame3762->Frame376_2App.AppData.Len; i++)
	{
		frame3762->Frame376_2App.AppData.Buffer[i] = inBuffer->Data[index++];
//		printf(" %x",frame3762->Frame376_2App.AppData.Buffer[i]);
	}
//	printf("\n");
	//解析完成  有帧待处理
	frame3762->IsHaving = true;
}

/*
 * 函数功能:处理376.2请求
 * */
void DL3762_Process_Request(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762)
{
//	printf("rev 376.2 ok  AFN  %x\n",rvframe3762->Frame376_2App.AppData.AFN);
	switch(rvframe3762->Frame376_2App.AppData.AFN)
	{
		case AFN_REST:
			DL3762_AFN01_Analy(rvframe3762, snframe3762);
			break;
		case AFN_DATASEND:
			DL3762_AFN02_Analy(rvframe3762, snframe3762);
			break;
		case AFN_QUERDATA:
			DL3762_AFN03_Analy(rvframe3762, snframe3762);
			break;
		case AFN_PORTDETE:
			DL3762_AFN04_Analy(rvframe3762, snframe3762);
			break;
		case AFN_CTRL:
			DL3762_AFN05_Analy(rvframe3762, snframe3762);
			break;
		case AFN_ROUTEQUERY:
			DL3762_AFN10_Analy(rvframe3762, snframe3762);
			break;
		case AFN_ROUTESET:
			DL3762_AFN11_Analy(rvframe3762, snframe3762);
			break;
		case AFN_ROUTECTRL:
			DL3762_AFN12_Analy(rvframe3762, snframe3762);
			break;
		case AFN_ROUTESEND:
			DL3762_AFN13_Analy(rvframe3762, snframe3762);
			break;
		case AFN_ROUTERCV:
			DL3762_AFN14_Analy(rvframe3762);
			break;
//		case AFN_FILETRAN:
//			break;
		default :
			memset(rvframe3762, 0, sizeof(tpFrame376_2));
			break;
	}
}

/*
 * 函数功能:将376.2格式数据转换为可以发送的二进制数据
 * 参数:	snframe3762	待转换的数据
 * 		tpbuffer	转换后的数据
 * 返回值: -1 失败  0成功
 * */
char DL3762_Protocol_LinkPack(tpFrame376_2 *snframe3762, tp3762Buffer *tpbuffer)
{
	unsigned short 	index = 0;
	unsigned char 	temp = 0;
	unsigned short 	i = 0;

	if(true == snframe3762->IsHaving)
	{
		memset(tpbuffer, 0, sizeof(tp3762Buffer));

		//起始字符
		tpbuffer->Data[index++] = 0x68;

		//长度
		temp = (unsigned char)(snframe3762->Frame376_2Link.Len & 0x00FF);
		tpbuffer->Data[index++] = temp;
		temp = (unsigned char)((snframe3762->Frame376_2Link.Len & 0xFF00) >> 8);
		tpbuffer->Data[index++] = temp;

		//控制域
		temp = (snframe3762->Frame376_2Link.CtlField.DIR & 0x01) << 7;
		temp |= (snframe3762->Frame376_2Link.CtlField.PRM & 0x01) << 6;
		temp |= (snframe3762->Frame376_2Link.CtlField.CommMode & 0x3F);
		tpbuffer->Data[index++] = temp;

		//R信息域
		temp = (snframe3762->Frame376_2App.R.RelayRank & 0x0F) << 4;
		temp |= (snframe3762->Frame376_2App.R.CommModeIdentifying & 0x01) << 2;
		temp |= (snframe3762->Frame376_2App.R.RouteIdentifying & 0x01);
		tpbuffer->Data[index++] = temp;

		temp = 0x00;
		temp |= (snframe3762->Frame376_2App.R.ChannelIdentifying & 0x0F);
		tpbuffer->Data[index++] = temp;

		temp = 0x00;//电表通道特征  		实测象限标识
		tpbuffer->Data[index++] = temp;

		temp = 0x00;//末级应答信号品质	末级命令信号品质
		tpbuffer->Data[index++] = temp;

		temp = 0x00;//预留			事件标志
		tpbuffer->Data[index++] = temp;

		temp = snframe3762->Frame376_2App.R.MessageSerialNumber;
		tpbuffer->Data[index++] = temp;

		//地址域
		if((snframe3762->Frame376_2App.R.CommModeIdentifying & 0x01) == 1)
		{
			memcpy((tpbuffer->Data + index), snframe3762->Frame376_2App.Addr.SourceAddr, ADDR_LEN);
			index += ADDR_LEN;
			memcpy((tpbuffer->Data + index), snframe3762->Frame376_2App.Addr.RelayAddr,
					snframe3762->Frame376_2App.R.RelayRank * ADDR_LEN);
			index += snframe3762->Frame376_2App.R.RelayRank * ADDR_LEN;
			memcpy((tpbuffer->Data + index), snframe3762->Frame376_2App.Addr.DestinationAddr, ADDR_LEN);
			index += ADDR_LEN;
		}

		//功能码
		tpbuffer->Data[index++] = snframe3762->Frame376_2App.AppData.AFN;

		//数据内容
		for(i = 0; i < snframe3762->Frame376_2App.AppData.Len; i++)
		{
			tpbuffer->Data[index++] = snframe3762->Frame376_2App.AppData.Buffer[i];
		}

		//CS校验码
		temp = Get_CS(tpbuffer->Data, 3, (index - 3));
		tpbuffer->Data[index++] = temp;

		//结束字符
		tpbuffer->Data[index++] = 0x16;

		tpbuffer->Len = index;

		return 0;
	}
	return -1;
}

