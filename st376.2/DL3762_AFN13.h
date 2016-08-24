/*
 * DL3762_AFN13.h
 *
 *  Created on: 2016Äê7ÔÂ26ÈÕ
 *      Author: j
 */

#ifndef ST376_2_DL3762_AFN13_H_
#define ST376_2_DL3762_AFN13_H_

#include "DL376_2_DataType.h"
#include "SysPara.h"

void DL3762_AFN13_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762);
void Create3762AFN13_01(unsigned char *amm, unsigned char type, unsigned char *inbuf, unsigned char len, tpFrame376_2 *snframe3762);
#endif /* ST376_2_DL3762_AFN13_H_ */
