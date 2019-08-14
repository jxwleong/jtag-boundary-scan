#ifndef _BOUNDARYSCAN_H
#define _BOUNDARYSCAN_H



// Boundary Scan MACROs
#define CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH		232
#define BOUNDARY_SCAN_CELL_DIV_8				CORTEX_M3_BOUNDARY_SCAN_CELL_LENGTH/8

typedef enum{
	SAMPLE_DATA,
	PRELOAD_DATA
}BSCDataType;

typedef struct BSReg BSReg;
struct BSReg{
	int inputCellNum;
	int outputCellNum;
	int controlCellNum;
};

typedef struct BSCell BSCell;
struct BSCell{
	uint8_t bSCellSampleData[BOUNDARY_SCAN_CELL_DIV_8 + 1];
	uint8_t bSCellPreloadData[BOUNDARY_SCAN_CELL_DIV_8 + 1];
};



#endif // _BOUNDARYSCAN_H
