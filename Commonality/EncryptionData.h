/******************
 * EncryptionData.h
 *
 *  Created on: 2016-4-18
 *      Author: root
******************/

#ifndef ENCRYPTIONDATA_H_
#define ENCRYPTIONDATA_H_

typedef struct
{
    unsigned int count[2];//模64的bit数
    unsigned int state[4];//计算信息摘要
    unsigned char buffer[64];////输入缓冲区
}MD5_CTX;

/*定义四个基本的按位操作的函数*/
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))

#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))//函数移位是为了防止重复计算
#define FF(a,b,c,d,x,s,ac) \
          { \
          a += F(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define GG(a,b,c,d,x,s,ac) \
          { \
          a += G(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define HH(a,b,c,d,x,s,ac) \
          { \
          a += H(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }
#define II(a,b,c,d,x,s,ac) \
          { \
          a += I(b,c,d) + x + ac; \
          a = ROTATE_LEFT(a,s); \
          a += b; \
          }

#ifdef ENCRYPTIONDATA_C_

void MD5Init(MD5_CTX *context);//初始化MD5参数，四个state用来计算信息摘要

/*MD5初始化数据的时候将输入数据按字扩充到512的整数倍-64，后面的64位用输入数据的长度补充（长度用64位表示）
然后将数据分为512位的多个数据块进行加密，每次加密都需要上一个数据块的散列值和本个数据块的数据，多次加密得到
固定长度的散列值即MD5值，本函数用来连续处理多个消息块的摘要。并且更新context*/
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);

void MD5Final(MD5_CTX *context,unsigned char digest[16]);//输入、输入的长度、输出数组直接传入

/*进行轮变换变换以块为基础单位，即消息按512分组的块*/
void MD5Transform(unsigned int state[4],unsigned char block[64]);

void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);


void main_StrToHex(unsigned char inStr[],unsigned char outHex[],unsigned char StrLen);

int MD5EncryptionInit(unsigned char *InData,unsigned char *OutData, unsigned short InDataLen);

int	MD5Verify(unsigned char *inbuf, int len, unsigned char src);

#endif

#ifndef ENCRYPTIONDATA_C_

/*
 * 外部接口传入参数
 *InData 需要加密16进制字符串
 *OutData 加密字符串MAC
 */
extern int MD5EncryptionInit(unsigned char *InData,unsigned char *OutData, unsigned short InDataLen);
extern int	MD5Verify(unsigned char *inbuf, int len, unsigned char src);
#endif


#endif /* ENCRYPTIONDATA_H_ */
