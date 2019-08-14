
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <string.h>
#include "Tap.h"
#include "TAP_LookUpTable.h"
#include "global.h"
#include "BoundaryScan.h"
#include "USART.h"
//#include "BoundaryScan.c"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

// Instruction MACROs
#define CORTEX_M3_JTAG_INSTRUCTION_LENGTH	9
#define BYPASS_BOTH_TAP		0b111111111
#define READ_BOTH_IDCODE	0b000011110

#define READ_BSC_IDCODE_BYPASS_M3_TAP	0b000011111
#define BYPASS_BSC_TAP_READ_M3_IDCODE	0b111111110

#define SAMPLE_PRELOAD_BSC_TAP_BYPASS_M3_TAP	0b000101111
#define EXTEST_BSC_TAP_BYPASS_M3_TAP			0b000001111

// Miscellaneous MACROs
#define DUMMY_DATA			0x1234abcd
#define CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH	232

#define BUFFER_SIZE			128
#define BSCELL_STR_LENGTH	16
// HAL GPIO MACROs
#define setClock(clk)	HAL_GPIO_WritePin(TCK_GPIO_Port, TCK_Pin, clk);
#define setTms(tms)		HAL_GPIO_WritePin(TMS_GPIO_Port, TMS_Pin, tms);
#define writeTdi(tdi)	HAL_GPIO_WritePin(TDI_GPIO_Port, TDI_Pin, tdi);
#define readTdo()		HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin);


typedef enum{
	OUTPUT_PIN = 0,
	INPUT_PIN = 1,
}BSCPinMode;

typedef enum{
	BYPASS,
	IDCODE,
	SAMPLE_PRELOAD,
	EXTEST,
	DONT_CARE,
}JTAGInstruction;

StateTable stateTable[16][16] = {
  // current state is TEST_LOGIC_RESET
  {{1, TEST_LOGIC_RESET}, {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE},
   {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE},
   {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE},
   {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE},
   {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE}, {0, RUN_TEST_IDLE},
   {0, RUN_TEST_IDLE}},

  // current state is RUN_TEST_IDLE
  {{1, SELECT_DR_SCAN}, {0, RUN_TEST_IDLE}, {1, SELECT_DR_SCAN},
   {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
   {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
   {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
   {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
   {1, SELECT_DR_SCAN}},

  // current state is SELECT_DR_SCAN
  {{1, SELECT_IR_SCAN}, {1, SELECT_IR_SCAN}, {X, X},
   {0, CAPTURE_DR}, {0, CAPTURE_DR}, {0, CAPTURE_DR},
   {0, CAPTURE_DR}, {0, CAPTURE_DR}, {0, CAPTURE_DR},
   {1, SELECT_IR_SCAN}, {1, SELECT_IR_SCAN}, {1, SELECT_IR_SCAN},
   {1, SELECT_IR_SCAN}, {1, SELECT_IR_SCAN}, {1, SELECT_IR_SCAN},
   {1, SELECT_IR_SCAN}},

  // current state is CAPTURE_DR
  {{1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {X, X}, {0, SHIFT_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}},

  // current state is SHIFT_DR
  {{1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {0, SHIFT_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
   {1, EXIT1_DR}},

  // current state is EXIT1_DR
  {{1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
   {1, UPDATE_DR}, {0, PAUSE_DR}, {X, X},
   {0, PAUSE_DR}, {0, PAUSE_DR}, {1, UPDATE_DR},
   {1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
   {1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
   {1, UPDATE_DR}},

  // current state is PAUSE_DR
  {{1, EXIT2_DR}, {1, EXIT2_DR}, {1, EXIT2_DR},
   {1, EXIT2_DR}, {1, EXIT2_DR}, {1, EXIT2_DR},
   {0, PAUSE_DR}, {1, EXIT2_DR}, {1, EXIT2_DR},
   {1, EXIT2_DR}, {1, EXIT2_DR}, {1, EXIT2_DR},
   {1, EXIT2_DR}, {1, EXIT2_DR}, {1, EXIT2_DR},
   {1, EXIT2_DR}},

   // current state is EXIT2_DR
   {{1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
    {1, UPDATE_DR}, {0, SHIFT_DR}, {0, SHIFT_DR},
    {0, SHIFT_DR}, {X, X}, {1, UPDATE_DR},
    {1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
    {1, UPDATE_DR}, {1, UPDATE_DR}, {1, UPDATE_DR},
    {1, UPDATE_DR}},

    // current state is UPDATE_DR
    {{1, SELECT_DR_SCAN}, {0, RUN_TEST_IDLE}, {1, SELECT_DR_SCAN},
     {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
     {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {X, X},
     {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
     {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
     {1, SELECT_DR_SCAN}},

     // current state is SELECT_IR_SCAN
     {{1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET},
      {1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET},
      {1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET}, {1, TEST_LOGIC_RESET},
      {X, X}, {0, CAPTURE_IR}, {0, CAPTURE_IR},
      {0, CAPTURE_IR}, {0, CAPTURE_IR}, {0, CAPTURE_IR},
      {0, CAPTURE_IR}},

     // current state is CAPTURE_IR
     {{1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_DR},
      {1, EXIT1_DR}, {1, EXIT1_DR}, {1, EXIT1_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}, {X, X}, {0, SHIFT_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}},

     // current state is SHIFT_IR
     {{1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {0, SHIFT_IR},
      {1, EXIT1_IR}, {1, EXIT1_IR}, {1, EXIT1_IR},
      {1, EXIT1_IR}},

     // current state is EXIT1_IR
     {{1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
      {1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
      {1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
      {1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
      {1, UPDATE_IR}, {0, PAUSE_IR}, {0, PAUSE_IR},
      {1, UPDATE_IR}},

     // current state is PAUSE_IR
     {{1, EXIT2_IR}, {1, EXIT2_IR}, {1, EXIT2_IR},
      {1, EXIT2_IR}, {1, EXIT2_IR}, {1, EXIT2_IR},
      {1, EXIT2_IR}, {1, EXIT2_IR}, {1, EXIT2_IR},
      {1, EXIT2_IR}, {1, EXIT2_IR}, {1, EXIT2_IR},
      {1, EXIT2_IR}, {0, PAUSE_IR}, {1, EXIT2_IR},
      {1, EXIT2_IR}},

      // current state is EXIT2_IR
      {{1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
       {1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
       {1, UPDATE_IR}, {1, UPDATE_IR}, {1, UPDATE_IR},
       {1, UPDATE_IR}, {1, UPDATE_IR}, {0, SHIFT_IR},
       {0, SHIFT_IR}, {0, SHIFT_IR}, {1, UPDATE_IR},
       {1, UPDATE_IR}},

       // current state is UPDATE_IR
       {{1, SELECT_DR_SCAN}, {0, RUN_TEST_IDLE}, {1, SELECT_DR_SCAN},
        {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
        {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
        {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
        {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN}, {1, SELECT_DR_SCAN},
        {X, X}},


};

TapTestTable tapTrackTable[16] = {{TEST_LOGIC_RESET, RUN_TEST_IDLE, TEST_LOGIC_RESET},
                             {RUN_TEST_IDLE, RUN_TEST_IDLE, SELECT_DR_SCAN},
                             {SELECT_DR_SCAN, CAPTURE_DR, SELECT_IR_SCAN},
                             {CAPTURE_DR, SHIFT_DR, EXIT1_DR},
                             {SHIFT_DR, SHIFT_DR, EXIT1_DR},
                             {EXIT1_DR, PAUSE_DR, UPDATE_DR},
                             {PAUSE_DR, PAUSE_DR, EXIT2_DR},
                             {EXIT2_DR, SHIFT_DR, UPDATE_DR},
                             {UPDATE_DR, RUN_TEST_IDLE, SELECT_DR_SCAN},
                             {SELECT_IR_SCAN, CAPTURE_IR, TEST_LOGIC_RESET},
                             {CAPTURE_IR, SHIFT_IR, EXIT1_IR},
                             {SHIFT_IR, SHIFT_IR, EXIT1_IR},
                             {EXIT1_IR, PAUSE_IR, UPDATE_IR},
                             {PAUSE_IR, PAUSE_IR, EXIT2_IR},
                             {EXIT2_IR, SHIFT_IR, UPDATE_IR},
                             {UPDATE_IR, RUN_TEST_IDLE, SELECT_DR_SCAN},
                           };




uint64_t tms = 0;
uint64_t tdi = 0;
uint64_t tdo = 0;
uint32_t Count = 0;

TapState currentTapState = TEST_LOGIC_RESET;
JTAGInstruction currentIR = DONT_CARE;
volatile BSCell bsc1;
char tempBuffer[BUFFER_SIZE];
char menuBuffer[BUFFER_SIZE];
uint64_t tempVal = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void jtagDelay(uint32_t cycles){
	while(cycles--);
}


TapState updateTapState(TapState currentState, int tms){
	TapState retval;
      if(tms == 1)
    	  retval =  (tapTrackTable[currentState].nextState_tms1);
      else if(tms == 0)
    	  retval =  (tapTrackTable[currentState].nextState_tms0);
    return retval;
}

void tckCycle(){
	setClock(0);
	jtagDelay(500);
	setClock(1);
	jtagDelay(500);
}

void tdiCycle(int tdi){
	writeTdi(tdi);
	setClock(0);
	jtagDelay(500);
	setClock(1);
	jtagDelay(500);
}

int tdoCycle(){
	int val = 0;
	setClock(0);
	val = readTdo();
	return val;

}

#define tdioCycle(tdi, tdo)	\
	setClock(0);		\
	jtagDelay(500);		\
	tdo = setTdi(tdi);	\
	setClock(0);		\
	jtagDelay(500);		\


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

/*
 * Reset the TAP controller state to TEST_LOGIC_RESET
 * state.
 */
void resetTapState(){
	int i = 0;

	 while(i < 5){
	   jtagClkIoTms(0, 1);
	   currentTapState = updateTapState(currentTapState, 1);
	   i++;
	 }
}

/*
 * Transition of TAP state from parameter "start"
 * to "end"
 */
void tapTravelFromTo(TapState start, TapState end){
    int tmsRequired;

      while(currentTapState != end){
          tmsRequired = getTmsRequired(currentTapState, end);
          jtagClkIoTms(0, tmsRequired);
          currentTapState = updateTapState(currentTapState, tmsRequired);
          }
}

void jtagWriteTms(uint64_t data, int length){
  int dataMask = 1;
  int oneBitData = 0;

  while(length > 0){
    oneBitData = dataMask & data;
    jtagClkIoTms(0, oneBitData);
    length--;
    data = data >> 1;
  }
}


void switchSwdToJtagMode(){
	jtagWriteTms(0x3FFFFFFFFFFFF, 50);
	jtagWriteTms(0xE73C, 16);
	resetTapState();
}

uint64_t jtagWriteAndReadBits(uint64_t data, int length){
	int dataMask = 1;
	int oneBitData = 0;
	uint64_t tdoData = 0;
	int i = 0;
	int n = 0;
	uint64_t outData = 0;

	// noted that last bit of data must be set at next tap state
	for (n = length ; n > 1; n--) {
	  oneBitData = dataMask & data;
	  tdoData = jtagClkIoTms(oneBitData, 0);
	  currentTapState = updateTapState(currentTapState, 0);
	  outData |= tdoData << (i*1);
	  data = data >> 1;
	  i++;
	}
	oneBitData = dataMask & data;
	tdoData = jtagClkIoTms(oneBitData, 1);
	currentTapState = updateTapState(currentTapState, 1);
	outData |= tdoData << (i*1);;
	return outData;
}

void loadJtagIR(int instructionCode, int length, TapState start){
	tapTravelFromTo(start, SHIFT_IR);
	jtagWriteAndReadBits(instructionCode, length);
	tapTravelFromTo(EXIT1_IR, RUN_TEST_IDLE);
}

uint64_t jtagWriteAndRead(uint64_t data, int length){
  uint64_t outData = 0;
  tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
  outData = jtagWriteAndReadBits(data, length);
  tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
  return outData;
}

uint64_t jtagBypass(int instruction, int instructionLength, int data, int dataLength){
	uint64_t valRead = 0;
	loadJtagIR(instruction, instructionLength, RUN_TEST_IDLE);
	valRead = jtagWriteAndRead(data,dataLength);
	return valRead;
}

void loadBypassRegister(int instruction, int instructionLength, int data, int dataLength){
	loadJtagIR(instruction, instructionLength, RUN_TEST_IDLE);
	jtagWriteAndRead(data, dataLength);
}

uint64_t jtagReadIdCode(int instructionCode, int instructionLength, int data, int dataLength){
	uint64_t valRead = 0;
	loadJtagIR(instructionCode, instructionLength, RUN_TEST_IDLE);
	valRead = jtagWriteAndRead(data, dataLength);
	return valRead;
}

uint64_t jtagReadIDCodeResetTAP(int data, int dataLength){
	uint64_t valRead = 0;
	resetTapState();
	tapTravelFromTo(TEST_LOGIC_RESET, SHIFT_DR);
	valRead = jtagWriteAndReadBits(data, dataLength);
	tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
	return valRead;
}

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

void jtagSetIr(JTAGInstruction instruction){
	currentIR = instruction;
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

void skipWhiteSpaces(char **str){

    while(**str == ' ')
        ++*str;
}




BSReg *getBSRegFromStr(char *str){
    char BSRegInStr[BSCELL_STR_LENGTH];
    int i = 0;
    // skip the blank spaces in string
    skipWhiteSpaces2(&str);

    while(*str != ' ' && *str != '\0'){
        BSRegInStr[i] = *str;
        str++;
        i++;
    }

    if(!(strcasecmp(BSRegInStr, "pa9")))
        return &pa9;

}


void commandLineOperation(char *commandStr){
	int i = 0;
	if(!(strcasecmp(commandStr, "jtag idcode"))){
		tempVal = jtagReadIdCode(READ_BOTH_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, DUMMY_DATA, 64);
		uint32_t bSCID = 0, cortexM3ID = 0;
		cortexM3ID = tempVal & 0xffffffff;
		bSCID = tempVal>>32;
		sprintf(tempBuffer, "Cortex M3 ID Code : 0x%X \nBoundary Scan Cell ID Code : 0x%X \n\n", cortexM3ID, bSCID);
	}
	//else if(!(strcasecmp(commandStr, "jtag sample")))
	uartTransmitBuffer(uart1, tempBuffer);

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	volatile uint64_t val = 0, i = 0;
	volatile char commandBuffer[BUFFER_SIZE];
	sprintf( menuBuffer, "\t\tWelcome to JTAG Interface for STM32F103C8T6, \n \t\t\t type 'help' for enquiry \n\n");
	BSReg *bSGPtr;
	bSGPtr = getBSRegFromStr("pa9");
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  switchSwdToJtagMode();
  jtagIoInit();

  /*	Bypass Instruction
  	   * 	Instruction op-code : 9 bit of 1
  	   * 	1. Load the DR of Boundary Scan(BS) and Cortex TAP
  	   * 	   with 1.
  	   * 	2. Send data of 0b1110001 to shift out.
  	   * 	3. Expected data is
  	   *
  	   * 							BS TAP  Cortex TAP
  	   * 	TDI : 	0b1110001		0b		0b
  	   * 			--------------> ------> ----> 	TDO
  	   * 	Expect result from TDO is 0b111000100	(LSB first)
  	   * */
  	  val = jtagBypass(BYPASS_BOTH_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0b1110001, 10);
  	  val = 0;

  	  /*	IDCODE Instruction
  	   * 	Instruction op-code : 0b000011110
  	   *	1. Load the IR of Boundary Scan(BS) with 0b00001
  	   *	   and Cortex TAP with 0b1110
  	   *    2. Shift out the IDCODE from both TAP's DR.
  	   *    3. Expected result is
  	   *        			BS TAP		Cortex TAP
  	   *	TDI : 		0x16410041		0x3ba00477(LSB)
  	   *
  	   *	Expect result from TDO is 0x164100413ba00477
  	   * */
  	  val = jtagReadIdCode(READ_BOTH_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, DUMMY_DATA, 64);
  	  val = 0;

  	  /*	Get IDCODE after reset TAP state to TEST_LOGIC_RESET
  	   * 	Expect result from TDO is 0x164100413ba00477
  	   * */
  	  val = jtagReadIDCodeResetTAP(DUMMY_DATA, 64);
  	  val = 0;

  	  /*	Get IDCODE for BS TAP and bypass Cortex TAP
  	   *	retval need to shift right by 1 bit bcoz of
  	   *	bypass bit for Cortex TAP
  	   *
  	   *	Expect result from TDO is 0xXXXXXXXX16410041
  	   * */
  	  val = jtagReadIdCode(READ_BSC_IDCODE_BYPASS_M3_TAP, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0xffffffffffffffff, 64);
  	  val = val>>1;
  	  val = 0;

  	  /*	Bypass BS TAP and get IDCODE for Cortex TAP
  	   *
  	   * 	Expect result from TDO is 0xXXXXXXXX3ba00477
  	   * */
  	  val = jtagReadIdCode(BYPASS_BSC_TAP_READ_M3_IDCODE, CORTEX_M3_JTAG_INSTRUCTION_LENGTH, 0xffffffffffffffff, 64);
  	  val = 0;

  	  /*	SAMPLE Instruction
  	   * 	Sample the Gpio pin while the MCU is running in normal operation
  	   *
  	   * 	Example
  	   * 	When pin pa12 is connected to GND the expected val is 0,
  	   * 	if pin pa12 is connected to 3.3V the expected val is 1
  	   * */
  	  bSCInIt(&bsc1);
  	  bSCPinConfigure(&bsc1, pa12, INPUT_PIN);
  	  val = bSCSampleGpioPin(&bsc1, pa12);
  	  /*	PRELOAD Instruction
  	   * 	Loaded the test pattern that will use for EXTEST later
  	   *
  	   * 	Example
  	   *	Load '1' at the output cell of pb9 (tri-stated).
  	   *	When loading EXTEST instruction, pb9 will output HIGH '1'
  	   *
  	   * */
  	  writePreloadData(&bsc1, pa12, 1);
  	  val = jtagReadBSCPin(&bsc1, pa12.outputCellNum, PRELOAD_DATA);

  	  /*	EXTEST Instruction
  	   * 	Apply the test pattern preloaded from PRELOAD Instruction
  	   * 	to the circuit
  	   *
  	   * 	NOTICE : Make sure the currentIR is not EXTEST for first time
  	   * 			 then update the currentIR to EXTEST
  	   * */
  	 bSCPinConfigure(&bsc1, pa9, OUTPUT_PIN);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 1);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
  	 bSCExtestGpioPin(&bsc1, pa9, 0);
   	uartTransmitBuffer(uart1, menuBuffer);
   	bSGPtr = getBSRegFromStr("pa9");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  restart:
	  		  if(usartIsRxRegNotEmpty(uart1)){
	  			  commandBuffer[i] = (uart1)->DR;
	  			  if(usartIsTxRegEmpty(uart1)){
	  				  (uart1)->DR = commandBuffer[i];
	  			  }
	  			  if(commandBuffer[i] == '\b'){
	  				  // check for backspace
	  				  i--;
	  				  goto restart;
	  			  }
	  			  if(commandBuffer[i] == '\n'){
	  				  commandBuffer[i] = '\0';
	  				  commandLineOperation(commandBuffer);
	  				  i = 0;
	  				  goto restart;
	  			  }
	  			  i++;

	  	  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TCK_Pin|TMS_Pin|TDI_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : TDO_Pin */
  GPIO_InitStruct.Pin = TDO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TDO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TCK_Pin TMS_Pin TDI_Pin */
  GPIO_InitStruct.Pin = TCK_Pin|TMS_Pin|TDI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
