/*
 * Usart.h
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */

#ifndef COMMONALITY_HLDUSART_H_
#define COMMONALITY_HLDUSART_H_

#define SEND_LEN	50

typedef struct
{
	unsigned int 	BaudRate;	//波特率
	unsigned char 	isStop;		//是否停止位 2 1
	unsigned char	DataLen;	//数据位长度
	unsigned char	isParity;	//奇偶校验 0 1 2
	unsigned char	FlowCtl;	//控制流 0 1 2
}UsartAttribute;	//串口属性

char SetUsart(unsigned char num, UsartAttribute *attribute);
void UsartSend(int fd, unsigned char *outbuff, unsigned short outlen);
#endif /* COMMONALITY_HLDUSART_H_ */
