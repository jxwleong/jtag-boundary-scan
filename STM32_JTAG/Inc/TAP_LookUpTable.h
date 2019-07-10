#ifndef _TAP_LOOKUPTABLE_H
#define _TAP_LOOKUPTABLE_H

#include "TAP.h"
#include "TAP_Mock.h"


typedef struct StateTable StateTable;
struct StateTable{
  int tms;
  TapState nextState;
};


void jtagGoTo(JtagState start, TapState end);
int getTmsRequired(TapState currentState, TapState endState);
TapState getTapState(TapState currentState, int tms);

#endif // _TAP_LOOKUPTABLE_H
