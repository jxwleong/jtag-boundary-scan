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
 * This function is used to perform a single JTAG clock cycle, during which
 * the Test Data Input (TDI) and Test Mode Select (TMS) are set to the desired 
 * values. It also reads the value at the Test Data Output (TDO) during the 
 * same clock cycle.
 *
 * Parameters:
 *   - data: The value to be written to the TDI.
 *   - tms: The value to be written to the TMS.
 *
 * Returns:
 *   - val: The value read from the TDO at the end of the clock cycle.
 *
 * Process:
 *   - The clock (TCK) is initially set to a low state (0).
 *   - The TMS and TDI are then set to the input values provided (tms and data, respectively).
 *   - The system then pauses for 500 microseconds to ensure stable signal conditions.
 *   - The clock (TCK) is then transitioned to a high state (1), initiating a JTAG clock cycle.
 *   - The TDO value is read and stored.
 *   - The system pauses again for 500 microseconds to stabilize before the next operation.
 *   - The TDO value read is then returned as the output of the function.
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
