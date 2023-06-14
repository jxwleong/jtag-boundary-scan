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
#include "jtagLowLevel.h"

extern JTAGInstruction currentIR;
extern TapState currentTapState;

/*
 * This function is used to initialize a BSCell structure by setting all the
 * elements of the bSCellPreloadData and bSCellSampleData arrays to zero. 
 *
 * The BSCell structure represents the state of a boundary-scan cell (BSC), 
 * a circuit element used in testing via the JTAG interface. Each BSC 
 * contains two key data arrays:
 *  - bSCellPreloadData: used for preloading data into the BSC.
 *  - bSCellSampleData: used for sampling the data from the BSC.
 *
 * The function loops over the bSCellPreloadData and bSCellSampleData arrays 
 * based on the length defined by the BOUNDARY_SCAN_CELL_DIV_8 constant. 
 *
 * This constant represents the number of bytes in these arrays, each byte 
 * corresponding to 8 cells in the Boundary Scan Chain. Therefore, the division 
 * by 8: to convert from a total number of bits (cells) to a total number of bytes.
 * This indicates that the data might be handled byte-wise, which is a common 
 * practice in digital systems.
 * 
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that needs to be initialized.
 */
void bSCInIt(volatile BSCell *bSC){
	  int i = 0;

	  // Loop until all array elements are initialized to 0.
	  while(i < BOUNDARY_SCAN_CELL_DIV_8){
		  // Set the i-th element of the bSCellPreloadData array to 0.
		  bSC->bSCellPreloadData[i] = 0x00000000;
		  // Set the i-th element of the bSCellSampleData array to 0.
		  bSC->bSCellSampleData[i] = 0x00000000;
		  // Increment the index.
		  i++;
	  }
}

/*
 * This function, jtagWriteAndReadBSCells, performs simultaneous reading and writing operations
 * on a Boundary Scan Cell (BSC) via the JTAG interface. It is designed for effective 
 * hardware testing and debugging by providing access to the internal state of the BSC.
 *
 * Parameters:
 *   - *bSC: A pointer to a volatile BSCell structure. This structure holds the 
 *           data intended for writing to the BSC (inside the bSCellPreloadData array) and 
 *           is also designed to receive the data read from the BSC (stored in 
 *           the bSCellSampleData array).
 *   - length: This signifies the number of bits intended for reading from and writing to the BSC.
 *
 * Initially, the function bypasses the first bit specifically for the CORTEX M3 TAP (Test Access Port) controller.
 * Then it initiates a loop that traverses through each bit in the BSC, excluding the last bit.
 * For each bit, the function executes these steps:
 *   - Extracts the bit to be written from the bSCellPreloadData array.
 *   - Writes this isolated bit to the JTAG interface while simultaneously reads a bit 
 *     from the JTAG interface, saving the read data.
 *   - Updates the state of the JTAG TAP (Test Access Port) controller.
 *   - Records the bit read from the JTAG interface in the equivalent location 
 *     in the bSCellSampleData array.
 *   - Executes a right-shift operation on the bits in the bSCellPreloadData array in preparation for the next cycle.
 *
 * Post the loop, the function handles the last bit in a similar fashion, but with the 
 * TAP controller state being updated to Exit1-DR (Data Register) or Update-DR state.
 *
 * The function uses a counter 'i' to handle data in a byte-wise manner, which is a common practice in digital systems.
 * The counter keeps track of the current bit location and resets for every 8 bits, which corresponds to a byte.
 */
void jtagWriteAndReadBSCells(volatile BSCell *bSC, int length){
	int dataMask = 1;
	int oneBitData = 0;
	volatile uint64_t tdoData = 0;
	uint64_t i = 0;
	int n = 0;
	int count = 0;

	// Get rid of bypass bit for CORTEX M3 TAP. This bit is 
    // the first bit to be shifted out during JTAG operations.
	tdoData = jtagClkIoTms(oneBitData, 0);


	// Note that the last bit of data must be set at next tap state.
	// The loop will shift in all but the last bit of the boundary 
    // scan data. 
	for (n = length ; n > 1; n--) {
		// Extract the bit to be shifted in	
	  oneBitData = dataMask & bSC->bSCellPreloadData[count/8];

	   // Extract the bit to be shifted in
	  tdoData = jtagClkIoTms(oneBitData, 0);

	  // Store the output data into the sampled data array. 
	  // DIV by 8 is used to get the index for uint8_t array
	  currentTapState = updateTapState(currentTapState, 0);
	  // Update the current TAP state
	  (bSC->bSCellSampleData[count/8]) |= tdoData << (i*1);;

	   // Shift the remaining data to the right
	  (bSC->bSCellPreloadData[count/8]) = bSC->bSCellPreloadData[count/8] >> 1;
	  i++;
		if(i%8 == 0)
			i = 0;
	  count++;
	}

	// For the last bit of data, transition to EXIT1_DR state (TMS=1)
	oneBitData = dataMask & bSC->bSCellPreloadData[count/8];
	tdoData = jtagClkIoTms(oneBitData, 1);
	currentTapState = updateTapState(currentTapState, 1);

	// Store the final output bit
	(bSC->bSCellSampleData[count/8]) |= tdoData << (i*1);
}

/*
 * The jtagReadBSRegInput function is used to read data from a specific Boundary Scan (BS) Register input cell.
 * The BS Register is a collection of BS cells used in a JTAG system for testing and debugging purposes.
 *
 * Parameters:
 *   - *bSC: A pointer to a volatile BSCell structure. This structure contains an array bSCellSampleData that 
 *           holds the data read from the BSC (Boundary Scan Cell).
 *   - bSReg: The BS Register from which the data needs to be read. This structure contains details about 
 *            the input cell number (inputCellNum) that specifies the location from which data needs to be read.
 *
 * The function works as follows:
 *   - Calculates the array index by dividing the inputCellNum by 8. This is because each element of the array 
 *     represents 8 bits (or 1 byte), so this division gives the index of the byte that contains the required bit.
 *   - Performs a right-shift operation on the data located at the calculated index in the bSCellSampleData array. 
 *     The number of shifts is equal to the remainder of inputCellNum divided by 8, which gives the position of the 
 *     required bit within the selected byte.
 *   - Finally, the function applies a bitwise AND operation with a mask to isolate the required bit.
 *
 * The function returns this isolated bit as the read data from the specified BS Register input cell.
 *
 * Returns:
 *   - bSRegInput: The bit read from the specified input cell of the BS Register.
 */
int jtagReadBSRegInput(volatile BSCell *bSC, BSReg bSReg){
	int bSRegInput = 0;
	int mask = 1;

	int arrayIndex = (bSReg.inputCellNum)/ 8;
	bSRegInput = (bSC->bSCellSampleData[arrayIndex]) >> (bSReg.inputCellNum %8);
	bSRegInput &= mask;
	return bSRegInput;
}

/*
 * The jtagReadBSCPin function is used to read data from a specific pin in a Boundary Scan Cell (BSC) 
 * using JTAG (Joint Test Action Group) methodology for testing. The BSC structure contains two arrays: 
 * bSCellSampleData and bSCellPreloadData, that hold the data read from and to be written into the BSC respectively.
 *
 * Parameters:
 *   - *bSC: A pointer to a volatile BSCell structure.
 *   - pin: The specific pin in the BSC from which the data needs to be read.
 *   - bSCDataType: A enum value representing the type of data to be read. 
 *                  SAMPLE_DATA refers to data read from BSC, PRELOAD_DATA refers to data that was previously written.
 *
 * The function operates in the following manner:
 *   - First, it calculates the array index by dividing the pin number by 8. This is because each array element 
 *     represents 8 pins (or 1 byte), so this division gives the index of the byte that contains the required pin.
 *   - Then, it selects the appropriate array (bSCellSampleData for SAMPLE_DATA and bSCellPreloadData for PRELOAD_DATA) 
 *     and performs a right-shift operation on the data located at the calculated index in the selected array. 
 *     The number of shifts is equal to the remainder of pin number divided by 8, which gives the position of the 
 *     required pin within the selected byte.
 *   - Finally, the function applies a bitwise AND operation with a mask to isolate the required bit.
 *
 * The function then returns this isolated bit as the read data from the specified pin in the BSC.
 *
 * Returns:
 *   - bSRegInput: The bit read from the specified pin of the BSC.
 */
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

/*
 * This function triggers the process of sampling data from a Boundary Scan Cell (BSC).
 * It navigates the TAP (Test Access Port) state machine from the Run-Test/Idle state to 
 * the Shift-DR state, performs the read/write operations on the BSC and then returns to 
 * the Run-Test/Idle state.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 */
void sampleBSC(volatile BSCell *bSC){
	tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
	jtagWriteAndReadBSCells(bSC, CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH);
	tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
}

/*
 * This function triggers the process of sampling data from a Boundary Scan Cell (BSC).
 * It navigates the TAP (Test Access Port) state machine from the Run-Test/Idle state to 
 * the Shift-DR state, performs the read/write operations on the BSC and then returns to 
 * the Run-Test/Idle state.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 */
int bSCSampleGpioPin(volatile BSCell *bSC, BSReg bSReg){
	int val = 0;

	loadJtagIR(SAMPLE_PRELOAD_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
	sampleBSC(bSC);
	jtagSetIr(SAMPLE_PRELOAD);
	val = jtagReadBSRegInput(bSC, bSReg);
	return val;

}

/*
 * This function configures the specified pin of a Boundary Scan Cell (BSC) to the specified mode.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 *   - bSReg: A BSReg structure that defines the boundary scan register to be configured.
 *   - pinMode: A BSCPinMode enumeration value representing the desired pin mode.
 */
void bSCPinConfigure(volatile BSCell *bSC, BSReg bSReg, BSCPinMode pinMode){
	int arrayIndex = (bSReg.controlCellNum)/ 8;
	bSC->bSCellPreloadData[arrayIndex]  =  bSC->bSCellPreloadData[arrayIndex] | (pinMode << (bSReg.controlCellNum % 8));
}

/*
 * This function writes preload data to the specified boundary scan register output cell.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 *   - bSReg: A BSReg structure that defines the boundary scan register to be written.
 *   - data: The data to be preloaded into the boundary scan register output cell.
 */
void writePreloadData(volatile BSCell *bSC, BSReg bSReg, int data){
	int arrayIndex = (bSReg.outputCellNum)/ 8;
	bSC->bSCellPreloadData[arrayIndex]  =  bSC->bSCellPreloadData[arrayIndex] | (data << (bSReg.outputCellNum % 8));
}


/*
 * This function preloads the specified data into a boundary scan register output cell, 
 * navigates the TAP (Test Access Port) state machine from the Run-Test/Idle state to the Shift-DR state,
 * performs the read/write operations on the BSC, and then returns to the Run-Test/Idle state.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 *   - bSReg: A BSReg structure that defines the boundary scan register to be preloaded.
 *   - data: The data to be preloaded into the boundary scan register output cell.
 */
void bSCPreloadData(volatile BSCell *bSC, BSReg bSReg, int data){
	  writePreloadData(bSC, bSReg, data);
	  if(currentIR != EXTEST){
	  loadJtagIR(SAMPLE_PRELOAD_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
	  }
	  tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
	  jtagWriteAndReadBSCells(bSC, CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH);
	  tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
}

/*
 * This function preloads the specified data into a boundary scan register output cell and triggers the EXTEST instruction. 
 * If the current Instruction Register (IR) value is not EXTEST, it loads the EXTEST instruction into the IR.
 *
 * Parameters:
 *   - *bSC: A pointer to the BSCell structure that contains the boundary scan data.
 *   - bSReg: A BSReg structure that defines the boundary scan register to be written.
 *   - data: The data to be written into the boundary scan register output cell.
 */
void bSCExtestGpioPin(volatile BSCell *bSC, BSReg bSReg, int data){
	  bSCPreloadData(bSC, bSReg, data);
	  if(currentIR != EXTEST){
		  loadJtagIR(EXTEST_BSC_TAP_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, RUN_TEST_IDLE);
		  jtagSetIr(EXTEST);
	  }
}

/*
 * This function fetches a Boundary Scan Register (BSReg) object from a lookup table based on a string input. 
 * It first skips any whitespace in the input string, then reads characters into a buffer until 
 * a space or the end of the string is encountered. This buffer string is then used as a key to fetch 
 * a BSReg object from the lookup table.
 *
 * Parameters:
 *   - **str: A pointer to the string that needs to be parsed to fetch the BSReg object.
 *
 * Returns:
 *   - A BSReg object fetched from the lookup table based on the input string.
 */
BSReg getBSRegFromStr(char **str){
    char BSRegInStr[BSCELL_STR_LENGTH] = {0};
    int i = 0;
    // skip the blank spaces in string
    skipWhiteSpaces(&(*str));

    while(**str != ' ' && **str != '\0'){
        BSRegInStr[i] = **str;
        ++*str;
        i++;
    }

    return (getBSRegFromTable(BSRegInStr));

}

