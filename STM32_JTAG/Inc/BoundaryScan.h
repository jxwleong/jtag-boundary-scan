#ifndef _BOUNDARYSCAN_H
#define _BOUNDARYSCAN_H

#include <stdint.h>

// Boundary Scan MACROs
#define CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH		232
#define BOUNDARY_SCAN_CELL_DIV_8				CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH/8

typedef struct BSReg BSReg;
struct BSReg{
	int inputCellNum;
	int outputCellNum;
	int controlCellNum;
	char *pinName;
};

typedef enum{
	SAMPLE_DATA,
	PRELOAD_DATA
}BSCDataType;



typedef enum{
	OUTPUT_PIN = 0,
	INPUT_PIN = 1,
}BSCPinMode;


typedef struct BSCell BSCell;
struct BSCell{
	uint8_t bSCellSampleData[BOUNDARY_SCAN_CELL_DIV_8 + 1];
	uint8_t bSCellPreloadData[BOUNDARY_SCAN_CELL_DIV_8 + 1];
};

void bSCInIt(volatile BSCell *bSC);
void jtagWriteAndReadBSCells(volatile BSCell *bSC, int length);
int jtagReadBSRegInput(volatile BSCell *bSC, BSReg bSReg);
int jtagReadBSCPin(volatile BSCell *bSC, int pin, BSCDataType bSCDataType);
void sampleBSC(volatile BSCell *bSC);
int bSCSampleGpioPin(volatile BSCell *bSC, BSReg bSReg);
void bSCPinConfigure(volatile BSCell *bSC, BSReg bSReg, BSCPinMode pinMode);
void writePreloadData(volatile BSCell *bSC, BSReg bSReg, int data);
void bSCPreloadData(volatile BSCell *bSC, BSReg bSReg, int data);
void bSCExtestGpioPin(volatile BSCell *bSC, BSReg bSReg, int data);

BSReg getBSRegFromStr(char *str);

#endif // _BOUNDARYSCAN_H
