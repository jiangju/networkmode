/*
 * Infrared.h
 *
 *  Created on: 2016��6��24��
 *      Author: j
 */

#ifndef INFRARED_INFRARED_H_
#define INFRARED_INFRARED_H_

#define INFRARED_RV_DATA_LEN  2048		//�������ݽ��ջ��泤��
#define INFRARED_RD_LEN	500		//����1һ�ζ�ȡ���ݳ���

typedef struct
{
	unsigned short 	ReadIndex;							//��
	unsigned short 	WriteIndex;							//д
	unsigned char	DataBuffer[INFRARED_RV_DATA_LEN];	//����
}InfraredRvBuffer;										//������ջ���

void *Infrared(void *data);

#endif /* INFRARED_INFRARED_H_ */
