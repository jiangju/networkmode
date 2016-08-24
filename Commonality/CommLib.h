/*
 * CommLib.h
 *
 *  Created on: 2016Äê6ÔÂ24ÈÕ
 *      Author: j
 */

#ifndef COMMONALITY_COMMLIB_H_
#define COMMONALITY_COMMLIB_H_

char DTtoFN(unsigned char *DT);
unsigned char Get_CS(unsigned char *buf, unsigned short index, unsigned short len);
int WriteFile(int fd, int offset, void *buff, int len);
int ReadFile(int fd, int offset, void *buff, int len);
int IfConfigEthnert(const char *ifname, const char *ipaddr,const char *netmask, const char *gwip);
unsigned char Func_CS(void *inBuf,unsigned short inBufLen);
void FNtoDT(unsigned char fn, unsigned char *dt);
int CompareUcharArray(unsigned char *A, unsigned char *B, int len);
#endif /* COMMONALITY_COMMLIB_H_ */
