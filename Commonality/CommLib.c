/*
 * CommLib.c
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */
#define COMMONALITY_COMMLIB_C_
#include "CommLib.h"
#undef 	COMMONALITY_COMMLIB_C_

#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
/*
 * 函数功能:hex 转 bcd
 * 参数:	hex	hex
 * 返回值:bcd
 * */
unsigned char HexToBcd(unsigned char hex)
{
	return (hex % 10 + (hex / 10 * 16));
}

/*
 * 函数功能:bcd 转 hex
 * 参数	bcd		bcd
 * 返回值:	hex
 * */
unsigned char BcdToHex(unsigned char bcd)
{
	return (bcd % 16 + (bcd / 16 * 10));
}

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

/*
 * 函数功能:将文件A拷贝到文件B
 * 参数:	A	文件A
 * 		B	文件B
 * 返回值 0成功 -1失败
 * */
int HLDFileCopy(char *A, char *B)
{
	FILE *fp0 = NULL;
	FILE *fp1 = NULL;

	fp0 = fopen(A, "rb");
	if(NULL == fp0)
	{
		perror("fopen A:");
		return -1;
	}

	fp1 = fopen(B, "wb");
	if(NULL == fp1)
	{
		perror("fopen B:");
		return -1;
	}

	int num = 1024;
	char buf[1028] = {0};
	while(1)
	{
		num = fread(buf, 1, 1024, fp0);
//		printf("fread num  %d\n",num);
		if(num < 0)
			return -1;
		fwrite(buf, 1, num, fp1);
		if(num != 1024)
		{
			break;
		}
		memset(buf, 0, 1028);
	}
	fclose(fp0);
	fclose(fp1);

	return 0;
}

/*
 * 函数功能:CRC 16校验
 * 参数:	puchMsg		数据
 * 		usDataLen	数据长度
 * 		CRCHandL	CRC初始值
 * 返回值:校验结果
 * */
unsigned short HLDCrc16(unsigned char *puchMsg, unsigned short usDataLen, unsigned short CRCHandL)
{
    unsigned char uchCRCHi = (unsigned char)(CRCHandL / 0x100); 			/*CRC高字节初始化*/
    unsigned char uchCRCLo = (unsigned char)(CRCHandL % 0x100); 			/*CRC低字节初始化*/
    unsigned char uIndex ;					/*CRC循环索引*/
    while (usDataLen--)						/*传输消息缓冲区*/
    {
        uIndex = uchCRCHi ^ *puchMsg++;		/*计算CRC*/
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }

    return (uchCRCHi << 8 | uchCRCLo);
}

/*
 * 函数功能:计算文件CRC校验码
 * 参数:	fname 	文件名
 * 		crc		计算结果
 * 返回值: 0计算成功 -1 失败
 * */
int get_file_crc(char *fname, unsigned short *crc)
{
	printf("fname : %s\n",fname);

	FILE *fp = NULL;
	fp = fopen(fname, "r");
	if(NULL == fp)
	{
		perror("open wait crc file:");
		return -1;
	}

	int ret = 0;
	unsigned char buf[1028] = {0};
	unsigned short temp = 0xFFFF;
	while(1)
	{
		ret = fread(buf, 1, 1024, fp);
		if(ret < 0)
			return -1;
		temp = HLDCrc16(buf, (unsigned short)ret, temp);
		if(ret != 1024)
			break;
	}
	*crc = temp;
	printf("*crc=  %x\n",*crc);
	fclose(fp);
	return 0;
}

/*
 * 函数功能:字符转换整数字
 * 参数： c	输入字符
 * 返回值: -1 输入有误		>= 0 转换后
 * */
char chars_to_char(char c)
{
	char i = -1;

	if(c >= '0' && c <= '9')
	{
		i = c - '0';
	}
	else if (c >= 'a' && c <= 'f')
	{
		i = c -'a' + 10;
	}
	else if (c >= 'A' && c <= 'F')
	{
		i = c -'A' + 10;
	}
	else
	{
		i = -1;
	}
	return i;
}
/*
 * 函数功能:获取MAC地址
 * 参数 mac	返回的MAC
 * 返回值：0 成功 -1 失败
 * */
int get_mac(char * mac)    //返回值是实际写入char * mac的字符个数（不包括'\0'）
{
    struct ifreq ifq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifq.ifr_name, "eth0");    //Currently, only get eth0

    if (ioctl (sock, SIOCGIFHWADDR, &ifq) < 0)
    {
        perror ("ioctl");
        return -1;
    }

	memcpy(mac, ifq.ifr_hwaddr.sa_data, 6);

    return 0;
}
