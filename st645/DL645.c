/*
 * DL645.c
 *
 *  Created on: 2016年7月24日
 *      Author: j
 */

#include "DL645.h"
#include <stdio.h>
#include <string.h>
#include "CommLib.h"

/*
 * 函数功能:构造645帧
 * */
int Create645From(tpFrame645 * buf645, Buff645 *outbuf)
{
	if(NULL == buf645)
		return -1;
	unsigned char index = 0;
	memset(outbuf, 0, DL645_DATALENGTH);

	outbuf->buf[index++] = 0x68;
	memcpy((outbuf->buf + index), buf645->Address, AMM_ADDR_LEN);
	index += AMM_ADDR_LEN;
	outbuf->buf[index++] = 0x68;
	outbuf->buf[index++] = buf645->CtlField;
	outbuf->buf[index++] = buf645->Length;
	memcpy((outbuf->buf + index), buf645->Datas, buf645->Length);
	index += buf645->Length;
	outbuf->buf[index++] = Func_CS(outbuf->buf, buf645->Length + 10);
	outbuf->buf[index++] = 0x16;
	outbuf->len = index;
	return 0;
}

/*
 * 函数功能:解析数据判断645帧
 * 参数:	buf
 * 		len	buf的长度
 * */
int ProtoAnaly645BufFromCycBuf(unsigned char *buf, int len, tpFrame645 * buf645)
{
	int index = 0;
	unsigned char LL = 0;
	unsigned char  CS = 0;
	int i = 0;
	int beginchar = 0;
	if(len < 12)
		return -1;
	memset(buf645, 0, sizeof(tpFrame645));
	while(index <= len)
	{
		if(buf[index] != 0x68)
		{
			index++;
		}
		else
		{
			if(buf[index + 7] != 0x68)
			{
				index = index + 8;
			}
			else
			{
				beginchar = index;
				memcpy(buf645->Address, (buf + index + 1), AMM_ADDR_LEN);
				index += 8;
				buf645->CtlField = buf[index++];
				buf645->Length = buf[index++];
				LL = 1 + 6 + 1 + 1 + 1 + buf645->Length + 2;

				if (buf[LL - 1] != 0x16)
				{
					return -1;
				}
				CS = buf[LL - 2];
				for(i = 0; i < buf645->Length; i++)
				{
					buf645->Datas[i] = buf[index++];
				}
				if(CS == Func_CS(buf + beginchar, LL - 2))
				{
					return 0;
				}
				return -1;
			}
		}
	}
	return 0;
}
