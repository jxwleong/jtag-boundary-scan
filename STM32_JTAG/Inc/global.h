#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "TAP.h"

#define BYPASS    0b11111
#define EXTEST    0b00000
#define SAMPLE    0b00010
#define PRELOAD   0b00010
#define IDCODE    0b0000101110

extern JtagState jtagState;

#endif // _GLOBAL_H
