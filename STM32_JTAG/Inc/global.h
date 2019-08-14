#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "TAP.h"
#include "main.h"

extern JtagState jtagState;

typedef enum{
	BYPASS,
	IDCODE,
	SAMPLE_PRELOAD,
	EXTEST,
	DONT_CARE,
}JTAGInstruction;


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



void jtagSetIr(JTAGInstruction instruction);
void jtagDelay(uint32_t cycles);

#endif // _GLOBAL_H
