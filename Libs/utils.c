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
bool lastWasCreateFrame = false;

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

  bool iscreateframe = strcmp(arg, "CREATEFRAME\n") == 0;
  if (!iscreateframe || !lastWasCreateFrame)
  {
    va_list ap;
    va_start(ap, arg);
    arrPos += vsprintf(&(Iarr[arrPos]), arg, ap);
    va_end(ap);
  }
  lastWasCreateFrame = iscreateframe;
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
  if (symb > 57)
    return "non-teminal";

  char *tokenTypeStrings[] = {
    "opPlus",       //  0
		"opMns",        //  1
		"opMul",        //  2
		"opDivFlt",     //  3
		"opDiv",        //  4
		"opLeftBrc",    //  5
		"opRightBrc",   //  6
		"ident",        //  7
		"opComma",      //  8
    "opEq",         //  9
		"opNotEq",      // 10
		"opLes",        // 11
		"opLessEq",     // 12
		"opGrt",        // 13
		"opGrtEq",      // 14
    "asgn",         // 15
		"opPlusEq",     // 16
		"opMnsEq",      // 17
		"opMulEq",      // 18
		"opDivEq",      // 19
		"opDivFltEq",   // 20
    "opBoolNot",    // 21
		"opBoolAnd",    // 22
		"opBoolOr",     // 23
		"eol",          // 24
    "opSemcol",     // 25
    "dataType",     // 26
    "eof",          // 27
    "kwAs",         // 28
		"kwDeclare",    // 30
		"kwDim",        // 31
		"kwDo",         // 32
		"kwElse",       // 33
		"kwEnd",        // 34
		"kwFunction",   // 35
		"kwIf",         // 36
		"kwInput",      // 37
		"kwLoop",       // 39
    "kwPrint",      // 40
		"kwReturn",     // 41
		"kwScope",      // 42
		"kwThen",       // 44
		"kwWhile",      // 45
		"kwContinue",   // 46
		"kwElseif",     // 47
		"kwExit",       // 48
		"kwFalse",      // 49
		"kwFor",        // 50
    "kwNext",       // 51
		"kwShared",     // 52
		"kwStatic",     // 53
		"kwTrue",       // 54
		"kwTo",         // 55
    "kwUntil",      // 56
    "kwStep"        // 57
  };
  return tokenTypeStrings[symb];
}

// Fuction prints build-in functions
void util_printBuildFunc()
{
  printf("LABEL $$Length\nPUSHFRAME\nDEFVAR LF@%%retval\nMOVE LF@%%retval int@0\nSTRLEN LF@%%retval LF@p1\nPOPFRAME\nRETURN\n");

  printf("LABEL $$SubStr\nPUSHFRAME\nDEFVAR LF@%%retval\nDEFVAR LF@len\nDEFVAR LF@help\nSTRLEN LF@len LF@p1\nMOVE LF@%%retval string@\n");
  printf("JUMPIFEQ $$EndSubStr LF@%%retval LF@p1\nGT LF@help int@1 LF@p2\nJUMPIFEQ $$EndSubStr LF@help bool@true\nGT LF@help int@0 LF@p3\n");
  printf("JUMPIFEQ $$SubStrExtra1 LF@help bool@true\nSUB LF@help LF@len LF@p2\nGT LF@help LF@p3 LF@help\nJUMPIFEQ $$SubStrExtra2 LF@help bool@true\n");
  printf("JUMP $$SubStrStart\nLABEL $$SubStrExtra1\nMOVE LF@p2 LF@len\nJUMP $$SubStrStart\nLABEL $$SubStrExtra2\nMOVE LF@p3 LF@len\n");
  printf("SUB LF@p3 LF@p3 LF@p2\nLABEL $$SubStrStart\nADD LF@p3 LF@p2 LF@p3\nLABEL $$CycleSubStr\nGETCHAR LF@help LF@p1 LF@p2\n");
  printf("CONCAT LF@%%retval LF@%%retval LF@help\nADD LF@p2 LF@p2 int@1\nJUMPIFNEQ $$CycleSubStr LF@p2 LF@p3\nLABEL $$EndSubStr\nPOPFRAME\nRETURN\n");

  printf("LABEL $$Asc\nPUSHFRAME\nDEFVAR LF@%%retval\nDEFVAR LF@help\nMOVE LF@%%retval int@0\nSTRLEN LF@help LF@p1\nGT LF@help LF@help LF@p2\n");
  printf("JUMPIFEQ $$EndAsc LF@help bool@false\nGETCHAR LF@%%retval LF@p1 LF@p2\nSTRI2INT LF@%%retval LF@%%retval int@0\nLABEL $$EndAsc\nPOPFRAME\n");
  printf("RETURN\n");

  printf("LABEL $$Chr\nPUSHFRAME\nDEFVAR LF@%%retval\nMOVE LF@%%retval string@\nINT2CHAR LF@%%retval LF@p1\nPOPFRAME\nRETURN\n");
}

// true if string is buid-in function
bool util_isBuildInFunc(char *str)
{
  char * buildInFunc[] = {"length", "substr", "asc", "chr"};
  for (int i = 0; i < 4; i++)
    if (strcmp(str, buildInFunc[i]) == 0)
      return true;
  return false;
}
