#include <unity.h>
#include <stdio.h>
#include "myString.h"


void setUp(void){}
void tearDown(void){}

/*
void test_skipWhiteSpaces_given_one_blankspace_abc_expect_abd(void){
  char *str = " abc";
  skipWhiteSpaces(&str);
  TEST_ASSERT_EQUAL_STRING ("abc", str);
}

void test_skipWhiteSpaces_given_multiple_blankspaces_1234cd_expect_1234cd(void){
  char *str = "                 1234cd";
  skipWhiteSpaces(&str);
  TEST_ASSERT_EQUAL_STRING ("1234cd", str);
}*/

void test_bypassCharactersInStr_given_zzzzasdq_bypass_5_character_expect_sdq(void){
  char *str = "zzzzasdq";
  bypassCharactersInStr(&str, 5);
  TEST_ASSERT_EQUAL_STRING("sdq", str);
}

void test_bypassCharactersInStr_given_10_blank_spaces_adaZZZ111_bypass_15_character_expect_Z111(void){
  char *str = "          adaZZZ111";
  bypassCharactersInStr(&str, 15);
  TEST_ASSERT_EQUAL_STRING("Z111", str);
}

void test_bypassCharactersInStr_given_10_blank_spaces_adaZZZ111_bypass_22_character_expect_error(void){
  char *str = "          adaZZZ111";
  bypassCharactersInStr(&str, 22);
  TEST_ASSERT_EQUAL_STRING("Z111", str);
}
/*
void test_getNumberTypeInStr_given_0xface_expect_HEXADECIMAL(void){
  char *str = "0xface";
  TEST_ASSERT_EQUAL(HEXADECIMAL, getNumberTypeInStr(&str));
}

void test_getNumberTypeInStr_given_0b1111_expect_BINARY(void){
  char *str = "0b1111";
  TEST_ASSERT_EQUAL(BINARY, getNumberTypeInStr(&str));
}

void test_getNumberTypeInStr_given_1234_expect_BINARY(void){
  char *str = "1234";
  TEST_ASSERT_EQUAL(DECIMAL, getNumberTypeInStr(&str));
}

void test_getNumberTypeInStr_given_abcd_expect_INVALID(void){
  char *str = "cvcvcv";
  TEST_ASSERT_EQUAL(INVALID, getNumberTypeInStr(&str));
}

void test_decimalToBinaryInStr_given_87_expect_01010111(void){
  TEST_ASSERT_EQUAL_STRING("01010111", decimalToBinaryInStr(87, 8));
}

// 0b10101111(str) = 175(decimal)
void test_getNumberFromStr_given_0b10101111_expect_175(void){
  char *str = "0b10101111";
  TEST_ASSERT_EQUAL(175, getNumberFromStr(&str));
}

// 0xffff(str) = 65535(decimal)
void test_getNumberFromStr_given_0xffff_expect_65535(void){
  char *str = "0xffff";
  TEST_ASSERT_EQUAL(65535, getNumberFromStr(&str));
}
// 123456789(str) = 123456789(decimal)
void test_getNumberFromStr_given_123456789_expect_123456789(void){
  char *str = "123456789";
  TEST_ASSERT_EQUAL(123456789, getNumberFromStr(&str));
}

void test_getDataLengthFromStr_given_10_expect_10(void){
  char *str = "  10";
  TEST_ASSERT_EQUAL(10, getDataLengthFromStr(&str));
}*/
