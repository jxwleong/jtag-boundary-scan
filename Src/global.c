/*
 * global.c
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#include "global.h"

extern JTAGInstruction currentIR;

/*
 * This function sets the current JTAG Instruction Register (IR) value.
 *
 * The 'currentIR' is a global variable which stores the current state of the JTAG Instruction Register.
 * This function updates the value of 'currentIR' with the 'instruction' argument passed to it.
 *
 * Parameters:
 *   - instruction: The instruction value to be set in the JTAG Instruction Register.
 *
 * The function does not return any value.
 */

void jtagSetIr(JTAGInstruction instruction){
	currentIR = instruction;
}

/*
 * This function causes a delay in the execution of the JTAG commands.
 *
 * It operates by running a decrementing loop, where each iteration signifies one delay cycle.
 *
 * Parameters:
 *   - cycles: The number of cycles to delay.
 *
 * The function does not return any value.
 */

void jtagDelay(uint32_t cycles){
	while(cycles--);
}

