#include "TAP_LookUpTable.h"
#include "global.h"
#include "Exception.h"
//#include "CException.h"
#include "CExceptionConfig.h"

StateTable stateMachineLookUpTable[16][16] = {
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

TapTestTable tapTestTable[16] = {{TEST_LOGIC_RESET, RUN_TEST_IDLE, TEST_LOGIC_RESET},
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
/*
@param The start of the TAP state, the destination of TAp states

@desc  Traverse in the TAP state machine from start to end by refering
       to a lookup table..

*/
void jtagGoTo(JtagState start, TapState end){
    TapState currentState = start.state;
    int tmsRequired;
    int i = 0;

      if(currentState == end)
          throwError(1,"Start state (%s) is same as end state(%s)", \
          getTapStateName(currentState), getTapStateName(end));

      while(currentState != end){
          tmsRequired = getTmsRequired(currentState, end);
          tapStep(tmsRequired, 0);// int tapStep(tms, tdi)
          jtagState.state = getTapState(currentState, tmsRequired);
          currentState = stateMachineLookUpTable[currentState][end].nextState;
          i++;
          }
}

/*  @param current TAP state, end TAP state
    @desc  get the required TMS to traverse in TAP state machine
           by one cycle
*/
int getTmsRequired(TapState currentState, TapState endState){
      return (stateMachineLookUpTable[currentState][endState]).tms;
}

/*  @param  TapState and tms
    @retval updated TAP state
*/
TapState getTapState(TapState currentState, int tms){
      if(tms == 1)
        return (tapTestTable[currentState].nextState_tms1);
      else if(tms == 0)
        return (tapTestTable[currentState].nextState_tms0);
}
