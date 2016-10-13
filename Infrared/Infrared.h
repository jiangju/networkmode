/*
 * Infrared.h
 *
 *  Created on: 2016年6月24日
 *      Author: j
 */

#ifndef INFRARED_INFRARED_H_
#define INFRARED_INFRARED_H_

#define INFRARED_RV_DATA_LEN  2048		//红外数据接收缓存长度
#define INFRARED_RD_LEN	500		//红外1一次读取数据长度

typedef struct
{
	unsigned short 	ReadIndex;							//读
	unsigned short 	WriteIndex;							//写
	unsigned char	DataBuffer[INFRARED_RV_DATA_LEN];	//缓存
}InfraredRvBuffer;										//红外接收缓存

void *Infrared(void *data);

#endif /* INFRARED_INFRARED_H_ */
