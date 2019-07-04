#include <stdio.h>
#include "TAP_LookUpTable.h"
#include "TAP.h"
#include "TAP_Mock.h"
#include "UnityErrorHandler.h"
#include "global.h"

//JtagState jtagState;  // must define else linker cant link

/*  @desc    return the name of TAP state in string instead of number(enum)
    @retval  TAP state in string
*/
char *getTapStateName(TapState tapState){
  switch(tapState){
    case TEST_LOGIC_RESET : return "TEST_LOGIC_RESET"; break;
    case RUN_TEST_IDLE    : return "RUN_TEST_IDLE"; break;
    case SELECT_DR_SCAN   : return "SELECT_DR_SCAN"; break;
    case CAPTURE_DR       : return "CAPTURE_DR"; break;
    case SHIFT_DR         : return "SHIFT_DR"; break;
    case EXIT1_DR         : return "EXIT1_DR"; break;
    case PAUSE_DR         : return "PAUSE_DR"; break;
    case EXIT2_DR         : return "EXIT2_DR"; break;
    case UPDATE_DR        : return "UPDATE_DR"; break;
    case SELECT_IR_SCAN   : return "SELECT_IR_SCAN"; break;
    case CAPTURE_IR       : return "CAPTURE_IR"; break;
    case SHIFT_IR         : return "SHIFT_IR"; break;
    case EXIT1_IR         : return "EXIT1_IR"; break;
    case PAUSE_IR         : return "PAUSE_IR"; break;
    case EXIT2_IR         : return "EXIT2_IR"; break;
    case UPDATE_IR        : return "UPDATE_IR"; break;
    default: return "Invalid TAP state";
  }
}

/*  @desc   return the current TAP state after funcion calls
    @param  jtagState which contain the updated TAP state between
            function calls
    @retval Tapstate that keep updating between function calls
*/
/*
TapState getCurrentState(JtagState currentState){
  return currentState.state;
}*/




/*  @param current TAP State and current TMS state
    @desc  Reset to TEST_LOGIC_RESET state no matter
            current state.
*/
void tapTestLogicReset(){
  int i = 0;

  while(i < 5){
    tapStep(1, 0);// int tapStep(tms, tdi)
    jtagState.state = getTapState(jtagState.state, 1);
    i++;
  }
}


/*  @param the data desired to write and the length of the data
    @desc  write the data into Data Register or Instruction Register
           based on the TAP state (SHIFT_DR or SHIFT_IR)
*/
void tapWriteBits(uint64_t data, int length){

}

/*  @desc  shifft the input data from tdi into JTAG devices, meanwhile
*/
//void tapShiftBits(TckState tck, TdiState tdi){}
