/*
 * jtagLowLevel.c
 *
 *  Created on: Aug 17, 2019
 *      Author: User
 */

#include "jtagLowLevel.h"
#include "main.h"
#include "stm32f1xx_hal.h"

// include in this file bcoz header need to use mock for unit test
// else when run ceedling, it will keep asking for stm32xxx files
// HAL GPIO MACROs
#define setClock(clk)	HAL_GPIO_WritePin(TCK_GPIO_Port, TCK_Pin, clk);
#define setTms(tms)		HAL_GPIO_WritePin(TMS_GPIO_Port, TMS_Pin, tms);
#define writeTdi(tdi)	HAL_GPIO_WritePin(TDI_GPIO_Port, TDI_Pin, tdi);
#define readTdo()		HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin);

/*
 * Initialize all JTAG's input pin to zero
 */
void jtagIoInit(){
	setClock(0);
	setTms(0);
	writeTdi(0);
}

/*
 * Handle the TCK, TMS, TDI(data), and
 * return value read at same CLK cycle.
 */
int jtagClkIoTms(int data, int tms){
	int val = 0;

	setClock(0);
	setTms(tms);
	writeTdi(data);
	jtagDelay(500);
	setClock(1);
	val = readTdo();
	jtagDelay(500);
	return val;
}
