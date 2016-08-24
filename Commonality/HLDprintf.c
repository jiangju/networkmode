/*
 * HLDprintf.c
 *
 *  Created on: 2016��8��22��
 *      Author: j
 */
#include <termios.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "HLDprintf.h"

static int hld_printf;
static int hld_printf_flag;	//��ӡ�ļ���ʧ��
/*
 * ��������:�������ӡ��ʼ��
 * */
void hld_printf_init(void)
{
#if HLD_PRINTF_KEY
	hld_printf = open(HLD_PRINTF, O_RDWR);
	if(hld_printf < 0)
	{
		perror("open hld printf file:");
		hld_printf_flag = 0xff;
		return;
	}
#endif
}

/*
 * ��������:HLD��ӡ
 * */
void hld_printf_out(char *str, int len)
{
#if HLD_PRINTF_KEY
	if(0xFF != hld_printf_flag)
	{
		lseek(hld_printf, 0, SEEK_SET);
		write(hld_printf, str, len);
	}
#endif
}
