#include <unity.h>
#include <stdio.h>
#include <inttypes.h>
#include "TAP.h"
#include "cmock.h"
#include "mock_jtagLowLevel.h"
#include "TAP_LookUpTable.h"
#include "UnityErrorHandler.h"
#include "CExceptionConfig.h"
#include "CException.h"
#include "Exception.h"
#include "error.h"
#include "global.h"

// ceedling does not compile main.c
// so it will show undefined reference
JtagState jtagState = {TEST_LOGIC_RESET, 0, 0};
TapState currentTapState = TEST_LOGIC_RESET;
JTAGInstruction currentIR = DONT_CARE;

TapTestTable tapTable[16] = {{TEST_LOGIC_RESET, RUN_TEST_IDLE, TEST_LOGIC_RESET},
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

TapSequence *tapSeqPtr = NULL;
int tapSeqIndex = 0;

void setUp(void){}

void tearDown(void){}


int getTmsFromSeq(){
  if(tapTable[(tapSeqPtr[tapSeqIndex].state)].nextState_tms0 == tapSeqPtr[tapSeqIndex+1].state)
    return 0;

  else if(tapTable[(tapSeqPtr[tapSeqIndex].state)].nextState_tms1 == tapSeqPtr[tapSeqIndex+1].state)
    return 1;
  else
    unityError("On entry %i, transition from %s to %s is invalid.",
    tapSeqIndex,
    getTapStateName(tapSeqPtr[tapSeqIndex].state),
    getTapStateName(tapSeqPtr[tapSeqIndex+1].state));
}

int jtagStateMachineTester(int tms, int tdi){
  int retval = 0;
  int seqTms, seqTdi;
  TapState scriptState = (tapSeqPtr[tapSeqIndex+1].state);


      switch(jtagState.operation){
        case JTAG_WRITE_BITS:
        case JTAG_TAP_TRAVEL:
        if(scriptState != END){
         seqTms = getTmsFromSeq();
         seqTdi = (tapSeqPtr[tapSeqIndex+1].tdi);


        if(tms == seqTms){
          if(tdi == (seqTdi&1)){
            if((tapSeqPtr[tapSeqIndex+1].bits)>1){
              (tapSeqPtr[tapSeqIndex+1].bits)--;
              (tapSeqPtr[tapSeqIndex+1].tdi)>>=1;
            }
            else{
              verifyTapState();
              tapSeqIndex++;
            }
          }
          else
            unityError("On entry %i, expected tdi is %i but received %i",
            tapSeqIndex+1, (seqTdi&1), tdi);
          }

      else{
        unityError("On entry %i, expected tms is %i but received %i",
        tapSeqIndex+1, seqTms, tms);
      }
    }
      break;
      case JTAG_READ_BITS:
        verifyTapState();
        retval = returnReadBit();
        return retval;

      }


}


void verifyTapState(){
    TapState scriptState = (tapSeqPtr[tapSeqIndex].state);

    if(scriptState != jtagState.state){
      unityError("On entry %i, expected TAP state was %s but actual state is %s",
      tapSeqIndex, getTapStateName(scriptState), getTapStateName(jtagState.state));
    }
    else{
      printf("\nThe expecting TAP state %s is equal to actual state %s",
     getTapStateName(scriptState), getTapStateName(jtagState.state));
    }
}

int returnReadBit(){
  int retval;
  retval = (tapSeqPtr[tapSeqIndex].tdo) & 1;
  (tapSeqPtr[tapSeqIndex].tdo)>>=1;
  return retval;
}

int fake_jtagClkIoTms(int tms, int tdi, int CallNum){
  int retval;
  retval = jtagStateMachineTester(tms, tdi);
  printf("\nFunction jtagClkIoTms was called :%i times", CallNum+1);
  return retval;
}


void setupFakeTapSeq(TapSequence tapSeq[]){
  jtagClkIoTms_StubWithCallback(fake_jtagClkIoTms);

  tapSeqPtr = tapSeq;
  tapSeqIndex = 0;
}

void verifyTapSequence(TapSequence tapSeq[]){
    TapState scriptState = (tapSeqPtr[tapSeqIndex+1].state);

    if(scriptState == END){
      printf("\nThe operation is completed!");
    }
    else{
      unityError("On entry %i, expecting END state but received %s",
      tapSeqIndex , getTapStateName(scriptState));
      // minus by one bcoz at jtagStateMachineTester alrdy increment by 1
    }
}

/*Base on TAP controller state machine, a sequence
  of 5 TMS HIGH is required to reset to TestLogicReset
  state, TAP state machine only travel at rising edge so
  TMS must be set before the rising edge*/

/*
void test_resetTapState_given_extra_seq_on_entry_5_expect_error_msg(void){


    // Initialize current state of TAP
    jtagState.state = TEST_LOGIC_RESET;
    jtagState.DataReg = 0;

    jtagState.operation = JTAG_TAP_TRAVEL;

    TapSequence tapSeq[] = {
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {END, 0, 0, 1}};

    setupFakeTapSeq(tapSeq);
    resetTapState();

    TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, jtagState.state);
    verifyTapSequence(tapSeq);

}


void test_resetTapState_given_correct_seq_expect_TAP_LOGIC_RESET_state(void){

    // Initialize current state of TAP and endState
    jtagState.state = TEST_LOGIC_RESET;
    jtagState.DataReg = 0;

    jtagState.operation = JTAG_TAP_TRAVEL;

    TapSequence tapSeq[] = {
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {END, 0, 0, 1}};

    setupFakeTapSeq(tapSeq);
    resetTapState();

    TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, jtagState.state);
    verifyTapSequence(tapSeq);

}


void test_resetTapState_given_wrong_TAP_seq_start_from_SELECT_DR_SCAN_expect_error_msg(void){

    // Initialize current state of TAP and endState
    jtagState.state = SELECT_DR_SCAN;
    jtagState.DataReg = 0;

    jtagState.operation = JTAG_TAP_TRAVEL;

    TapSequence tapSeq[] = {
    {SELECT_DR_SCAN, 0, 0, 1},
    {UPDATE_DR, 0, 0, 1},     // expect SELECT_IR_SCAN
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {END, 0, 0, 1}};

    setupFakeTapSeq(tapSeq);
    resetTapState();

    TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, jtagState.state);
    verifyTapSequence(tapSeq);

}

void test_resetTapState_given_correct_seq_start_from_CAPTURE_DR_expect_TAP_LOGIC_RESET_state(void){

    // Initialize current state of TAP and endState
    jtagState.state = CAPTURE_DR;
    jtagState.DataReg = 0;

    jtagState.operation = JTAG_TAP_TRAVEL;

    TapSequence tapSeq[] = {
    {CAPTURE_DR, 0, 0, 1},
    {EXIT1_DR, 0, 0, 1},
    {UPDATE_DR, 0, 0, 1},
    {SELECT_DR_SCAN, 0, 0, 1},
    {SELECT_IR_SCAN, 0, 0, 1},
    {TEST_LOGIC_RESET, 0, 0, 1},
    {END, 0, 0, 0}};

    setupFakeTapSeq(tapSeq);
    resetTapState();

    TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, jtagState.state);
    verifyTapSequence(tapSeq);

}


*/
void test_getTapStateName_given_X_expect_Invalid_TAP_state_string(void)
{
  TapState tapstate;
  char *tapStateName;

  tapStateName = getTapStateName(X);
  TEST_ASSERT_EQUAL_STRING("Invalid TAP state", tapStateName);

}
/*
void test_getTmsRequired_given_TEST_LOGIC_RESET_to_RUN_TEST_IDLE_expect_tms_LOW(void){
  int expectedTms;

  expectedTms = getTmsRequired(TEST_LOGIC_RESET, RUN_TEST_IDLE);

  TEST_ASSERT_EQUAL(0, expectedTms);

}

void test_getTmsRequired_given_TEST_LOGIC_RESET_to_TEST_LOGIC_RESET_expect_tms_HIGH(void){
  int expectedTms;

  expectedTms = getTmsRequired(TEST_LOGIC_RESET, TEST_LOGIC_RESET);

  TEST_ASSERT_EQUAL(1, expectedTms);

}

void test_jtagGoTo_from_TEST_LOGIC_RESET_to_RUN_TEST_IDLE_expect_correct(void){

  // Initialize current state of TAP and endState
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  TapSequence tapSeq[] = {
  {TEST_LOGIC_RESET, 0, 0, 1},
  {RUN_TEST_IDLE, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, RUN_TEST_IDLE);

  TEST_ASSERT_EQUAL(RUN_TEST_IDLE, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_TEST_LOGIC_RESET_to_SHIFT_IR_expect_correct(void){
  // Initialize current state of TAP and endState
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  //Tms
  // 0-1-1-0-0
  TapSequence tapSeq[] = {
  {TEST_LOGIC_RESET, 0, 0, 1},
  {RUN_TEST_IDLE, 0, 0, 1},
  {SELECT_DR_SCAN, 0, 0, 1},
  {SELECT_IR_SCAN, 0, 0, 1},
  {CAPTURE_IR, 0, 0, 1},
  {SHIFT_IR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, SHIFT_IR);

  TEST_ASSERT_EQUAL(SHIFT_IR, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_EXIT2_DR_to_SHIFT_DR_expect_correct(void){
  jtagState.state = EXIT2_DR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {EXIT2_DR, 0, 0, 1},
  {SHIFT_DR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, SHIFT_DR);

  TEST_ASSERT_EQUAL(SHIFT_DR, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_CAPTURE_DR_to_UPDATE_DR_expect_correct(void){
  jtagState.state = CAPTURE_DR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {CAPTURE_DR, 0, 0, 1},
  {EXIT1_DR, 0, 0, 1},
  {UPDATE_DR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, UPDATE_DR);

  TEST_ASSERT_EQUAL(UPDATE_DR, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_EXIT2_IR_to_SHIFT_IR_expect_correct(void){
  jtagState.state = EXIT2_IR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {EXIT2_IR, 0, 0, 1},
  {SHIFT_IR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, SHIFT_IR);

  TEST_ASSERT_EQUAL(SHIFT_IR, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_CAPTURE_IR_to_UPDARE_IR_expect_correct(void){
  jtagState.state = CAPTURE_IR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {CAPTURE_IR, 0, 0, 1},
  {EXIT1_IR, 0, 0, 1},
  {UPDATE_IR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, UPDATE_IR);

  TEST_ASSERT_EQUAL(UPDATE_IR, jtagState.state);
  verifyTapSequence(tapSeq);
}

void test_jtagGoTo_given_TEST_LOGIC_RESET_to_SHIFT_IR_wrong_seq_expect_error_message(void){
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {TEST_LOGIC_RESET, 0, 0, 1},
  {RUN_TEST_IDLE, 0, 0, 1},
  {SELECT_DR_SCAN, 0, 0, 1},
  {CAPTURE_DR, 0, 0, 1}, // expect SELECT_IR_SCAN
  {CAPTURE_IR, 0, 0, 1},
  {SHIFT_IR, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagGoTo(jtagState, SHIFT_IR);

  TEST_ASSERT_EQUAL(SHIFT_IR, jtagState.state);
  verifyTapSequence(tapSeq);
}



void test_jtagGoTo_given_TEST_LOGIC_RESET_to_TEST_LOGIC_RESET_wrong_seq_expect_error_thrown(void){

  CEXCEPTION_T e;
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_TAP_TRAVEL;

  // tms
  // 0-1-0-..
  TapSequence tapSeq[] = {
  {TEST_LOGIC_RESET, 0, 0, 1},
  {TEST_LOGIC_RESET, 0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);

  Try{
    jtagGoTo(jtagState, TEST_LOGIC_RESET);
    TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, jtagState.state);
  }
  Catch(e){
    TEST_FAIL_MESSAGE(e->errorMsg);
    TEST_ASSERT_EQUAL(ERR_SAME_TAP_STATE, e->errorCode);
  }
  verifyTapSequence(tapSeq);
}


void test_tapWriteBits_given_0xf1_expect_correct(void){
  jtagState.state = SHIFT_DR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_WRITE_BITS;

  TapSequence tapSeq[] = {
  {SHIFT_DR, 0, 0, 1},    //First cycle data does not inputted
  {SHIFT_DR, 0b1110001, 0, 7}, // tdi = 1111 0001
  {EXIT1_DR, 0b1, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  jtagWriteAndReadBits(0xf1, 8);

  TEST_ASSERT_EQUAL(0xf1, jtagState.DataReg);
  verifyTapSequence(tapSeq);
}

/*
void test_tapWriteBits_given_0x60_in_seq_but_0x61_in_function_expect_error(void){
  jtagState.state = SHIFT_IR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_WRITE_BITS;

  TapSequence tapSeq[] = {
  {SHIFT_IR, 0, 0, 1},    //First cycle data does not inputted
  {SHIFT_IR, 0b110000, 0, 7}, // tdi = 0110 0000
  {EXIT1_IR, 0b0, 0, 1},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  tapWriteBits(0x61, 8);

  TEST_ASSERT_EQUAL(0x60, jtagState.DataReg);
  verifyTapSequence(tapSeq);
}


void test_tapReadBits_given_0xff_expect_read_0xff(void){
  uint64_t data = 0;
  jtagState.state = SHIFT_DR;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_READ_BITS;

  TapSequence tapSeq[] = {
  {SHIFT_DR, 0, 0xff, 8}, // tdi = 1111 1111
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  data = tapReadBits(8);
  TEST_ASSERT_EQUAL(255, data);
  verifyTapSequence(tapSeq);
}


void test_tapReadBits_given_wrong_state_expect_error_message(void){
  uint64_t data = 0;
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_READ_BITS;

  TapSequence tapSeq[] = {
  {SHIFT_DR, 0, 0xff, 8}, // tdi = 1111 1111
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  data = tapReadBits(8);
  TEST_ASSERT_EQUAL(255, data);
  verifyTapSequence(tapSeq);
}


void test_loadInstructionRegister_given_BYPASS_expect_instruction_written(void){
  jtagState.state = TEST_LOGIC_RESET;
  jtagState.DataReg = 0;
  jtagState.operation = JTAG_WRITE_BITS;

  TapSequence tapSeq[] = {
  {TEST_LOGIC_RESET, 0, 0, 1},    //First cycle data does not inputted
  {RUN_TEST_IDLE, 0, 0, 1}, // tdi = 0110 0000
  {SELECT_DR_SCAN, 0, 0, 1},
  {SELECT_IR_SCAN, 0, 0, 1},
  {CAPTURE_IR, 0, 0, 1},
  {SHIFT_IR, 0, 0, 0},
  {SHIFT_IR, 0b1111, 0, 0},
  {SHIFT_IR, 0b1, 0, 0},
  {END, 0, 0, 1}};

  setupFakeTapSeq(tapSeq);
  loadInstructionRegister(BYPASS ,5);

  TEST_ASSERT_EQUAL(0b11111, jtagState.DataReg);
  verifyTapSequence(tapSeq);
}
*/
