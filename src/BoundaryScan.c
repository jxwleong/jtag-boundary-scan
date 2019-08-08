include "BoudaryScan.h"

void jtagWriteAndReadBSCells(BSCell bSC, int length){
	int dataMask = 1;
	int oneBitData = 0;
	uint64_t tdoData = 0;
	uint64_t i = 0;
	int n = 0;

	// noted that last bit of data must be set at next tap state
	for (n = length ; n > 1; n--) {
		if(i/64 == 1)
			i = 0;
	  oneBitData = dataMask & bSC.bSCellWriteData[i/64];
	  tdoData = jtagClkIoTms(oneBitData, 0);
	  currentTapState = updateTapState(currentTapState, 0);
	  // DIV by 64 to get the index for uint64_t array
	  bSC.bSCellReadData[i/64] |= tdoData << (i*1);
	  bSC.bSCellWriteData[i/64] = bSC.bSCellWriteData[i/64] >> 1;
	  i++;
	}
	oneBitData = dataMask & bSC.bSCellWriteData[i/64];
	tdoData = jtagClkIoTms(oneBitData, 1);
	currentTapState = updateTapState(currentTapState, 1);
	bSC.bSCellReadData[i/64] |= tdoData << (i*1);;
}

int getSubtractNumber(int inputCellNum){
	if(inputCellNum < 64)
		return 0;
	else  if(inputCellNum < 128)
		return 64;
	else if(inputCellNum < 192)
		return 192;
	else
		return 256;
}

int jtagReadBSRegInput(BSCell bSC, BSReg bSReg){
	int bSRegInput = 0;
	int mask = 1;

	int arrayIndex = (bSReg.inputCellNum)/ 64;
	bSRegInput = (bSC.bSCellReadData[arrayIndex]) >> (bSReg.inputCellNum - (getSubtractNumber(bSReg.inputCellNum)));
	bSRegInput &= mask;
	return bSRegInput;
}
