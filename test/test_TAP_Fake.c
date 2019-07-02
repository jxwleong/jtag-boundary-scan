#include <unity.h>      /**<unity.h> must place befre mock header*/
#include <stdio.h>
#include "cmock.h"
#include "TAP.h"
#include "TAP_LookUpTable.h"
#include "mock_TAP_Mock.h"
#include "UnityErrorHandler.h"

void setUp(void){}
void tearDown(void){}

/*
int expTapSequenceCallNumbers = 0;
int getTapStatesCallNumbers = 0;
int getTdoCallNumbers = 0;
int setTmsCallNumbers = 0;
int setTckCallNumbers = 0;
int setTdiCallNumbers = 0;

int expectedTapSequenceMaxCalls = 0;
int expectedGetTdoMaxCalls = 0;
int expectedGetTapStatesMaxCalls = 0;
int expectedSetTmsMaxCalls = 0;
int expectedSetTckMaxCalls = 0;
int expectedSetTdiMaxCalls = 0;

TapSequence *expectedTapSequence = NULL;
TdiState *expectedTdo = NULL;
TapState *expectedTapStates = NULL;
TmsState *expectedTmsStates = NULL;
TckState *expectedTck = NULL;
TdiState *expectedTdi = NULL;
*/
/*
void fake_verifyTapSequence(TapSequence sequence, int NumCalls){
  TapSequence expectedSequence;
  expTapSequenceCallNumbers ++;
  if(NumCalls < expectedTapSequenceMaxCalls){
    expectedSequence = expectedTapSequence[NumCalls];
    NumCalls++;
  }

  if(sequence.state != ((expectedTapSequence[NumCalls - 1]).state)){
    unityError("expected state on %i entry is %s but detect %s",\
    NumCalls, getTapStateName(expectedTapSequence[NumCalls - 1].state),
    getTapStateName(sequence.state));
  }
  if(sequence.tdi != ((expectedTapSequence[NumCalls - 1]).tdi)){
    unityError("expected tdi on %i entry is %i but detect %i",\
    NumCalls, sequence.tdi, expectedTapSequence[NumCalls - 1].tdi);
  }
  if(sequence.tdo != ((expectedTapSequence[NumCalls - 1]).tdo)){
    unityError("expected tdo on %i entry is %i but detect %i",\
    NumCalls, sequence.tdo, expectedTapSequence[NumCalls - 1].tdo);
  }
  if(sequence.bits != ((expectedTapSequence[NumCalls - 1]).bits)){
    unityError("expected bits on %i entry is %i but detect %i",\
    NumCalls, sequence.bits, expectedTapSequence[NumCalls - 1].bits);
  }

}

void fake_setTms(TmsState state,int NumCalls){
  char *msg;
  TmsState expectedState;
  setTmsCallNumbers++;
  if(NumCalls < expectedSetTmsMaxCalls){
   expectedState = expectedTmsStates[NumCalls];
   NumCalls++;
  }

 if(state != expectedTmsStates[NumCalls-1]){
  unityError("setTms() was called with %s, but expect %s on %i cycle",  getTckStateName(state), getTckStateName(expectedState), NumCalls);
  }
}


void fake_setTck(TckState state,int NumCalls){
  char *msg;
  TmsState expectedState;
  setTckCallNumbers++;
  if(NumCalls < expectedSetTckMaxCalls){
     expectedState = expectedTck[NumCalls];
     NumCalls++;
   }
  if(state != expectedTck[NumCalls-1]){
    unityError("setTck() was called with %s, but expect %s on %i cycle",  getTckStateName(state), getTckStateName(expectedState), NumCalls);
  }
}

void fake_setTdi(TdiState state,int NumCalls){
  char *msg;
  TdiState expectedState;
  setTdiCallNumbers++;
  if(NumCalls < expectedSetTdiMaxCalls){
     expectedState = expectedTdi[NumCalls];
     NumCalls++;
   }
  if(state != expectedTdi[NumCalls-1]){
    unityError("setTdi() was called with %s, but expect %s on %i cycle",  getTdiStateName(state), getTdiStateName(expectedState), NumCalls);
  }
}

TapState fake_getTapStates(int NumCalls){
  if(NumCalls < expectedGetTapStatesMaxCalls){
    return expectedTapStates[NumCalls];
  }
  else
  TEST_FAIL_MESSAGE("Received extra getTapStates() calls.");
}

TapState fake_getTdo(int NumCalls){
  if(NumCalls < expectedGetTdoMaxCalls){
    return expectedTdo[NumCalls];
  }
  else
  TEST_FAIL_MESSAGE("Received extra getTdo() calls.");
}
/*
void setupFake(TdoState expTdo[], int tdoMaxCalls, TdiState expTdi[], int tdiMaxCalls, TmsState expTmsStates[], int TmsMaxCalls, TckState expTck[], int TckMaxCalls){
   getTapStatesCallNumbers = 0;
   setTmsCallNumbers = 0;
   setTckCallNumbers = 0;
   setTdiCallNumbers = 0;

   getTdo_StubWithCallback(fake_getTdo);
   expectedTdo = expTdo;
   expectedGetTdoMaxCalls = tdoMaxCalls;

   setTdi_StubWithCallback(fake_setTdi);
   expectedTdi = expTdi;
   expectedSetTdiMaxCalls = tdiMaxCalls;

   setTms_StubWithCallback(fake_setTms);
   expectedTmsStates = expTmsStates;
   expectedSetTmsMaxCalls = TmsMaxCalls;

   setTck_StubWithCallback(fake_setTck);
   expectedTck = expTck;
   expectedSetTckMaxCalls = TckMaxCalls;
}
*/
/*
void setupFake(TapSequence expSeq[], int seqMaxCall){
  expTapSequenceCallNumbers = 0;

  verifyTapSequence_StubWithCallback(fake_verifyTapSequence);
  expectedTapSequence = expSeq;
  expectedTapSequenceMaxCalls = seqMaxCall;

}
*/


/*
void test_tapTestLogicReset_given_TEST_LOGIC_RESET_expect_TEST_LOGIC_RESET(void){
  TapStateSequence stateSequence = {TEST_LOGIC_RESET, 1};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TmsState  expectedTmsStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, LOW};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 5, expectedTckStates, 5);
  tapTestLogicReset(&stateSequence);
  TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, stateSequence.currentState);

}

void test_tapTestLogicReset_given_SHIFT_DR_expect_TEST_LOGIC_RESET(void){
  TapStateSequence stateSequence = {SHIFT_DR, 0};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TmsState  expectedTmsStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, LOW};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 5, expectedTckStates, 5);
  tapTestLogicReset(&stateSequence);
  TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, stateSequence.currentState);

}

void test_tapTestLogicReset_given_EXIT1_IR_Set_TMS_LOW_on_third_cycleexpect_TEST_LOGIC_RESET_and_error_message(void){
  TapStateSequence stateSequence = {EXIT1_IR, 0};

  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TmsState  expectedTmsStates[] = {HIGH, HIGH, LOW, HIGH, HIGH};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, LOW};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 5, expectedTckStates, 5);
  tapTestLogicReset(&stateSequence);
  TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, stateSequence.currentState);

}


void test_tapTestLogicReset_given_PAUSE_DR_Set_TCK_HIGH_on_fifth_cycle_expect_TEST_LOGIC_RESET_and_error_message(void){
  TapStateSequence stateSequence = {PAUSE_DR, 0};

  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TmsState  expectedTmsStates[] = {HIGH, HIGH, HIGH, HIGH, HIGH};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, HIGH};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 5, expectedTckStates, 5);
  tapTestLogicReset(&stateSequence);
  TEST_ASSERT_EQUAL(TEST_LOGIC_RESET, stateSequence.currentState);

}

void test_tapGoToFrom_given_TEST_LOGIC_RESET_to_TEST_LOGIC_RESET_expect_error_messasge(void){
  TapStateSequence stateSequence = {TEST_LOGIC_RESET, 0};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TckState  expectedTckStates[] = {LOW};
  TmsState  expectedTmsStates[] = {LOW};
  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 1, expectedTckStates, 1);
  tapGoToFrom(TEST_LOGIC_RESET, TEST_LOGIC_RESET, &stateSequence);
  TEST_ASSERT_EQUAL(RUN_TEST_IDLE, stateSequence.currentState);
}

void test_tapGoToFrom_given_TEST_LOGIC_RESET_to_RUN_TEST_IDLE_expect_currentState_RUN_IDLE_TEST(void){
  TapStateSequence stateSequence = {TEST_LOGIC_RESET, 0};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TckState  expectedTckStates[] = {LOW};
  TmsState  expectedTmsStates[] = {LOW};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 1, expectedTckStates, 1);
  tapGoToFrom(TEST_LOGIC_RESET, RUN_TEST_IDLE, &stateSequence);
  TEST_ASSERT_EQUAL(RUN_TEST_IDLE, stateSequence.currentState);
}

void test_tapGoToFrom_given_TEST_LOGIC_RESET_to_UPDATE_DR_expect_currentState_UPDATE_DR(void){
  TapStateSequence stateSequence = {TEST_LOGIC_RESET, 0};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, LOW};
  TmsState  expectedTmsStates[] = {LOW, HIGH, LOW, HIGH, HIGH};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 5, expectedTckStates, 5);
  tapGoToFrom(TEST_LOGIC_RESET, UPDATE_DR, &stateSequence);
  TEST_ASSERT_EQUAL(UPDATE_DR, stateSequence.currentState);
}

void test_tapGoToFrom_given_SHIFT_DR_to_UPDATE_IR_expect_currentState_UPDATE_IR(void){
  TapStateSequence stateSequence = {SHIFT_DR, 0};
  TdoState  expectedTdo[] = {X_CARE};
  TdiState  expectedTdi[] = {X_CARE};
  TckState  expectedTckStates[] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW};
  TmsState  expectedTmsStates[] = {HIGH, HIGH, HIGH, HIGH, LOW, HIGH, HIGH};

  setupFake(expectedTdo, 0, expectedTdi, 0, expectedTmsStates, 7, expectedTckStates, 7);
  tapGoToFrom(SHIFT_DR, UPDATE_IR, &stateSequence);
  TEST_ASSERT_EQUAL(UPDATE_IR, stateSequence.currentState);
}
*/
