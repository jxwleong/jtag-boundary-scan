/*
 * BoundaryScan.c

 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#include "BoundaryScan.h"
#include "TAP.h"
#include "global.h"
#include "BSReg_Table.h"
#include "myString.h"

extern JTAGInstruction currentIR;
extern TapState currentTapState;

// reset the array to 0
void bSCInIt(volatile BSCell *bSC){
	  int i = 0;

	  while(i < BOUNDARY_SCAN_CELL_DIV_8){
		  bSC->bSCellPreloadData[i] = 0x00000000;
		  bSC->bSCellSampleData[i] = 0x00000000;
		  i++;
	  }
}

// Read and Write the Boundary Scan Cell at the same time
void jtagWriteAndReadBSCells(volatile BSCell *bSC, int length){
	int dataMask = 1;
	int oneBitData = 0;
	volatile uint64_t tdoData = 0;
	uint64_t i = 0;
	int n = 0;
	int count = 0;

	// get rid of bypass bit for CORTEX M3 TAP
	tdoData = jtagClkIoTms(oneBitData, 0);
	// noted that last bit of data must be set at next tap state
	for (n = length ; n > 1; n--) {
	  oneBitData = dataMask & bSC->bSCellPreloadData[count/8];
	  tdoData = jtagClkIoTms(oneBitData, 0);
	  currentTapState = updateTapState(currentTapState, 0);
	  // DIV by 8 to get the index for uint8_t array
	  (bSC->bSCellSampleData[count/8]) |= tdoData << (i*1);;
	  (bSC->bSCellPreloadData[count/8]) = bSC->bSCellPreloadData[count/8] >> 1;
	  i++;
		if(i%8 == 0)
			i = 0;
	  count++;
	}
	oneBitData = dataMask & bSC->bSCellPreloadData[count/8];
	tdoData = jtagClkIoTms(oneBitData, 1);
	currentTapState = updateTapState(currentTapState, 1);
	(bSC->bSCellSampleData[count/8]) |= tdoData << (i*1);
}


int jtagReadBSRegInput(volatile BSCell *bSC, BSReg bSReg){
	int bSRegInput = 0;
	int mask = 1;

	int arrayIndex = (bSReg.inputCellNum)/ 8;
	bSRegInput = (bSC->bSCellSampleData[arrayIndex]) >> (bSReg.inputCellNum %8);
	bSRegInput &= mask;
	return bSRegInput;
}

int jtagReadBSCPin(volatile BSCell *bSC, int pin, BSCDataType bSCDataType){
	int bSRegInput = 0;
	int mask = 1;

	int arrayIndex = pin/ 8;
	if(bSCDataType == SAMPLE_DATA)
		bSRegInput = ((bSC->bSCellSampleData[arrayIndex])) >> (pin%8);
	else if(bSCDataType == PRELOAD_DATA)
		bSRegInput = ((bSC->bSCellPreloadData[arrayIndex])) >> (pin%8);

	bSRegInput &= mask;
	return bSRegInput;
}

void sampleBSC(volatile BSCell *bSC){
	tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
	jtagWriteAndReadBSCells(bSC, CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH);
	tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
}

int bSCSampleGpioPin(volatile BSCell *bSC, BSReg bSReg){
	int val = 0;

	loadJtagIR(SAMPLE_PRELOAD_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
	sampleBSC(bSC);
	val = jtagReadBSRegInput(bSC, bSReg);
	return val;

}

void bSCPinConfigure(volatile BSCell *bSC, BSReg bSReg, BSCPinMode pinMode){
	int arrayIndex = (bSReg.controlCellNum)/ 8;
	bSC->bSCellPreloadData[arrayIndex]  =  bSC->bSCellPreloadData[arrayIndex] | (pinMode << (bSReg.controlCellNum % 8));
}

void writePreloadData(volatile BSCell *bSC, BSReg bSReg, int data){
	int arrayIndex = (bSReg.outputCellNum)/ 8;
	bSC->bSCellPreloadData[arrayIndex]  =  bSC->bSCellPreloadData[arrayIndex] | (data << (bSReg.outputCellNum % 8));
}



void bSCPreloadData(volatile BSCell *bSC, BSReg bSReg, int data){
	  writePreloadData(bSC, bSReg, data);
	  if(currentIR != EXTEST){
	  loadJtagIR(SAMPLE_PRELOAD_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
	  }
	  tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
	  jtagWriteAndReadBSCells(bSC, CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH);
	  tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
}

void bSCExtestGpioPin(volatile BSCell *bSC, BSReg bSReg, int data){
	  bSCPreloadData(bSC, bSReg, data);
	  if(currentIR != EXTEST){
		  loadJtagIR(EXTEST_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
		  jtagSetIr(EXTEST);
	  }
}

BSReg getBSRegFromStr(char *str){
    char BSRegInStr[BSCELL_STR_LENGTH] = {0};
    int i = 0;
    // skip the blank spaces in string
    skipWhiteSpaces(&str);

    while(*str != ' ' && *str != '\0'){
        BSRegInStr[i] = *str;
        str++;
        i++;
    }

    return (getBSRegFromTable(BSRegInStr));

}

