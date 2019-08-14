/*
 * myString.c
 *
 *  Created on: Aug 14, 2019
 *      Author: user
 */

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

