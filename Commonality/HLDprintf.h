/*
 * HLDprintf.h
 *
 *  Created on: 2016年8月22日
 *      Author: j
 */

#ifndef COMMONALITY_HLDPRINTF_H_
#define COMMONALITY_HLDPRINTF_H_

#define	HLD_PRINTF	"/opt/hldprintf"	//华立达打印文件
#define HLD_PRINTF_KEY	0		//华立达打印开关

void hld_printf_init(void);
void hld_printf_out(char *str, int len);

#endif /* COMMONALITY_HLDPRINTF_H_ */
