/*
 * CommLib.c
 *
 *  Created on: 2016��6��24��
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
 * ��������:hex ת bcd
 * ����:	hex	hex
 * ����ֵ:bcd
 * */
unsigned char HexToBcd(unsigned char hex)
{
	return (hex % 10 + (hex / 10 * 16));
}

/*
 * ��������:bcd ת hex
 * ����	bcd		bcd
 * ����ֵ:	hex
 * */
unsigned char BcdToHex(unsigned char bcd)
{
	return (bcd % 16 + (bcd / 16 * 10));
}

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

/*
 * ��������:���ļ�A�������ļ�B
 * ����:	A	�ļ�A
 * 		B	�ļ�B
 * ����ֵ 0�ɹ� -1ʧ��
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
 * ��������:CRC 16У��
 * ����:	puchMsg		����
 * 		usDataLen	���ݳ���
 * 		CRCHandL	CRC��ʼֵ
 * ����ֵ:У����
 * */
unsigned short HLDCrc16(unsigned char *puchMsg, unsigned short usDataLen, unsigned short CRCHandL)
{
    unsigned char uchCRCHi = (unsigned char)(CRCHandL / 0x100); 			/*CRC���ֽڳ�ʼ��*/
    unsigned char uchCRCLo = (unsigned char)(CRCHandL % 0x100); 			/*CRC���ֽڳ�ʼ��*/
    unsigned char uIndex ;					/*CRCѭ������*/
    while (usDataLen--)						/*������Ϣ������*/
    {
        uIndex = uchCRCHi ^ *puchMsg++;		/*����CRC*/
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }

    return (uchCRCHi << 8 | uchCRCLo);
}

/*
 * ��������:�����ļ�CRCУ����
 * ����:	fname 	�ļ���
 * 		crc		������
 * ����ֵ: 0����ɹ� -1 ʧ��
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
 * ��������:�ַ�ת��������
 * ������ c	�����ַ�
 * ����ֵ: -1 ��������		>= 0 ת����
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
 * ��������:��ȡMAC��ַ
 * ���� mac	���ص�MAC
 * ����ֵ��0 �ɹ� -1 ʧ��
 * */
int get_mac(char * mac)    //����ֵ��ʵ��д��char * mac���ַ�������������'\0'��
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
