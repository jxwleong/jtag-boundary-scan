/*
 * myString.h
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

#ifndef MYSTRING_H_
#define MYSTRING_H_

typedef enum{
  DECIMAL = 10,
  BINARY = 2,
  HEXADECIMAL = 16,
  INVALID = 20,
}NumberTypeBase;

void skipWhiteSpaces(char **str);
void bypassCharactersInStr(char **str, int length);

NumberTypeBase getNumberTypeInStr(char **str);
char *decimalToBinaryInStr(int data, int length);

long long getNumberFromStr(char **str);
int getDataLengthFromStr(char **str);

#endif /* MYSTRING_H_ */
