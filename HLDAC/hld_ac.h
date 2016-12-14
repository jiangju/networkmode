/*
 * hld_ac.h
 *
 *  Created on: 2016年11月18日
 *      Author: j
 */

#ifndef HLDAC_HLD_AC_H_
#define HLDAC_HLD_AC_H_
#include "SysPara.h"
#define HLD_AC_FILE		"/opt/hld_ac_file"		//华立达认证文件
#define HLD_LIB			"/opt/libcryptopp.so"	//加密算法库
#define HLD_PUB			"/opt/hld_pub"			//证书文件

typedef struct
{
	unsigned char ip[4];	//主站ip
	unsigned short pr;		//端口号
}HldAcPara;						//华立达认证模块参数

typedef struct
{
	char	status;			//0 成功
							//-1链接失败
							//-2身份认证失败
							//-3注册失败
							//-4未注册
							//-5超时
							//-6加密后的特征码格式错误
							//其他
	pthread_mutex_t mutex;	//锁
}HldAcStruct;

typedef struct
{
	void (*init_status)(void);
	char (*get_status)(void);
	void (*set_status)(char);

	void*(*thread)(void*);
}HldAcMode;

#ifndef _HLD_AC_C_
HldAcMode _hld_ac;
#endif

#ifdef _HLD_AC_C_
extern HldAcMode _hld_ac;
#endif

int open_hld_ac(void);
void init_hld_ac_mode(void);
void init_hld_ac_mode(void);
void init_hld_ac_mode_para(void);
void init_hld_ac_mode_status(void);
char get_hld_ac_mode_status(void);
void set_hld_ac_status(char in);
void *HldAcPthread(void *arg);

#endif /* HLDAC_HLD_AC_H_ */
