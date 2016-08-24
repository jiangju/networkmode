/*
 * HLDprintf.c
 *
 *  Created on: 2016年8月22日
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
static int hld_printf_flag;	//打印文件打开失败
/*
 * 函数功能:华立达打印初始化
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
 * 函数功能:HLD打印
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
