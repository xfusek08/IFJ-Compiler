/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.c
 * \brief   Liblary providing supportive function for project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    14.11.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "MMng.h"
#include "utils.h"
#include <stdarg.h>

#define UTILS_ARR_CHUNK 1000

unsigned arrPos = 0;
char *Iarr;
unsigned arrSize;

void printInstruction(const char *arg, ...)
{
  if (arrSize == 0)
  {
    arrSize = UTILS_ARR_CHUNK;
    Iarr = mmng_safeMalloc(sizeof(char) * arrSize);
  }

  int counter = 0;
  for (unsigned i = 0; i < strlen(arg); i++)
  {
    if(arg[i] == '%')
      counter++;
  }

  if (counter * 128 + strlen(arg) + arrPos > arrSize)
  {
    arrSize += UTILS_ARR_CHUNK;
    Iarr = mmng_safeRealloc(Iarr, sizeof(char) * arrSize);
  }

  va_list ap;
  va_start(ap, arg);
  //vfprintf(stderr, arg, ap);
  arrPos += vsprintf(&(Iarr[arrPos]), arg, ap);
  va_end(ap);
}

void flushCode()
{
  printf("%s", Iarr);
  arrPos = 0;
}

// hard string copy
char *util_StrHardCopy(const char *str)
{
  if (str != NULL)
    return strcpy(mmng_safeMalloc(sizeof(char) * strlen(str) + 1), str);
  return NULL;
}

// Fuction returns concatenated str1 and str2
char *util_StrConcatenate(const char *str1, const char *str2)
{
  if (str1 != NULL && str2 != NULL)
  {
    char *str = mmng_safeMalloc(sizeof(char) * (strlen(str1) + strlen(str2) + 1));
    strcpy(str, str1);
    strcat(str, str2);
    return str;
  }
  return NULL;
}

// Fuction converts data type enum To string
const char *util_dataTypeToString(const DataType dataType)
{
  switch(dataType)
  {
    case dtInt:      return "int";
    case dtFloat:    return "float";
    case dtString:   return "string";
    case dtBool:     return "bool";
    default:         return "Unspecified";
  }
}

// Return grammar symbol as tring
char *grammarToString(EGrSymb symb)
{
  char *tokenTypeStrings[] = {
    "opPlus",
		"opMns",
		"opMul",
		"opDivFlt",
		"opDiv",
		"opLeftBrc",
		"opRightBrc",
		"ident",
		"opComma",
    "opEq",
		"opNotEq",
		"opLes",
		"opLessEq",
		"opGrt",
		"opGrtEq",
    "asgn",
		"opPlusEq",
		"opMnsEq",
		"opMulEq",
		"opDivEq",
		"opDivFltEq",
    "opBoolNot",
		"opBoolAnd",
		"opBoolOr",
		"eol",
    "opSemcol",
    "dataType",
    "eof",
    "kwAs",
		"kwAsc",
		"kwDeclare",
		"kwDim",
		"kwDo",
		"kwElse",
		"kwEnd",
		"kwFunction",
		"kwIf",
		"kwInput",
		"kwLength",
		"kwLoop",
    "kwPrint",
		"kwReturn",
		"kwScope",
		"kwSubStr",
		"kwThen",
		"kwWhile",
		"kwContinue",
		"kwElseif",
		"kwExit",
		"kwFalse",
		"kwFor",
    "kwNext",
		"kwShared",
		"kwStatic",
		"kwTrue",
		"kwTo",
    "kwUntil",
    "kwStep"
  };
  return tokenTypeStrings[symb];
}
