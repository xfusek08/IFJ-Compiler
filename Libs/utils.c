/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.c
 * \brief   Liblary providing supportive function for project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    6.12.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "mmng.h"
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
  if (arg == NULL)
    return;

  unsigned arglen = strlen(arg);

  if (arglen == 0)
    return;

  int counter = 0;
  for (unsigned i = 0; i < arglen; i++)
  {
    if(arg[i] == '%')
      counter++;
  }
  while (counter * 128 + arglen + arrPos + 1 > arrSize)
  {
    arrSize += UTILS_ARR_CHUNK;
    Iarr = mmng_safeRealloc(Iarr, sizeof(char) * arrSize);
  }
  bool iscreateframe = strcmp(arg, "CREATEFRAME\n") == 0;
  if (!iscreateframe || !lastWasCreateFrame)
  {
    va_list ap;
    va_start(ap, arg);
    // vfprintf(stderr, arg, ap);
    // va_end(ap);
    // va_start(ap, arg);
    arrPos += vsprintf(&(Iarr[arrPos]), arg, ap);
    va_end(ap);
  }
  lastWasCreateFrame = iscreateframe;
}

void printLongInstruction(unsigned len, const char *arg, ...)
{
  if (arrSize == 0)
  {
    arrSize = UTILS_ARR_CHUNK;
    Iarr = mmng_safeMalloc(sizeof(char) * arrSize);
  }
  if (arg == NULL)
    return;

  unsigned arglen = strlen(arg);

  if (arglen == 0)
    return;
  while (len + arglen + arrPos + 1 > arrSize)
  {
    arrSize += UTILS_ARR_CHUNK;
    Iarr = mmng_safeRealloc(Iarr, sizeof(char) * arrSize);
  }

  va_list ap;
  va_start(ap, arg);
  // vfprintf(stderr, arg, ap);
  // va_end(ap);
  // va_start(ap, arg);
  arrPos += vsprintf(&(Iarr[arrPos]), arg, ap);
  va_end(ap);
}

void flushCode()
{
  if (Iarr != NULL)
  {
    printf("%s", Iarr);
    arrPos = 0;
  }
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
    "+",            // opPlus      0
		"-",            // opMns       1
		"*",            // opMul       2
		"/",            // opDivFlt    3
		"\\",           // opDiv       4
		"(",            // opLeftBrc   5
		")",            // opRightBrc  6
		"identifier",   // ident       7
		",",            // opComma     8
    "=",            // opEq        9
		"<>",           // opNotEq    10
		"<",            // opLes      11
		"<=",           // opLessEq   12
		">",            // opGrt      13
		">=",           // opGrtEq    14
    "=",            // asgn       15
		"+=",           // opPlusEq   16
		"-=",           // opMnsEq    17
		"*=",           // opMulEq    18
		"\\=",          // opDivEq    19
		"/=",           // opDivFltEq 20
    "not",          // opBoolNot  21
		"and",          // opBoolAnd  22
		"or",           // opBoolOr   23
		"eol",          // eol        24
    ";",            // opSemcol   25
    "dataType",     // dataType   26
    "eof",          // eof        27
    "as",           // kwAs       28
		"declare",      // kwDeclare  30
		"dim",          // kwDim      31
		"do",           // kwDo       32
		"else",         // kwElse     33
		"end",          // kwEnd      34
		"function",     // kwFunction 35
		"if",           // kwIf       36
		"input",        // kwInput    37
		"loop",         // kwLoop     39
    "print",        // kwPrint    40
		"return",       // kwReturn   41
		"scope",        // kwScope    42
		"then",         // kwThen     44
		"while",        // kwWhile    45
		"continue",     // kwContinue 46
		"elseif",       // kwElseif   47
		"exit",         // kwExit     48
		"false",        // kwFalse    49
		"for",          // kwFor      50
    "next",         // kwNext     51
    "shared",       // kwShared   52
    "static",       // kwStatic   53
		"true",         // kwTrue     54
		"to",           // kwTo       55
    "until",        // kwUntil    56
    "step"          // kwStep     57
  };
  return tokenTypeStrings[symb];
}

// Fuction prints build-in functions
void util_printBuildFunc()
{
  printf("LABEL $$Length\nPUSHFRAME\nDEFVAR LF@%%retval\nMOVE LF@%%retval int@0\nSTRLEN LF@%%retval LF@p1\nPOPFRAME\nRETURN\n");

  printf("LABEL $$SubStr\nPUSHFRAME\nDEFVAR LF@%%retval\nDEFVAR LF@len\nDEFVAR LF@help\nSTRLEN LF@len LF@p1\nSUB LF@p2 LF@p2 int@1 \nMOVE LF@%%retval string@\n");
  printf("JUMPIFEQ $$EndSubStr LF@%%retval LF@p1\nGT LF@help int@0 LF@p2\nJUMPIFEQ $$EndSubStr LF@help bool@true\nGT LF@help int@0 LF@p3\n");
  printf("JUMPIFEQ $$SubStrExtra1 LF@help bool@true\nSUB LF@help LF@len LF@p2\nGT LF@help LF@p3 LF@help\nJUMPIFEQ $$SubStrExtra2 LF@help bool@true\n");
  printf("JUMP $$SubStrStart\nLABEL $$SubStrExtra1\nSUB LF@p3 LF@len LF@p2\nJUMP $$CycleSubStr\nLABEL $$SubStrExtra2\nMOVE LF@p3 LF@len\n");
  printf("SUB LF@p3 LF@p3 LF@p2\nLABEL $$SubStrStart\nADD LF@p3 LF@p2 LF@p3\nLABEL $$CycleSubStr\nGETCHAR LF@help LF@p1 LF@p2\n");
  printf("CONCAT LF@%%retval LF@%%retval LF@help\nADD LF@p2 LF@p2 int@1\nJUMPIFNEQ $$CycleSubStr LF@p2 LF@p3\nLABEL $$EndSubStr\nPOPFRAME\nRETURN\n");

  printf("LABEL $$Asc\nPUSHFRAME\nDEFVAR LF@%%retval\nSUB LF@p2 LF@p2 int@1 \nDEFVAR LF@help\nMOVE LF@%%retval int@0\nSTRLEN LF@help LF@p1\nGT LF@help LF@help LF@p2\n");
  printf("JUMPIFEQ $$EndAsc LF@help bool@false\nGT LF@help int@0 LF@p2 \nJUMPIFEQ $$EndAsc LF@help bool@true \nGETCHAR LF@%%retval LF@p1 LF@p2\nSTRI2INT LF@%%retval LF@%%retval int@0\n");
  printf("LABEL $$EndAsc\nPOPFRAME\nRETURN\n");

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
