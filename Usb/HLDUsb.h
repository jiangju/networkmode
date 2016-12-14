/*
 * HLDUsb.h
 *
 *  Created on: 2016��9��13��
 *      Author: j
 */

#ifndef USB_HLDUSB_H_
#define USB_HLDUSB_H_

#include <pthread.h>
/*********************************************************************************/
struct usb_status
{
	char s_in;				//����״̬   0δ����  1����
	char s_log;				//������־״̬ 0��������1���ڿ�����2�����ɹ�����������4λ����4���ļ�����Ӧ
							//λ=1����ļ���������
	char s_update;			//����״̬
							//0��������
							//1����������
							//2�����ɹ���
							//3��ȡ�����ļ�����CRCʧ��
							//4������������ʧ��
							//5���㿽������ļ�CRCʧ��
							//6CRC��ƥ��
};

struct hld_usb_s
{
	struct usb_status status;
	pthread_mutex_t mutex;
};

/************************************************************************************/
void init_usb_status();
void set_usb_status(struct usb_status in);
void get_usb_status(struct usb_status *out);

void *pthread_usb(void *arg);

#endif /* USB_HLDUSB_H_ */
