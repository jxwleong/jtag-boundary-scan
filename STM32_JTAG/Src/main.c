
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
#include "Tap.h"
#include "TAP_LookUpTable.h"
#include "global.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
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

uint64_t tms = 0;
uint64_t tdi = 0;
uint64_t tdo = 0;
uint32_t Count = 0;
uint64_t jtagReadIDCode(){
//uint32_t val;
}

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void jtagDelay(uint32_t cycles){
	while(cycles--);
}


void setClock(int clk){
	HAL_GPIO_WritePin(TCK_GPIO_Port, TCK_Pin, clk);
}

void setTdi(int tdi){
	HAL_GPIO_WritePin(TDI_GPIO_Port, TDI_Pin, tdi);
}

void setTms(int tms){
	HAL_GPIO_WritePin(TMS_GPIO_Port, TMS_Pin, tms);
}

int readTdo(){
	int tdo = HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin);
	return tdo;
}

void tckCycle(){
	setClock(0);
	jtagDelay(500);
	setClock(1);
	jtagDelay(500);
}

void tdicycle(int tdi){
	setTdi(tdi);
	setClock(0);
	jtagDelay(500);
	setClock(1);
	jtagDelay(500);
}

#define tdioCycle(tdi, tdo)	\
	setClock(0);		\
	jtagDelay(500);		\
	tdo = setTdi(tdi);	\
	setClock(0);		\
	jtagDelay(500);		\


void jtagIOInit(){
	 tms = 0;
	 tdi = 0;
	 tdo = 0;
	 Count = 0;
}

void resetTapState(){
	int i = 0;

	 while(i < 5){
	   setTms(1);
	   tckCycle();
	   tms |= 1 << (Count*1);
	   tdi |= 0 << (Count*1);
	   tdo |= 0 << (Count*1);
	   Count++;
	   i++;
	 }
}


void tapTravelFromTo(TapState start, TapState end){
    TapState currentState = start;
    int tmsRequired;
    int i = 0;

      while(currentState != end){
          tmsRequired = getTmsRequired(currentState, end);
          setTms(tmsRequired);
          tckCycle();
          tms |= tmsRequired << (Count*1);
          tdi |= 0 << (Count*1);
          tdo |= 0 << (Count*1);
          Count++;
          currentState = stateTable[currentState][end].nextState;
          i++;
          }
}



void jtagWriteTms(uint64_t data, int length){
  int dataMask = 1;
  int oneBitData = 0;

  while(length > 0){
    oneBitData = dataMask & data;
    setTms(oneBitData);
    tckCycle();
    length--;
    data = data >> 1;
  }
}


void switchSwdToJtagMode(){
	jtagWriteTms(0x3FFFFFFFFFFFF, 50);
	jtagWriteTms(0xE73C, 16);
	resetTapState();
}


int jtagWriteAndReadBit(int data){
	int tdo;
	setTdi(data);
	jtagDelay(100);
	setClock(1);		// Insert the input first
	jtagDelay(500);
	setClock(0);
	//jtagDelay(500);
	tdo = HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin);
	return tdo;//
}

uint64_t jtagWriteAndReadBits(uint64_t data, int length){
	int dataMask = 1;
	int oneBitData = 0;
	int tdoData = 0;
	int i = 0;
	int n = 0;
	uint64_t outData = 0;

	// noted that last bit of data must be set at next tap state
	for (n = length ; n > 1; n--) {
	  oneBitData = dataMask & data;
	  setTms(0);
	  tdoData = jtagWriteAndReadBit(oneBitData);
      tdo |= tdoData << (Count*1);
	  outData |= tdoData << (i*1);
      tms |= 0 << (Count*1);
      tdi |= oneBitData << (Count*1);
      Count++;
	  data = data >> 1;
	  i++;
	}
	oneBitData = dataMask & data;
	setTms(1);
	jtagDelay(100);
	tdoData = jtagWriteAndReadBit(oneBitData);
	tdo |= tdoData << (Count*1);
	outData |= tdoData << (i*1);;
	tdi |= oneBitData << (Count*1);
    tms |= 1 << (Count*1);
    Count++;
	return outData;
}

void loadJtagIR(int instructionCode, int length){
  resetTapState();
  tapTravelFromTo(TEST_LOGIC_RESET, SHIFT_IR);
  jtagWriteAndReadBits(instructionCode, length);
  tapTravelFromTo(EXIT1_IR, RUN_TEST_IDLE);
}


uint64_t jtagWriteAndRead(uint64_t data, int length){
  uint64_t outData;
  tapTravelFromTo(RUN_TEST_IDLE, SHIFT_DR);
  outData = jtagWriteAndReadBits(data, length);
  tapTravelFromTo(EXIT1_DR, RUN_TEST_IDLE);
  return outData;
}

uint64_t jtagBypass(int data, int length){
	uint64_t val = 0;
	loadJtagIR(0b111111111, length);
	jtagIOInit();
	val = jtagWriteAndRead(data,length);
	return val;
}

void loadBypassRegister(int bypassData, int length){
	loadJtagIR(0b111111111, length);
	jtagIOInit();
	jtagWriteAndRead(bypassData, length);

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
	volatile uint64_t val = 0;
	int i = 0;
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
  /* USER CODE BEGIN 2 */
/*
  switchSwdToJtagMode();
  jtagIOInit();
  loadBypassRegister(0b00, 2);
  jtagIOInit();
  val = jtagBypass(0b1110001, 9);
*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /*	Bypass Instruction
	   * 	Instruction op-code : 9 bit of 1
	   * 	1. Load the DR of Boundary Scan(BS) and Cortex TAP
	   * 	   with 1.
	   * 	2. Send data of 0b1110001 to shift out.
	   * 	3. Expected data is
	   *
	   * 							BS TAP  Cortex TAP
	   * 	TDI : 	0b1110001		0b1		0b1
	   * 			--------------> ------> ----> 	TDO
	   * 	Expect result from TDO is 0b111000111	(LSB first)
	   */


	  val = 0;

	  setClock(1);
	  jtagDelay(500);

	  setClock(0);
	  val = HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin);
	  jtagDelay(500);
	  /*
	  val = 1;
	  setClock(1);
	  val = HAL_GPIO_ReadPin(TMS_GPIO_Port, TMS_Pin);
	  jtagDelay(500);
	  setClock(0);
	  jtagDelay(500);
*/
/*
	  val = 0;
	  setClock(1);
	  jtagDelay(500);
	  HAL_GPIO_WritePin(TDO_GPIO_Port, TDO_Pin, 0);
	  setClock(0);
	  val |= HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin) << (i*1);
	  i++;

	  setClock(1);
	  jtagDelay(500);
	  HAL_GPIO_WritePin(TDO_GPIO_Port, TDO_Pin, 1);
	  setClock(0);
	  val |= HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin) << (i*1);
	  i++;

	  setClock(1);
	  jtagDelay(500);
	  HAL_GPIO_WritePin(TDO_GPIO_Port, TDO_Pin, 0);
	  setClock(0);
	  val |= HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin) << (i*1);
	  i++;

	  setClock(1);
	  jtagDelay(500);
	  HAL_GPIO_WritePin(TDO_GPIO_Port, TDO_Pin, 1);
	  setClock(0);
	  val |= HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin) << (i*1);
	  i++;

	  setClock(1);
	  jtagDelay(500);
	  HAL_GPIO_WritePin(TDO_GPIO_Port, TDO_Pin, 0);
	  setClock(0);
	  val |= HAL_GPIO_ReadPin(TDO_GPIO_Port, TDO_Pin) << (i*1);
	  i++;
*/


	  //resetTapState();
	  //val = jtagWriteAndRead(0x00000000, 64);
	  //tapTravelFromTo(TEST_LOGIC_RESET, SHIFT_IR);
	  //jtagWriteAndReadBits(0b10, 2);

	  //110010010
	  //110010010



	 //jtagReadBits(0x00000000, 32);
	//loadJtagDR(0x0ffff0000,33);

	//loadJtagIR(0b111111110,9);
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
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL13;
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
