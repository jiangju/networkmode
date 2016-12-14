/*
 * hld_ac.c
 *
 *  Created on: 2016年11月18日
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
 * 函数功能:开启华立达授权
 * */
int open_hld_ac(void)
{
	//初始授权状态
	_hld_ac.init_status();

//	//开启授权线程
//	pthread_t pt;
//	if( 0 > pthread_create(&pt, NULL, _hld_ac.thread, NULL))
//	{
//		_hld_ac.set_status(1);
//		return -1;
//	}

	return 0;
}

/*
 * 函数功能:初始化授权模块
 * */
void init_hld_ac_mode(void)
{
	_hld_ac.init_status = init_hld_ac_mode_status;
	_hld_ac.get_status = get_hld_ac_mode_status;
	_hld_ac.set_status = set_hld_ac_status;

	_hld_ac.thread = HldAcPthread;
}

/*
 * 函数功能:初始化模块授权状态
 * */
void init_hld_ac_mode_status(void)
{
	status.status = 0;
	pthread_mutex_init(&status.mutex, NULL);
}

/*
 * 获取当前授权状态
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
 * 设置当前授权状态
 * */
void set_hld_ac_status(char in)
{
	pthread_mutex_lock(&status.mutex);
	status.status = in;
	pthread_mutex_unlock(&status.mutex);
}
///************************************************************************/

/*
 * 函数功能:将MD5加密结果，按固定方式打乱
 * 参数:		md5		md5校验值
 * 			outmd5	输出打乱的md5
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
 * 函数功能:获取特征码
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
 * 函数功能:本地授权码校验
 * 返回值:0 校验成功  -1失败
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

	/*授权码校验*/
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
				//获取特征码
				if(0 != get_ac_condition(md5in, &len))
				{
					close(fd);
					return -1;
				}
				else	//加密特征码
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
 * 函数功能:构造身份认证帧
 * 参数:		outbuf	输出帧
 * */
void create_afn18_01(tpFrame376_1 *outbuf)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));

	/************************************链路层************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	outbuf->Frame376_1Link.EndChar = 0x16;
	//控制域
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//上行
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//启动站
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCB位无效
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//地址域
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************应用层****************************************/
	//功能码
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
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
	//数据标识	0x1
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;

	outbuf->Frame376_1App.AppBuf[index++] = 0x01;

	//控制命令需要pw域，默认设置为16个0
	memset((outbuf->Frame376_1App.AppBuf + index), 0x00, 16);
	index += 16;

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * 函数功能:请求证书
 * */
int request_p1(int s)
{
	tpFrame376_1 sndbuf;
	tp3761Buffer outbuf;
	int ret = 0;
	//请求主站证书
	create_afn18_01(&sndbuf);
	if(0 != DL3761_Protocol_LinkPack(&sndbuf, &outbuf)) //将3761格式数据转换为可发送二进制数据
		return -1;
	ret = write(s, outbuf.Data, outbuf.Len);
	if(ret != outbuf.Len)
		return -1;
	return 0;
}

/*
 * 函数功能:读取证书
 * 参数:		s	套接字
 * 			r1	随机数
 * 			r1len	长度
 * 返回值  0 成功  -1 失败
 * */
int read_p1(int s, unsigned char *r1, unsigned char *r1len)
{
	if(s < 0 || r1 == NULL || r1len == NULL)
		return -1;
	unsigned char buf[1024] = {0};
	int ret = 0;
	unsigned char p1[1024] = {0};	//公钥
	unsigned short r = 0;
	unsigned short w = 0;
	tpFrame376_1 rvdbuf;
	tp3761Buffer inbuf;
	int index = 4;
	int ticker = 4;
	//读取证书
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
		return -1;	//超时

	r = rvdbuf.Frame376_1App.AppBuf[index] + rvdbuf.Frame376_1App.AppBuf[index+1] * 256;	//公钥长度
	index += 2;
	w = rvdbuf.Frame376_1App.AppBuf[index++];	//随机数长度
	*r1len = w;
	memcpy(p1, rvdbuf.Frame376_1App.AppBuf + index, r);	//证书

	int fd = open(HLD_PUB, O_RDWR | O_CREAT | O_TRUNC, 0666);	//生成证书文件
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
	memcpy(r1, rvdbuf.Frame376_1App.AppBuf + index, w);	//随机数

	return 0;
}

/*
 * 函数功能:生成随机数r2
 * 参数:
 * 		r2		随机数
 * 返回值 0 成功 -1失败
 * */
void creat_r2(unsigned char *r2)
{
	int i = 0;
	for(i = 0; i < 8; i++)
		r2[i] = get_rng();
}

/*
 * 函数功能:构造秘钥协商报文
 * 参数:		outbuf		构造好的报文结果
 * 			buf			((R1)r2 + R2)p1
 * 			l1			(R1)r2长度
 * 			l2			R2长度
 * 			l3			((R1)r2 + R2)p1长度
 * */
void creat_afn18_fn02(tpFrame376_1 *outbuf, unsigned char *buf, unsigned char l1, unsigned char l2, unsigned short l3)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));

	/************************************链路层************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	outbuf->Frame376_1Link.EndChar = 0x16;
	//控制域
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//上行
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//启动站
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCB位无效
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//地址域
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************应用层****************************************/
	//功能码
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
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
	//数据标识	0x2
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x02;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	//设备类型
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;
	//(R1)r2 + R2长度
	outbuf->Frame376_1App.AppBuf[index++] = l1;
	//R2长度
	outbuf->Frame376_1App.AppBuf[index++] = l2;
	//((R1)r2 + R2)p1长度
	outbuf->Frame376_1App.AppBuf[index++] = l3 % 256;
	outbuf->Frame376_1App.AppBuf[index++] = l3 / 256;
	//((R1)r2 + R2)p1
	memcpy(outbuf->Frame376_1App.AppBuf + index, buf, l3);

	index += l3;

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;
}

/*
 * 函数功能:秘钥协商
 * 参数:		s	套接字
 * 			R1	随机数R1
 * 			r1len 	长度
 * 			r2	输出随机数R2
 * 返回值	0 成功 -1 失败
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
	void (*rsa_en)(const char *, const char *, const char *, char *, int *);	//rsa加密
	int (*des_en)(unsigned char *, int , unsigned char *, unsigned char *);		//des加密
	unsigned char des_buf[100] = {0};
	char str[200] = {0};
	unsigned char rsa_buf[2048] = {0};
	unsigned char R1[100] = {0};
	memcpy(R1, r1, r1len);

	creat_r2(r2);

	/*生成秘钥协商报文*/
	handle = dlopen(HLD_LIB, RTLD_NOW);
	if (handle == NULL)
	{
		fprintf(stderr, "Failed to open libaray %s error:%s\n", "libcryptopp.so", dlerror());
		return -1;
	}

	//获得加密接口
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

	//将R1进行R2为key的DES加密
	for(i = 0; i < (r1len / 8 + ((r1len % 8) != 0)); i++)
		des_en(r2, 8, (R1 + 8*i), (des_buf + 8 * i));
	l1 = i * 8;
	//将(R1)r2 + R2进行RSA加密
	memcpy(des_buf + l1, r2, 8);
	for(i = 0; i < (l1 + 8); i++)
		sprintf((str + i*2), "%02X", des_buf[i]);
//	printf("des_buf :  %s\n",des_buf);
	rsa_en(HLD_PUB, (char *)r1, (char *)str, (char *)rsa_buf, (int *)&l2);
//	printf("en: %s\n",rsa_buf);
	creat_afn18_fn02(&snbuf, rsa_buf, l1 + 8, 8, strlen((char *)rsa_buf));

	/*发送秘钥协商*/
	if(0 != DL3761_Protocol_LinkPack(&snbuf, &outbuf)) //将3761格式数据转换为可发送二进制数据
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
 * 函数功能:协商结果
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
 * 函数功能:身份认证
 * 参数:		s	socket
 * 			R2	随机数R2
 * 返回值: -1 失败  0成功
 * */
int hld_ac_id_discern(int s, unsigned char *r2)
{
	unsigned char R1[100] = {0};
	unsigned char l1 = 0;
	//请求主站证书
	printf("request_p1\n");
	if( 0 != request_p1(s))
	{
		printf("request_p1 erro\n");
		return -1;
	}

	//读取证书
	printf("read_p1\n");
	if(0 != read_p1(s, R1, &l1))
	{
		printf("read_p1 erro\n");
		return -1;
	}

	//秘钥协商
	printf("afn18_fn02\n");
	if(0 != afn18_fn02(s, R1, l1, r2))
	{
		printf("afn18_fn02 erro\n");
		return -1;
	}

	//等待协商结束
	printf("wait_result\n");
	if(0 != wait_result(s))
	{
		printf("wait_result erro\n");
		return -1;
	}

	return 0;
}

/*
 * 函数功能:创建afn18_fn03帧格式报文
 * 参数:		snbuf	帧格式报文
 * 			r2		随机数
 * 返回值: 0成  -1失败
 * */
int creat_afn18_fn03(tpFrame376_1 *outbuf, unsigned char *r2)
{
	int index = 0;
	memset(outbuf, 0, sizeof(tpFrame376_1));
	unsigned char a[256] = {0};
	unsigned char buf[256] = {0};
	int a_len = 0;
	int (*des_en)(unsigned char *, int , unsigned char *, unsigned char *);		//des加密
	void *handle = NULL;
	int i = 0;

	/************************************链路层************************************/
	outbuf->Frame376_1Link.BeginChar0 = 0x68;
	outbuf->Frame376_1Link.BeginChar1 = 0x68;
	//结束符
	outbuf->Frame376_1Link.EndChar = 0x16;
	//控制域
	outbuf->Frame376_1Link.CtlField.DIR = 1;	//上行
	outbuf->Frame376_1Link.CtlField.PRM = 1;	//启动站
	outbuf->Frame376_1Link.CtlField.FCV = 1;	//FCB位无效
	outbuf->Frame376_1Link.CtlField.FCB = 0;
	outbuf->Frame376_1Link.CtlField.FUNC_CODE = 0x10;
	//地址域
	outbuf->Frame376_1Link.AddrField.WardCode[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.WardCode[1] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[0] = 0xFF;
	outbuf->Frame376_1Link.AddrField.Addr[1] = 0xFF;

	outbuf->Frame376_1Link.AddrField.MSA = (DL3761MSA >> 1) & 0xFE;

	/***************************************应用层****************************************/
	//功能码
	outbuf->Frame376_1App.AFN = AFN3761_EXTEND18;
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
	//数据标识	0x3
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	outbuf->Frame376_1App.AppBuf[index++] = 0x04;
	outbuf->Frame376_1App.AppBuf[index++] = 0x00;
	//设备类型
	outbuf->Frame376_1App.AppBuf[index++] = 0x01;

	//加密后的特征码长度  及  特征码
	if(0 != get_ac_condition(a, &a_len))	//获取特征码
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
	outbuf->Frame376_1App.AppBuf[index++] = 8 * i;	//加密后 的特征码长度
	memcpy(outbuf->Frame376_1App.AppBuf + index, buf, 8 * i);	//加密后的内容
	index += (8 * i);

	outbuf->Frame376_1App.Len = index;
	outbuf->IsHaving = true;

	dlclose(handle);
	return 0;
}

/*
 * 函数功能:请求授权
 * 参数：	s	套接字
 * 			r2	随机数R2
 * 			len	R2长度
 * 返回值: 0成功 -1失败
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

	if(0 != DL3761_Protocol_LinkPack(&snbuf, &outbuf)) //将3761格式数据转换为可发送二进制数据
	{
		return -1;
	}

	int ret = write(s, outbuf.Data, outbuf.Len);
	if(ret != outbuf.Len)
		return -1;

	return 0;
}

/*
 * 函数功能:获取授权应答
 * 参数:		s	套接字
 * 			r2	随机数
 * 返回值:	0成功 -1超时 -2注册失败 -3 未注册 -4授权码格式错误 -5 其他
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

		//获取成功标志
		unsigned char flag = rvdbuf.Frame376_1App.AppBuf[index++];
		switch(flag)
		{
			case 1:
				break;
			case 2:
				return -2;	//注册失败
			case 3:
				return -3;	//未注册
			default:
				return -5;	//其他
		}

		unsigned char m_len = rvdbuf.Frame376_1App.AppBuf[index++];	//加密后的授权码长度
		if(m_len != 16)
		{
			return -4;	//授权码格式错误
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

		//des解密
		des_dn(r2, 8, buf, buf_buf);
		des_dn(r2, 8, buf+8, buf_buf+8);
		dlclose(handle);

		upset_md5(buf_buf, md5_buf);	//打乱加密后的特征码

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
 * 函数功能:授权认证
 * 参数:	  s		套接字
 * 		  r2	随机数
 * 返回值:0成功 -1超时 -2注册失败 -3 未注册 -4授权码格式错误 -5 其他
 * */
int hld_ac_impower(int s, unsigned char *r2)
{
	//请求授权
	if(0 != afn18_fn03(s, r2))
	{
		return -5;
	}

	//授权应答
	return read_afn18_fn03(s, r2);
}

/*
 * 授权管理线程
 * */
void *HldAcPthread(void *arg)
{
	sleep(30);		//休眠30S
	pthread_detach(pthread_self());	//线程分离

	char ipstr[20] = {0};
	unsigned short port = 0;

	if(0 != hld_ac_file_cs())	//授权码匹配失败
	{
		//重新申请授权		建立授权申请客户端
		int s = socket(PF_INET, SOCK_STREAM, 0);
		if(s < 0)
		{
			perror("hld ac socket:");
			_hld_ac.set_status(1);	//其他
			pthread_exit(NULL);
		}

		//获取ip及端口
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
		_hld_ac.set_status(1);	//链接失败
		if(0 > connect(s, (struct sockaddr *)&addr, addrlen)){
			perror("connect");
			pthread_exit(NULL);
		}

		//设置套接字非阻塞
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

		//身份认证
		_hld_ac.set_status(2);	//身份认证失败
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
		//授权认证
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
	else						//授权码匹配成功
	{
		_hld_ac.set_status(0);
	}

	pthread_exit(NULL);
}
