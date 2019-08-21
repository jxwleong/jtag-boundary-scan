/*
 * myString.c
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#include <malloc.h>
#include "myString.h"

void skipWhiteSpaces(char **str){
    while(**str == ' ')
        ++*str;
}

void bypassCharactersInStr(char **str, int length){
	while(length != 0){
		++*str;
		length--;
	}
}


// If  BINARY
//  0b xxxxx
//
// HEXADECIMAL
//  0x xxxxx
//
//  DECIMAL
//  xxxxx
// Assume the number is in correct format
NumberTypeBase getNumberTypeInStr(char **str){
++*str; // get to second char

if(**str == 'B' || **str == 'b'){
    ++*str; // remove the character 'b'
    return BINARY;
  }
else if(**str == 'x' || **str == 'X'){
    ++*str; // remove the character 'b'
    return HEXADECIMAL;
  }
else if(isdigit(**str)){
    --*str; // move the pointer to start
    return DECIMAL;
  }
else{
  printf("Invalid number is received");
  return INVALID;
  }
}

char *decimalToBinaryInStr(int data, int length){
int i = 0;
int updateData = 0;
int count = 0;
char *ansStr = (char*)malloc(length+1);// +1 for '\0'


// lengh-1, else extra msb bit
for (i = length-1 ; i >= 0 ; i--){
  updateData = data >> i;

  if (updateData & 1)
     *(ansStr+count) = '1';
  else
     *(ansStr+count) = '0';

  count++;
}

*(ansStr+count) = '\0';

return  ansStr;
}

long long getNumberFromStr(char **str){
	long long ans;
	skipWhiteSpaces(&(*str));
	NumberTypeBase base = getNumberTypeInStr(&(*str));
	ans = strtoll(*str, &(*str), base);
	return ans;
}

int getDataLengthFromStr(char **str){
	int ans = 0;
	skipWhiteSpaces(&(*str));
	NumberTypeBase base = getNumberTypeInStr(&(*str));
	ans = strtoll(*str, &(*str), base);
	return ans;
}
