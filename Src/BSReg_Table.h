/*
 * BSReg_Table.h
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#include "BoundaryScan.h"

#ifndef BSREG_TABLE_H_
#define BSREG_TABLE_H_

typedef enum{
	PA0, PA1, PA2, PA3,
	PA4, PA5, PA6, PA7,
	PA8, PA9, PA10, PA11,
	PA12,

	PB0, PB1, PB2, PB3,
	PB4, PB5, PB6, PB7,
	PB8, PB9, PB10, PB11,
	PB12, PB13, PB14, PB15,

	PC13, PC14, PC15,

	oscOutPD0,
	oscOutPD1,
}BSRegPin;




BSReg getBSRegFromTable(char *str);
int isBSRegValid(char *str);
#endif /* BSREG_TABLE_H_ */
