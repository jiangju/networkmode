/*
 * HLDprintf.h
 *
 *  Created on: 2016��8��22��
 *      Author: j
 */

#ifndef COMMONALITY_HLDPRINTF_H_
#define COMMONALITY_HLDPRINTF_H_

#define	HLD_PRINTF	"/opt/hldprintf"	//�������ӡ�ļ�
#define HLD_PRINTF_KEY	0		//�������ӡ����

void hld_printf_init(void);
void hld_printf_out(char *str, int len);

#endif /* COMMONALITY_HLDPRINTF_H_ */
