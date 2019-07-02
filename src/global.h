#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <stdio.h>
#include "TAP.h"

typedef struct JtagState JtagState;
struct JtagState{
  TapState state;
};

extern JtagState jtagState;

#endif // _GLOBAL_H
