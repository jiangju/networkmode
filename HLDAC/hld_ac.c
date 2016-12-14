/*
 * hld_ac.c
 *
 *  Created on: 2016��11��18��
 *      Author: j
 */
#define _HLD_AC_C_
#include "hld_ac.h"
#undef _HLD_AC_C_
#include "CommLib.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "DL376_1.h"
#include <string.h>

static HldAcStruct status;

/*
 * ��������:������������Ȩ
 * */
int open_hld_ac(void)
{
	//��ʼ��Ȩ״̬
	_hld_ac.init_status();

//	//������Ȩ�߳�
//	pthread_t pt;
//	if( 0 > pthread_create(&pt, NULL, _hld_ac.thread, NULL))
//	{
//		_hld_ac.set_status(1);
//		return -1;
//	}

	return 0;
}

/*
 * ��������:��ʼ����Ȩģ��
 * */
void init_hld_ac_mode(void)
{
	_hld_ac.init_status = init_hld_ac_mode_status;
	_hld_ac.get_status = get_hld_ac_mode_status;
	_hld_ac.set_status = set_hld_ac_status;

	_hld_ac.thread = HldAcPthread;
}

/*
 * ��������:��ʼ��ģ����Ȩ״̬
 * */
void init_hld_ac_mode_status(void)
{
	status.status = 0;
	pthread_mutex_init(&status.mutex, NULL);
}

/*
 * ��ȡ��ǰ��Ȩ״̬
 * */
char get_hld_ac_mode_status(void)
{
	char a;
	pthread_mutex_lock(&status.mutex);
	a = status.status;
	pthread_mutex_unlock(&status.mutex);
	return a;
}

/*
 * ���õ�ǰ��Ȩ״̬
 * */
void set_hld_ac_status(char in)
{
	pthread_mutex_lock(&status.mutex);
	status.status = in;
	pthread_mutex_unlock(&status.mutex);
}
///************************************************************************/

/*
 * ��������:��MD5���ܽ�������̶���ʽ����
 * ����:		md5		md5У��ֵ
 * 			outmd5	������ҵ�md5
 * */
void upset_md5(unsigned char *md5, unsigned char *outmd5)
{
	int i = 0;
	unsigned char buf[16] = {0,1,12,13,4,5,6,14,2,3,7,11,15,8,9,10};
	for(i = 0; i < 16; i++)
	{
		outmd5[i] = md5[buf[i]];
	}
}

/*
 * ��������:��ȡ������
 * */
int get_ac_condition(unsigned char *out, int *len)
{
	char buf[100] = {0};
	if(0 != get_mac(buf))
	{
		return -1;
	}
	memcpy(out, buf, 6);
	*len = 6;

	return 0;
}

/*
 * ��������:������Ȩ��У��
 * ����ֵ:0 У��ɹ�  -1ʧ��
 * */
int hld_ac_file_cs(void)
{
	int fd;
	int ret;
	unsigned char md5buf[16] = {0};
	int len = 0;
	unsigned char md5[16] = {0};
	unsigned char md5in[100] = {0};
	void *handle;
	int (*md5_operation)(unsigned char *, int, unsigned char *);

	/*��Ȩ��У��*/
	if(0 == access(HLD_AC_FILE, F_OK))
	{
		fd = open(HLD_AC_FILE, O_RDWR);
		if(fd < 0)
		{
			return -1;
		}
		else
		{
			ret = read(fd, md5buf, 16);
			if(ret != 16)
			{
				close(fd);
				return -1;
			}
			else
			{
				//��ȡ������
				if(0 != get_ac_condition(md5in, &len))
				{
					close(fd);
					return -1;
				}
				else	//����������
				{
					handle = dlopen(HLD_LIB, RTLD_NOW);
					if (handle == NULL)
					{
						fprintf(stderr, "Failed to open libaray %s error:%s\n", "libcryptopp.so", dlerror());
						close(fd);
						return -1;
					}
					md5_operation = (int (*)(unsigned char *, int, unsigned char *))dlsym(handle, "hld_md5_operation");
					if(NULL == md5_operation)
					{
						printf("dlsym erro : %s\n",dlerror());
						dlclose(handle);
						close(fd);
						return -1;
					}

					memcpy(md5in + len, "hello", 5);
					md5_operation(md5in, len + 5, md5);
					upset_md5(md5, md5in);
					if(1 != CompareUcharArray(md5in, md5buf, 16))
					{
						dlclose(handle);
						close(fd);
						return -1;
					}
				}
			}
		}
	}
	else
	{
		return -1;
	}
	close(fd);
	dlclose(handle);
	return 0;
}

/*
 * ��������:���������֤֡
 * ����:		outbuf	���֡
 * */
void create_afn18_01(tpFrame376_1 *outbuf)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));

	/************************************��·��************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//������
	outbuf->Frame376_1Link.EndChar = 0x16;
	//������
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//����
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//����վ
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCBλ��Ч
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//��ַ��
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************Ӧ�ò�****************************************/
	//������
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
	//����ȷ�ϱ�־
	outbuf->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//ʱ���ǩ
	outbuf->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//���ݱ�ʶ	0x1
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;

	outbuf->Frame376_1App.AppBuf[index++] = 0x01;

	//����������Ҫpw��Ĭ������Ϊ16��0
	memset((outbuf->Frame376_1App.AppBuf + index), 0x00, 16);
	index += 16;

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * ��������:����֤��
 * */
int request_p1(int s)
{
	tpFrame376_1 sndbuf;
	tp3761Buffer outbuf;
	int ret = 0;
	//������վ֤��
	create_afn18_01(&sndbuf);
	if(0 != DL3761_Protocol_LinkPack(&sndbuf, &outbuf)) //��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
		return -1;
	ret = write(s, outbuf.Data, outbuf.Len);
	if(ret != outbuf.Len)
		return -1;
	return 0;
}

/*
 * ��������:��ȡ֤��
 * ����:		s	�׽���
 * 			r1	�����
 * 			r1len	����
 * ����ֵ  0 �ɹ�  -1 ʧ��
 * */
int read_p1(int s, unsigned char *r1, unsigned char *r1len)
{
	if(s < 0 || r1 == NULL || r1len == NULL)
		return -1;
	unsigned char buf[1024] = {0};
	int ret = 0;
	unsigned char p1[1024] = {0};	//��Կ
	unsigned short r = 0;
	unsigned short w = 0;
	tpFrame376_1 rvdbuf;
	tp3761Buffer inbuf;
	int index = 4;
	int ticker = 4;
	//��ȡ֤��
	while(ticker--)
//	while(1)
	{
		sleep(1);
		memset(buf, 0, 1024);
		r = 0;
		w = 0;
		ret = read(s, buf, 1024);
		if(ret < 0)
		{
	//		pr_err("read");
			continue;
		}
		w = ret + 1;
		ret = ProtoAnaly_Get376_1BufFromCycBuf(buf, 1024, &r, &w, ZHU, &inbuf);
		if(ret == -1)
		{
			continue;
		}
		DL376_1_LinkFrame(&inbuf, &rvdbuf);
		if(0x18 != rvdbuf.Frame376_1App.AFN || (0x01 != rvdbuf.Frame376_1App.AppBuf[2] || 0x00 != rvdbuf.Frame376_1App.AppBuf[3]))
		{
			continue;
		}
		break;
	}

	if(0 > ticker)
		return -1;	//��ʱ

	r = rvdbuf.Frame376_1App.AppBuf[index] + rvdbuf.Frame376_1App.AppBuf[index+1] * 256;	//��Կ����
	index += 2;
	w = rvdbuf.Frame376_1App.AppBuf[index++];	//���������
	*r1len = w;
	memcpy(p1, rvdbuf.Frame376_1App.AppBuf + index, r);	//֤��

	int fd = open(HLD_PUB, O_RDWR | O_CREAT | O_TRUNC, 0666);	//����֤���ļ�
	if(fd < 0)
	{
		perror("open pub");
		return -1;
	}
	if(0 > write(fd, p1, r))
	{
		close(fd);
		perror("write");
		return -1;
	}
	close(fd);
	index += r;
	memcpy(r1, rvdbuf.Frame376_1App.AppBuf + index, w);	//�����

	return 0;
}

/*
 * ��������:���������r2
 * ����:
 * 		r2		�����
 * ����ֵ 0 �ɹ� -1ʧ��
 * */
void creat_r2(unsigned char *r2)
{
	int i = 0;
	for(i = 0; i < 8; i++)
		r2[i] = get_rng();
}

/*
 * ��������:������ԿЭ�̱���
 * ����:		outbuf		����õı��Ľ��
 * 			buf			((R1)r2 + R2)p1
 * 			l1			(R1)r2����
 * 			l2			R2����
 * 			l3			((R1)r2 + R2)p1����
 * */
void creat_afn18_fn02(tpFrame376_1 *outbuf, unsigned char *buf, unsigned char l1, unsigned char l2, unsigned short l3)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));

	/************************************��·��************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//������
	outbuf->Frame376_1Link.EndChar = 0x16;
	//������
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//����
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//����վ
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCBλ��Ч
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//��ַ��
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************Ӧ�ò�****************************************/
	//������
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
	//����ȷ�ϱ�־
	outbuf->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//ʱ���ǩ
	outbuf->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//���ݱ�ʶ	0x2
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x02;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	//�豸����
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	//(R1)r2 + R2����
	outbuf->Frame376_1App.AppBuf[index++] = l1;
	//R2����
	outbuf->Frame376_1App.AppBuf[index++] = l2;
	//((R1)r2 + R2)p1����
	outbuf->Frame376_1App.AppBuf[index++] = l3 % 256;
	outbuf->Frame376_1App.AppBuf[index++] = l3 / 256;
	//((R1)r2 + R2)p1
	memcpy(outbuf->Frame376_1App.AppBuf + index, buf, l3);

	index += l3;

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * ��������:��ԿЭ��
 * ����:		s	�׽���
 * 			R1	�����R1
 * 			r1len 	����
 * 			r2	��������R2
 * ����ֵ	0 �ɹ� -1 ʧ��
 * */
int afn18_fn02(int s, unsigned char *r1, unsigned char r1len, unsigned char *r2)
{
	if(r1len > 100 || r1 == NULL)
		return -1;
	tpFrame376_1 snbuf;
	tp3761Buffer outbuf;
	unsigned char l1;
	unsigned short l2;
	int i = 0;
	void *handle;
	void (*rsa_en)(const char *, const char *, const char *, char *, int *);	//rsa����
	int (*des_en)(unsigned char *, int , unsigned char *, unsigned char *);		//des����
	unsigned char des_buf[100] = {0};
	char str[200] = {0};
	unsigned char rsa_buf[2048] = {0};
	unsigned char R1[100] = {0};
	memcpy(R1, r1, r1len);

	creat_r2(r2);

	/*������ԿЭ�̱���*/
	handle = dlopen(HLD_LIB, RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "Failed to open libaray %s error:%s\n", "libcryptopp.so", dlerror());
		return -1;
	}

	//��ü��ܽӿ�
	rsa_en = (void (*)(const char *, const char *, const char *, char *, int *))dlsym(handle, "hld_rsa_encrypt");
	if(NULL == rsa_en)
	{
		dlclose(handle);
		return -1;
	}
	des_en = (int (*)(unsigned char *, int , unsigned char *, unsigned char *))dlsym(handle, "hld_des_encryption");
	if(NULL == des_en)
	{
		dlclose(handle);
		return -1;
	}

	//��R1����R2Ϊkey��DES����
	for(i = 0; i < (r1len / 8 + ((r1len % 8) != 0)); i++)
		des_en(r2, 8, (R1 + 8*i), (des_buf + 8 * i));
	l1 = i * 8;
	//��(R1)r2 + R2����RSA����
	memcpy(des_buf + l1, r2, 8);
	for(i = 0; i < (l1 + 8); i++)
		sprintf((str + i*2), "%02X", des_buf[i]);
//	printf("des_buf :  %s\n",des_buf);
	rsa_en(HLD_PUB, (char *)r1, (char *)str, (char *)rsa_buf, (int *)&l2);
//	printf("en: %s\n",rsa_buf);
	creat_afn18_fn02(&snbuf, rsa_buf, l1 + 8, 8, strlen((char *)rsa_buf));

	/*������ԿЭ��*/
	if(0 != DL3761_Protocol_LinkPack(&snbuf, &outbuf)) //��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
	{
		dlclose(handle);
		return -1;
	}
	int ret = write(s, outbuf.Data, outbuf.Len);
	if(ret != outbuf.Len)
	{
		dlclose(handle);
		return -1;
	}

	dlclose(handle);
	return 0;
}

/*
 * ��������:Э�̽��
 * */
int wait_result(int s)
{
	int i = 0;
	unsigned short w = 0;
	unsigned short r = 0;
	int ret = 0;
	unsigned char buf[1024] = {0};
	tpFrame376_1 rvdbuf;
	tp3761Buffer inbuf;
	int ticker = 4;
	while(ticker--)
//	while(1)
	{
		sleep(1);
		memset(buf, 0, 1024);
		ret = read(s, buf, 1024);
		if(ret < 0)
		{
			continue;
		}

		for(i = 0; i < ret; i++)
			printf("%02x ",buf[i]);
		printf("\n");

		w = ret + 1;
		ret = ProtoAnaly_Get376_1BufFromCycBuf(buf, 1024, &r, &w, ZHU, &inbuf);
		if(ret == -1)
		{
			continue;
		}
		DL376_1_LinkFrame(&inbuf, &rvdbuf);
		if(0x00 != rvdbuf.Frame376_1App.AFN )
		{
			continue;
		}
		if((0x01 != rvdbuf.Frame376_1App.AppBuf[2] || 0x00 != rvdbuf.Frame376_1App.AppBuf[3]))
			return -1;

		return 0;
	}

	return -1;
}

/*
 * ��������:�����֤
 * ����:		s	socket
 * 			R2	�����R2
 * ����ֵ: -1 ʧ��  0�ɹ�
 * */
int hld_ac_id_discern(int s, unsigned char *r2)
{
	unsigned char R1[100] = {0};
	unsigned char l1 = 0;
	//������վ֤��
	printf("request_p1\n");
	if( 0 != request_p1(s))
	{
		printf("request_p1 erro\n");
		return -1;
	}

	//��ȡ֤��
	printf("read_p1\n");
	if(0 != read_p1(s, R1, &l1))
	{
		printf("read_p1 erro\n");
		return -1;
	}

	//��ԿЭ��
	printf("afn18_fn02\n");
	if(0 != afn18_fn02(s, R1, l1, r2))
	{
		printf("afn18_fn02 erro\n");
		return -1;
	}

	//�ȴ�Э�̽���
	printf("wait_result\n");
	if(0 != wait_result(s))
	{
		printf("wait_result erro\n");
		return -1;
	}

	return 0;
}

/*
 * ��������:����afn18_fn03֡��ʽ����
 * ����:		snbuf	֡��ʽ����
 * 			r2		�����
 * ����ֵ: 0��  -1ʧ��
 * */
int creat_afn18_fn03(tpFrame376_1 *outbuf, unsigned char *r2)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));
	unsigned char a[256] = {0};
	unsigned char buf[256] = {0};
	int a_len = 0;
	int (*des_en)(unsigned char *, int , unsigned char *, unsigned char *);		//des����
	void *handle = NULL;
	int i = 0;

	/************************************��·��************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//������
	outbuf->Frame376_1Link.EndChar = 0x16;
	//������
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//����
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//����վ
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCBλ��Ч
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//��ַ��
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************Ӧ�ò�****************************************/
	//������
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
	//����ȷ�ϱ�־
	outbuf->Frame376_1App.SEQ.CON = 0;	//����Ҫȷ��
	//֡����
	outbuf->Frame376_1App.SEQ.FIR_FIN = FRM_FIRFIN_SING;
	//֡���
	outbuf->Frame376_1App.SEQ.PSEQ_RSEQ = _SendNum++;
	//ʱ���ǩ
	outbuf->Frame376_1App.SEQ.TPV = 0;	//��Ҫʱ��
	//������ ʱ��
	outbuf->Frame376_1App.AUX.AUXTP.flag = 0;	//��Ч
	outbuf->Frame376_1App.AUX.AUXEC.flag = 0;
	//���ݱ�ʶ	0x3
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x04;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	//�豸����
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;

	//���ܺ�������볤��  ��  ������
	if(0 != get_ac_condition(a, &a_len))	//��ȡ������
	{
		return -1;
	}
	printf("MAC : %02x %02x %02x %02x %02x %02x\n",a[0],a[1],a[2],a[3],a[4],a[5]);

	handle = dlopen(HLD_LIB, RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "Failed to open libaray %s error:%s\n", "libcryptopp.so", dlerror());
		return -1;
	}
	des_en = (int (*)(unsigned char *, int , unsigned char *, unsigned char *))dlsym(handle, "hld_des_encryption");
	if(NULL == des_en)
	{
		dlclose(handle);
		return -1;
	}
	for(i = 0; i < (a_len / 8 + ((a_len % 8) != 0)); i++)
			des_en(r2, 8, (a + 8*i), (buf + 8 * i));
	outbuf->Frame376_1App.AppBuf[index++] = 8 * i;	//���ܺ� �������볤��
	memcpy(outbuf->Frame376_1App.AppBuf + index, buf, 8 * i);	//���ܺ������
	index += (8 * i);

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;

	dlclose(handle);
	return 0;
}

/*
 * ��������:������Ȩ
 * ������	s	�׽���
 * 			r2	�����R2
 * 			len	R2����
 * ����ֵ: 0�ɹ� -1ʧ��
 * */
int afn18_fn03(int s, unsigned char *r2)
{
	if(r2 == NULL)
		return -1;
	tpFrame376_1 snbuf;
	tp3761Buffer outbuf;

	if(0 != creat_afn18_fn03(&snbuf, r2))
	{
		return -1;
	}

	if(0 != DL3761_Protocol_LinkPack(&snbuf, &outbuf)) //��3761��ʽ����ת��Ϊ�ɷ��Ͷ���������
	{
		return -1;
	}

	int ret = write(s, outbuf.Data, outbuf.Len);
	if(ret != outbuf.Len)
		return -1;

	return 0;
}

/*
 * ��������:��ȡ��ȨӦ��
 * ����:		s	�׽���
 * 			r2	�����
 * ����ֵ:	0�ɹ� -1��ʱ -2ע��ʧ�� -3 δע�� -4��Ȩ���ʽ���� -5 ����
 * */
int read_afn18_fn03(int s, unsigned char *r2)
{
	unsigned short w = 0;
	unsigned short r = 0;
	int ret = 0;
	unsigned char buf[1024] = {0};
	unsigned char buf_buf[1024] = {0};
	unsigned char md5_buf[16] = {0};
	tpFrame376_1 rvdbuf;
	tp3761Buffer inbuf;
	void *handle = NULL;
	int (*des_dn)(unsigned char *, int , unsigned char *, unsigned char *);

	int index = 4;
	int ticker = 4;
	while(ticker--)
//	while(1)
	{
		sleep(1);
		memset(buf, 0, 1024);
		ret = read(s, buf, 1024);
		if(ret < 0)
		{
			continue;
		}

		w = ret + 1;
		ret = ProtoAnaly_Get376_1BufFromCycBuf(buf, 1024, &r, &w, ZHU, &inbuf);
		if(ret == -1)
		{
			continue;
		}
		DL376_1_LinkFrame(&inbuf, &rvdbuf);
		if(0x00 != rvdbuf.Frame376_1App.AFN )
		{
			continue;
		}
		if(0x18 != rvdbuf.Frame376_1App.AFN || 0x04 != rvdbuf.Frame376_1App.AppBuf[2] || 0x00 != rvdbuf.Frame376_1App.AppBuf[3])
		{
			continue;
		}

		//��ȡ�ɹ���־
		unsigned char flag = rvdbuf.Frame376_1App.AppBuf[index++];
		switch(flag)
		{
			case 1:
				break;
			case 2:
				return -2;	//ע��ʧ��
			case 3:
				return -3;	//δע��
			default:
				return -5;	//����
		}

		unsigned char m_len = rvdbuf.Frame376_1App.AppBuf[index++];	//���ܺ����Ȩ�볤��
		if(m_len != 16)
		{
			return -4;	//��Ȩ���ʽ����
		}
		memset(buf, 0, 1024);
		memcpy(buf, rvdbuf.Frame376_1App.AppBuf + index, 16);

		handle = dlopen(HLD_LIB, RTLD_NOW);
		if (handle == NULL)
		{
			fprintf(stderr, "Failed to open libaray %s error:%s\n", "libcryptopp.so", dlerror());
			return -5;
		}
		des_dn = (int (*)(unsigned char *, int , unsigned char *, unsigned char *))dlsym(handle, "hld_des_dncryption");
		if(NULL == des_dn)
		{
			dlclose(handle);
			return -5;
		}

		//des����
		des_dn(r2, 8, buf, buf_buf);
		des_dn(r2, 8, buf+8, buf_buf+8);
		dlclose(handle);

		upset_md5(buf_buf, md5_buf);	//���Ҽ��ܺ��������

		int fd = open(HLD_AC_FILE, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if(fd < 0)
		{
			return -5;
		}
		if(16 != write(fd, md5_buf, 16))
			return  -5;
		close(fd);
		return 0;
	}
	return -1;
}

/*
 * ��������:��Ȩ��֤
 * ����:	  s		�׽���
 * 		  r2	�����
 * ����ֵ:0�ɹ� -1��ʱ -2ע��ʧ�� -3 δע�� -4��Ȩ���ʽ���� -5 ����
 * */
int hld_ac_impower(int s, unsigned char *r2)
{
	//������Ȩ
	if(0 != afn18_fn03(s, r2))
	{
		return -5;
	}

	//��ȨӦ��
	return read_afn18_fn03(s, r2);
}

/*
 * ��Ȩ�����߳�
 * */
void *HldAcPthread(void *arg)
{
	sleep(30);		//����30S
	pthread_detach(pthread_self());	//�̷߳���

	char ipstr[20] = {0};
	unsigned short port = 0;

	if(0 != hld_ac_file_cs())	//��Ȩ��ƥ��ʧ��
	{
		//����������Ȩ		������Ȩ����ͻ���
		int s = socket(PF_INET, SOCK_STREAM, 0);
		if(s < 0)
		{
			perror("hld ac socket:");
			_hld_ac.set_status(1);	//����
			pthread_exit(NULL);
		}

		//��ȡip���˿�
		pthread_mutex_lock(&_RunPara.mutex);
		sprintf(ipstr, "%d.%d.%d.%d", _RunPara.server.ip[0],_RunPara.server.ip[1], \
					_RunPara.server.ip[2],_RunPara.server.ip[3]);
		port = _RunPara.server.port;
		pthread_mutex_unlock(&_RunPara.mutex);

		struct sockaddr_in addr = {
			.sin_family	= PF_INET,
			.sin_port	= htons(port),
			.sin_addr = {
				.s_addr = inet_addr(ipstr),
			},
		};

		socklen_t addrlen = sizeof(addr);
		_hld_ac.set_status(1);	//����ʧ��
		if(0 > connect(s, (struct sockaddr *)&addr, addrlen)){
			perror("connect");
			pthread_exit(NULL);
		}

		//�����׽��ַ�����
		int flag = fcntl(s, F_GETFL);
		if(0 > flag){
			perror("fcntl");
			pthread_exit(NULL);
		}

		flag |= O_NONBLOCK;

		if(0 > fcntl(s, F_SETFL, flag)){
			perror("fcntl-1");
			pthread_exit(NULL);
		}

		//�����֤
		_hld_ac.set_status(2);	//�����֤ʧ��
		unsigned char R2[8] = {0};
		if(0 != hld_ac_id_discern(s, R2))
		{
			close(s);
			pthread_exit(NULL);
		}
		int i = 0;
		printf("R2: ");
		for(i = 0; i < 8 ; i++)
			printf("%02x ", R2[i]);
		printf("\n");
		//��Ȩ��֤
		switch(hld_ac_impower(s, R2))
		{
			case 0:
				break;
			case -1:
				close(s);
				_hld_ac.set_status(-5);
				pthread_exit(NULL);
			case -2:
				close(s);
				_hld_ac.set_status(-3);
				pthread_exit(NULL);
			case -3:
				close(s);
				_hld_ac.set_status(-4);
				pthread_exit(NULL);
			case -4:
				close(s);
				_hld_ac.set_status(-6);
				pthread_exit(NULL);
			case -5:
				close(s);
				_hld_ac.set_status(1);
				pthread_exit(NULL);
			default:
				close(s);
				_hld_ac.set_status(1);
				pthread_exit(NULL);
		}

		close(s);

		if(0 != hld_ac_file_cs())
		{
			_hld_ac.set_status(1);
			pthread_exit(NULL);
		}

		_hld_ac.set_status(0);
	}
	else						//��Ȩ��ƥ��ɹ�
	{
		_hld_ac.set_status(0);
	}

	pthread_exit(NULL);
}
