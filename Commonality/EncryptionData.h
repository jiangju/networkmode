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
    unsigned int count[2];//ģ64��bit��
    unsigned int state[4];//������ϢժҪ
    unsigned char buffer[64];////���뻺����
}MD5_CTX;

/*�����ĸ������İ�λ�����ĺ���*/
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))

#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))//������λ��Ϊ�˷�ֹ�ظ�����
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

void MD5Init(MD5_CTX *context);//��ʼ��MD5�������ĸ�state����������ϢժҪ

/*MD5��ʼ�����ݵ�ʱ���������ݰ������䵽512��������-64�������64λ���������ݵĳ��Ȳ��䣨������64λ��ʾ��
Ȼ�����ݷ�Ϊ512λ�Ķ�����ݿ���м��ܣ�ÿ�μ��ܶ���Ҫ��һ�����ݿ��ɢ��ֵ�ͱ������ݿ�����ݣ���μ��ܵõ�
�̶����ȵ�ɢ��ֵ��MD5ֵ������������������������Ϣ���ժҪ�����Ҹ���context*/
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);

void MD5Final(MD5_CTX *context,unsigned char digest[16]);//���롢����ĳ��ȡ��������ֱ�Ӵ���

/*�����ֱ任�任�Կ�Ϊ������λ������Ϣ��512����Ŀ�*/
void MD5Transform(unsigned int state[4],unsigned char block[64]);

void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);


void main_StrToHex(unsigned char inStr[],unsigned char outHex[],unsigned char StrLen);

int MD5EncryptionInit(unsigned char *InData,unsigned char *OutData, unsigned short InDataLen);

int	MD5Verify(unsigned char *inbuf, int len, unsigned char src);

#endif

#ifndef ENCRYPTIONDATA_C_

/*
 * �ⲿ�ӿڴ������
 *InData ��Ҫ����16�����ַ���
 *OutData �����ַ���MAC
 */
extern int MD5EncryptionInit(unsigned char *InData,unsigned char *OutData, unsigned short InDataLen);
extern int	MD5Verify(unsigned char *inbuf, int len, unsigned char src);
#endif


#endif /* ENCRYPTIONDATA_H_ */
