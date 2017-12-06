/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.h
 * \brief   Liblary providing supportive function and types for the project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    5.12.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _utils
#define _utils

#include <stdbool.h>
#include "grammar.h"


/**
 * Global data type enum
 */
typedef enum {
  dtUnspecified,
  dtInt,
  dtFloat,
  dtString,
  dtBool
} DataType;


void printInstruction(const char *, ...);

void printLongInstruction(unsigned len, const char *arg, ...);

void flushCode();

/**
 * Fuction create hard copy of given string
 * \note Memory is allocated here and free has to be called.
 */
char *util_StrHardCopy(const char *str);

/**
 * Fuction returns concatenated str1 and str2 (str = str1 + str2) Inputed strings are just readed and new string is created
 * \note Memory is allocated here and free has to be called.
 */
char *util_StrConcatenate(const char *str1, const char *str2);

/**
 * Fuction converts data type enum to constant string
 */
const char *util_dataTypeToString(const DataType dataType);

/**
 * Return grammar symbol as tring
 */
char *grammarToString(EGrSymb symb);

/**
 * Fuction prints build-in functions
 * \note These funcions are: Length, SubStr, Asc, Chr
 */
void util_printBuildFunc();

/**
 * True if string is buid-in function
 */
bool util_isBuildInFunc(char *str);


#endif // _utils
