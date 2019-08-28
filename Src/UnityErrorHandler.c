#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include "unity.h"
#include "UnityErrorHandler.h"

void unityError(char *message, ...){
  va_list args;
  char *buffer;
  int length;

  va_start(args, message);

  length = vsnprintf(buffer, 0, message, args);
  buffer = malloc(length + 1);
  vsnprintf(buffer, length + 1, message, args);

  TEST_FAIL_MESSAGE(buffer);
}


