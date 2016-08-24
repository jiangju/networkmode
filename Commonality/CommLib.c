/*
 * CommLib.c
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */

#include "CommLib.h"
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * 函数功能:将Fn转换为数据单元格式
 * 参数:	fn	FN
 * 		dt	数据单元格式数据
 * 返回值:无
 * */
void FNtoDT(unsigned char fn, unsigned char *dt)
{
	unsigned char bit = 0;
	*(dt + 1) = fn / 8;
	bit = fn % 8;
	*(dt + 0) = 0x01 << (bit - 1);
}

/*
 * 函数功能:将数据单元表示转换为Fn
 * 参数:数据单元标识
 * 返回值:-1 错误  0~7 bit0 ~ bit7
 * */
char DTtoFN(unsigned char *DT)
{
	char i = 7;

	while(i >= 0)
	{
		if((DT[0] >> i) & 0x01)
		{
			break;
		}
		i--;
	}

	if(i < 0)
	{
		return -1;
	}
	else
	{
		return (DT[1] * 8 + i + 1);
	}
}

/*
 * 函数功能:获取CS校验
 * 参数:	buf		需要校验的buf
 * 		index	校验的起始位置
 * 		len		校验的长度
 * 返回值:CS校验码
 * */
unsigned char Get_CS(unsigned char *buf, unsigned short index, unsigned short len)
{
	unsigned char CS = 0;
	unsigned short i = 0;
	for(i = 0; i < len; i++)
	{
		CS += *(buf + index + i);
	}
	return CS;
}

/*
 * 函数功能:向文件写数据
 * 参数:	fd		文件描述符
 * 		offset	写入的文件位置
 * 		buff	写入的数据
 * 		len		写入的长度
 * 返回值: 0 成功  -1失败
 * */
int WriteFile(int fd, int offset, void *buff, int len)
{
	int res = 0;
	int i = 3;
	int ret = 0;
	while(i--)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if (ret == offset)
		{
			ret = write(fd, buff, len);
			if (len == ret) {
				break;
			}
		}
	}

	res = i < 0 ? -1 : 0;
	return res;
}

/*
 * 函数功能:读取文件数据
 * 参数:	fd		文件描述符
 * 		offset	读取的位置
 * 		buff	读取后的存储位置
 * 		len		读取的长度
 * 返回值:0 成功	-1 失败
 * */
int ReadFile(int fd, int offset, void *buff, int len)
{
	int test, ret;
	test = 3;
	while (test--)
	{
		ret = lseek(fd, offset, SEEK_SET);
		if (ret == offset)
		{
			ret = read(fd, buff, len);
			if (len == ret)
			{
				break;
			}
		}
	}
	test = test < 0 ? -1 : 0;
	return test;
}

/*
 * 函数功能:设置网络
 * 参数:	ifname	网卡名
 * 		ipaddr	ip地址
 * 		netmask	子网掩码
 * 		gwip	网关
 * 返回值	0成功	-1失败
 * */
int IfConfigEthnert(const char *ifname, const char *ipaddr,const char *netmask, const char *gwip)
{
	char command[100] = { 0 };
	char gw[50] = { 0 };
	sprintf(command, "ifconfig %s %s netmask %s", ifname, ipaddr, netmask);
	sprintf(gw, "route add default gw %s %s", gwip, ifname);
	system(command);
	system(gw);
	sleep(3);
	return (0);
}

/*
 * 函数功能:计算CS校验和
 * 参数:	inbuf  		待计算缓存
 * 		inBuflen	需计算的长度
 * 返回值: 计算结果
 * */
unsigned char Func_CS(void *inBuf,unsigned short inBufLen)
{
	unsigned short i=0;
	unsigned char uCS=0;
	unsigned char *buffer = (unsigned char *)inBuf;

	for(i=0;i<inBufLen;i++)
	{
		uCS += (unsigned char)buffer[i];
	}
	return uCS;
}

/*
 * 函数功能:比较无符号char型数组
 * 参数:	A	数组A
 * 		B	数组B
 * 		len	比较的长度
 * 返回值 0 A>B -1 A<B 1 A = B
 * */
int CompareUcharArray(unsigned char *A, unsigned char *B, int len)
{
	int i = 0;
	for(i = 0; i < len; i++)
	{
		if(A[len - 1 - i] > B[len - 1 - i])
		{
			return 0;
		}
		else if(A[len - 1 - i] < B[len - 1 - i])
		{
			return -1;
		}
	}
	return 1;
}
