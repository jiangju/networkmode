/*
 * HLDUsb.h
 *
 *  Created on: 2016年9月13日
 *      Author: j
 */

#ifndef USB_HLDUSB_H_
#define USB_HLDUSB_H_

#include <pthread.h>
/*********************************************************************************/
struct usb_status
{
	char s_in;				//插入状态   0未插入  1插入
	char s_log;				//拷贝日志状态 0不拷贝，1正在拷贝，2拷贝成功，其他按高4位代表4个文件，相应
							//位=1则该文件拷贝出错
	char s_update;			//升级状态
							//0不升级，
							//1正在升级，
							//2升级成功，
							//3获取升级文件名及CRC失败
							//4拷贝升级程序失败
							//5计算拷贝后的文件CRC失败
							//6CRC不匹配
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
