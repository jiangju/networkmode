/*
 * hld_ac.h
 *
 *  Created on: 2016��11��18��
 *      Author: j
 */

#ifndef HLDAC_HLD_AC_H_
#define HLDAC_HLD_AC_H_
#include "SysPara.h"
#define HLD_AC_FILE		"/opt/hld_ac_file"		//��������֤�ļ�
#define HLD_LIB			"/opt/libcryptopp.so"	//�����㷨��
#define HLD_PUB			"/opt/hld_pub"			//֤���ļ�

typedef struct
{
	unsigned char ip[4];	//��վip
	unsigned short pr;		//�˿ں�
}HldAcPara;						//��������֤ģ�����

typedef struct
{
	char	status;			//0 �ɹ�
							//-1����ʧ��
							//-2�����֤ʧ��
							//-3ע��ʧ��
							//-4δע��
							//-5��ʱ
							//-6���ܺ���������ʽ����
							//����
	pthread_mutex_t mutex;	//��
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
