/*
 * CommLib.c
 *
 *  Created on: 2016��6��24��
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
 * ��������:��Fnת��Ϊ���ݵ�Ԫ��ʽ
 * ����:	fn	FN
 * 		dt	���ݵ�Ԫ��ʽ����
 * ����ֵ:��
 * */
void FNtoDT(unsigned char fn, unsigned char *dt)
{
	unsigned char bit = 0;
	*(dt + 1) = fn / 8;
	bit = fn % 8;
	*(dt + 0) = 0x01 << (bit - 1);
}

/*
 * ��������:�����ݵ�Ԫ��ʾת��ΪFn
 * ����:���ݵ�Ԫ��ʶ
 * ����ֵ:-1 ����  0~7 bit0 ~ bit7
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
 * ��������:��ȡCSУ��
 * ����:	buf		��ҪУ���buf
 * 		index	У�����ʼλ��
 * 		len		У��ĳ���
 * ����ֵ:CSУ����
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
 * ��������:���ļ�д����
 * ����:	fd		�ļ�������
 * 		offset	д����ļ�λ��
 * 		buff	д�������
 * 		len		д��ĳ���
 * ����ֵ: 0 �ɹ�  -1ʧ��
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
 * ��������:��ȡ�ļ�����
 * ����:	fd		�ļ�������
 * 		offset	��ȡ��λ��
 * 		buff	��ȡ��Ĵ洢λ��
 * 		len		��ȡ�ĳ���
 * ����ֵ:0 �ɹ�	-1 ʧ��
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
 * ��������:��������
 * ����:	ifname	������
 * 		ipaddr	ip��ַ
 * 		netmask	��������
 * 		gwip	����
 * ����ֵ	0�ɹ�	-1ʧ��
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
 * ��������:����CSУ���
 * ����:	inbuf  		�����㻺��
 * 		inBuflen	�����ĳ���
 * ����ֵ: ������
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
 * ��������:�Ƚ��޷���char������
 * ����:	A	����A
 * 		B	����B
 * 		len	�Ƚϵĳ���
 * ����ֵ 0 A>B -1 A<B 1 A = B
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
