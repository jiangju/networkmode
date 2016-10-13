/*
 * DL3761_AFN10.h
 *
 *  Created on: 2016Äê7ÔÂ25ÈÕ
 *      Author: j
 */

#ifndef ST376_1_DL3761_AFN10_H_
#define ST376_1_DL3761_AFN10_H_

#include "DL376_1_DataType.h"
void Create3761AFN10_01(unsigned char *ter, unsigned char *inbuf, int len,  unsigned char flag, tpFrame376_1 *outbuf);
void DL3761_AFN10_Analy(tpFrame376_1 *rvframe3761);
#endif /* ST376_1_DL3761_AFN10_H_ */
