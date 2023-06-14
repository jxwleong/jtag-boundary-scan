#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "USART.h"
#include "common.h"
#include "global.h"
#include "BoundaryScan.h"
#include "myString.h"

char tempBuffer[BUFFER_SIZE] = {0};
volatile BSCell bsc1;


/* This function configures the USART peripheral for a desired baud rate and mode.
 *
 * Parameters:
 *   - *usart: A pointer to the USART register structure. This will be used to modify the USART's settings.
 *   - mode: A 64-bit value where each bit represents a different setting or feature for the USART.
 *   - desiredBaudrate: The desired communication speed in baud (symbols per second).
 *   - peripheralFreq: The frequency at which the USART's clock is running.
 */
void usartConfigure(USARTRegs *usart, long long mode, int desiredBaudrate, int peripheralFreq){
	int mantissa, fractional;
	float divider;
	int shiftFrac = 8;
	int over8 = 1;

	/* Mode configuration: split the 64-bit mode variable into 4 16-bit parts and assign them 
     * to the USART's control registers (CR1, CR2, CR3). These control registers can enable/disable
     * various USART features and settings.
     */
	usart->CR1 = (uint32_t)(mode & 0xffff);
	usart->CR2 = (uint32_t)((mode >>16)& 0xffff);
	usart->CR3 = (uint32_t)(mode >> 32);

   /* Set the BRR with the calculated mantissa and fractional values. The BRR determines the
     * USART's baud rate.
     */
	if(usartOverSample16(usart)){
		shiftFrac = 16;
		over8 = 0;
	}

	/* Calculate the mantissa and fractional parts of the USART's baud rate register (BRR). 
     * These are calculated based on the peripheral frequency, the desired baud rate, and whether
     * we are oversampling by 8 or 16.
	 * 
	 * In the context of a USART (Universal Synchronous/Asynchronous Receiver/Transmitter), 
	 * oversampling by 8 or 16 refers to the number of times the USART samples the state of a received bit to determine whether it is a '0' or a '1'.

		For instance, in "oversample by 16" mode, the USART module will sample the state of a received bit 16 times. Then, it typically uses a majority voting system to determine the value of the bit. This can help improve the reliability of data reception, especially in noisy environments.
     */

	divider = ((float)peripheralFreq/ (8 * ((2 - over8) * desiredBaudrate)));
	mantissa = (int) divider;
	fractional = (int)((float)(divider - mantissa) * shiftFrac);

	/* Set the BRR with the calculated mantissa and fractional values. The BRR determines the
     * USART's baud rate.
     */
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
				             \n2. Connect the GPIO pin to an simple LED circuit then observe the output  \
				             \n	\
				             \nLoading instruction register or data register		\
				             \n---------------------------------------------	\
				             \n1. jtag load [IR/DR] [data] [length]	\
							 \n2. The result will be shown after loading DR\n\n");
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

