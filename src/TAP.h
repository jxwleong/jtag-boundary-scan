#ifndef _TAP_H
#define _TAP_H

#include <stdint.h>

/*defination for TAP state machine*/
typedef enum{
  TEST_LOGIC_RESET = 0,
  RUN_TEST_IDLE = 1,
  SELECT_DR_SCAN = 2,
  CAPTURE_DR = 3,
  SHIFT_DR = 4,
  EXIT1_DR = 5,
  PAUSE_DR = 6,
  EXIT2_DR = 7,
  UPDATE_DR = 8,
  SELECT_IR_SCAN = 9,
  CAPTURE_IR = 10,
  SHIFT_IR = 11,
  EXIT1_IR = 12,
  PAUSE_IR = 13,
  EXIT2_IR = 14,
  UPDATE_IR = 15,
  X = 16,
  END = 17,
}TapState;

typedef struct JtagState JtagState;
struct JtagState{
  TapState state;
  uint64_t inData;
  uint64_t outData;
};


typedef struct TapSequence TapSequence;
struct TapSequence{
  TapState state;
  uint64_t tdi;
  uint64_t tdo;
  int bits;
};



typedef struct TapTestTable TapTestTable;
struct TapTestTable{
  TapState currentState;
  TapState nextState_tms0;
  TapState nextState_tms1;
};


void tapTestLogicReset();
char *getTapStateName(TapState tapState);
void tapWriteBits(uint64_t data, int length);
int tapReadBits(int length);

#endif // _TAP_FUNC_H
