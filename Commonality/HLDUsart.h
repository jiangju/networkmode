/*
 * Usart.h
 *
 *  Created on: 2016��6��24��
 *      Author: j
 */

#ifndef COMMONALITY_HLDUSART_H_
#define COMMONALITY_HLDUSART_H_

#define SEND_LEN	50

typedef struct
{
	unsigned int 	BaudRate;	//������
	unsigned char 	isStop;		//�Ƿ�ֹͣλ 2 1
	unsigned char	DataLen;	//����λ����
	unsigned char	isParity;	//��żУ�� 0 1 2
	unsigned char	FlowCtl;	//������ 0 1 2
}UsartAttribute;	//��������

char SetUsart(unsigned char num, UsartAttribute *attribute);
void UsartSend(int fd, unsigned char *outbuff, unsigned short outlen);
#endif /* COMMONALITY_HLDUSART_H_ */
