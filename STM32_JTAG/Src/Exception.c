#include "Exception.h"
#include "CExceptionConfig.h"
#include <stdarg.h>
#include <malloc.h>
#include <stdio.h>

void throwError(int errorCode,char *format,...){
  va_list valist;
  int neededSize = 0;
  char *buffer;          // store data

  Exception *ex = (Exception*)malloc(sizeof(Exception));
  ex->errorCode = errorCode;

  va_start(valist,format);
  neededSize = vsnprintf(buffer, 0, format, valist) + 1; // since 0 it will return bytes required due to ISO99_source
  buffer = malloc(neededSize);
  vsnprintf(buffer, neededSize, format, valist);
  va_end(valist);
  ex->errorMsg = buffer;
//  printf(buffer);
  Throw(ex);
}

void freeError(Exception *ex){
  if(ex) {
    if(ex->errorMsg)
    free(ex->errorMsg);
    free(ex);
  }
}
