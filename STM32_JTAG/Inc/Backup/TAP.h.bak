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

typedef enum{
  JTAG_WRITE_BITS = 0,
  JTAG_READ_BITS = 1,
  JTAG_TAP_TRAVEL = 2,
}JtagOperation;

typedef struct JtagState JtagState;
struct JtagState{
  TapState state;
  uint64_t DataReg;
  JtagOperation operation;
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



char *getTapStateName(TapState tapState);
void jtagIoInit();
int jtagClkIoTms(int data, int tms);
void resetTapState();
void tapTravelFromTo(TapState start, TapState end);
void jtagWriteTms(uint64_t data, int length);
void switchSwdToJtagMode();
uint64_t jtagWriteAndReadBits(uint64_t data, int length);
void loadJtagIR(int instructionCode, int length, TapState start);
uint64_t jtagWriteAndRead(uint64_t data, int length);
uint64_t jtagBypass(int instruction, int instructionLength, int data, int dataLength);
void loadBypassRegister(int instruction, int instructionLength, int data, int dataLength);
uint64_t jtagReadIdCode(int instructionCode, int instructionLength, int data, int dataLength);
uint64_t jtagReadIDCodeResetTAP(int data, int dataLength);

#endif // _TAP_FUNC_H
