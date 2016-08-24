/*
 * DL3762_AFN2.h
 *
 *  Created on: 2016Äê8ÔÂ8ÈÕ
 *      Author: j
 */

#ifndef ST376_2_DL3762_AFN02_H_
#define ST376_2_DL3762_AFN02_H_

#include "DL376_2_DataType.h"
#include "SysPara.h"

void DL3762_AFN02_Analy(tpFrame376_2 *rvframe3762, tpFrame376_2 *snframe3762);
void Create3762AFN02_01(unsigned char *amm, unsigned char type, unsigned char *inbuf, unsigned char len, tpFrame376_2 *snframe3762);

#endif /* ST376_2_DL3762_AFN02_H_ */
