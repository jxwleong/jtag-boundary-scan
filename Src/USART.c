#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "USART.h"
#include "common.h"
#include "global.h"
#include "BoundaryScan.h"
#include "myString.h"

char tempBuffer[BUFFER_SIZE] = {0};
volatile BSCell bsc1;


void usartConfigure(USARTRegs *usart, long long mode, int desiredBaudrate, int peripheralFreq){
	int mantissa, fractional;
	float divider;
	int shiftFrac = 8;
	int over8 = 1;

	usart->CR1 = (uint32_t)(mode & 0xffff);
	usart->CR2 = (uint32_t)((mode >>16)& 0xffff);
	usart->CR3 = (uint32_t)(mode >> 32);


	if(usartOverSample16(usart)){
		shiftFrac = 16;
		over8 = 0;
	}

	divider = ((float)peripheralFreq/ (8 * ((2 - over8) * desiredBaudrate)));
	mantissa = (int) divider;
	fractional = (int)((float)(divider - mantissa) * shiftFrac);

	usart->BRR = (mantissa << 4)	| (fractional);

}


int usartOverSample16(USARTRegs *usart){
	// if 1 then its oversampling8
	if(usart->CR1 & USART_OVERSAMPLING_8){
		return FALSE;
	}
	else{
		return TRUE;
	}
}


void uartTransmitBuffer(USARTRegs *uart, char *buffer){
	int i = 0;
	while(buffer[i] != '\0'){
		if(usartIsTxRegEmpty(uart1)){
			(uart1)->DR = buffer[i];
			i++;
		}
	}
}

void commandLineOperation(char *commandStr){
	uint64_t ans = 0;
	long long tempLL = 0;
	int length = 0;
	char *ansStr;

	if(!(strcasecmp(commandStr, "jtag idcode"))){
		tempLL = jtagReadIdCode(READ_BOTH_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, DUMMY_DATA, 64);
		uint32_t bSCID = 0, cortexM3ID = 0;
		cortexM3ID = tempLL & 0xffffffff;
		bSCID = tempLL>>32;
		sprintf(tempBuffer, "Cortex M3 ID Code : 0x%X \nBoundary Scan Cell ID Code : 0x%X \n\n", cortexM3ID, bSCID);
	}

	else if(strstr(commandStr, "jtag sample") != '\0'){
		bypassCharactersInStr(&commandStr, 11);	// move the pointer after 'jtag sample'
		if(!(isBSRegValid(commandStr)))		// If the BS pin is invalid then print error message
			goto transmitBuffer;

		BSReg BSReg = getBSRegFromStr(&commandStr);
	  	bSCInIt(&bsc1);
		bSCPinConfigure(&bsc1, BSReg, INPUT_PIN);
		sprintf(tempBuffer, "Current value for pin %s is '%i'\n\n", BSReg.pinName, bSCSampleGpioPin(&bsc1, BSReg));
	}
	else if(strstr(commandStr, "jtag bypass") != '\0'){
		bypassCharactersInStr(&commandStr, 11);
		tempLL = getNumberFromStr(&commandStr);
		length = getDataLengthFromStr(&commandStr);
		ans = jtagBypass(BYPASS_BOTH_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, tempLL, length);
		ansStr = decimalToBinaryInStr(ans, length);
		sprintf(tempBuffer, "Result:  0b%s \n\n", ansStr);
	}

	else if(strstr(commandStr, "jtag extest") != '\0'){
		bypassCharactersInStr(&commandStr, 11);
		BSReg BSReg = getBSRegFromStr(&commandStr);
		tempLL = getNumberFromStr(&commandStr);	// get input from user
		bSCPinConfigure(&bsc1, BSReg, OUTPUT_PIN);
		bSCExtestGpioPin(&bsc1, BSReg, tempLL);
	}
	else if(strstr(commandStr, "jtag load IR") != '\0'){
		bypassCharactersInStr(&commandStr, 12);
		tempLL = getNumberFromStr(&commandStr);	// get input from user
		length = getDataLengthFromStr(&commandStr);
		loadJtagIR(tempLL, length, RUN_TEST_IDLE);

	}
	else if(strstr(commandStr, "jtag load DR") != '\0'){
		bypassCharactersInStr(&commandStr, 12);
		tempLL = getNumberFromStr(&commandStr);	// get input from user
		length = getDataLengthFromStr(&commandStr);
		ans = jtagWriteAndRead(tempLL, length);
		ansStr = decimalToBinaryInStr(ans, length);
		sprintf(tempBuffer, "Result:  0b%s \n\n", ansStr);
	}
	else if(!(strcasecmp(commandStr, "help"))){
		sprintf(tempBuffer, "\nIDCODE \
							 \n------\
				             \n1. Type 'jtag idcode' (case insenstive)	\
				             \n2. Id code for both TAP devices will be shown.\
				             \n	\
				             \nBYPASS \
				             \n------ \
				             \n1. Type 'jtag bypass [data] [length]' (data accepted in hex, binary and decimal format)\
							 \n2. The result of bypass will be shown after enter was pressed \
							 \n	\
				             \nSAMPLE \
				             \n------ \
				             \n1. Type 'jtag sample [Boundary Scan Register/ GPIO pin]'	\
				             \n2. The JTAG probe will read the current pin status and show the result through UART \
				             \n WARNING : Make sure the pin is connected to a valid source else the reading may be invalid.\
							 \n \
							 \nPRELOAD\
							 \n--------\
							 \n1. This instruction will be used in EXTEST mode	\
							 \n	\
							 \nEXTEST	\
							 \n------	\
				             \n1. Type 'jtag extest [Boundary Scan Register/ GPIO pin] [pin status (1/0)]'	\
				             \n2. Connect the GPIO pin to an simple LED circuit then observe the output\n\n");
	}
	else{
		sprintf(tempBuffer, "\nUnknown command received. Please type 'help' for assitance. \
				\n\n");
	}
transmitBuffer:
	uartTransmitBuffer(uart1, tempBuffer);
	// clear the buffer so that same instruction will not run
	// when user press 'ENTER'
	tempBuffer[0] = '\0';
}

