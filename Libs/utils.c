
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

#include <string.h>
#include "MMng.h"
#include "utils.h"

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
    char *str = mmng_safeMalloc(sizeof(char) * (strlen(str1) + strlen(str2)));
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
    case dtInt:      return "Integer";
    case dtFloat:    return "Float";
    case dtString:   return "String";
    case dtBool:     return "Boolean";
    default:         return "Unspecified";
  }
}

// Fuction prints build-in functions
void util_PrintBuildFunc()
{
  printf("LABEL $$Length
  PUSHFRAME 
  DEFVAR LF@%retval
  MOVE LF@%retval int@0
  STRLEN LF@%retval LF@p1 
  POPFRAME
  RETURN");
  
  printf("LABEL $$SubStr
  PUSHFRAME
  DEFVAR LF@%retval
  MOVE LF@%retval string@
  DEFVAR LF@len
  DEFVAR LF@help
  STRLEN LF@len LF@p1
  ADD LF@p3 LF@p2 LF@p3
LABEL $$CycleSubStr 
  GETCHAR LF@help LF@p1 LF@p2
  CONCAT LF@%retval LF@%retval LF@help
  ADD LF@p2 LF@p2 int@1
  JUMPIFNEQ $$CycleSubStr LF@p2 LF@p3
LABEL $$EndSubStr 
  POPFRAME
  RETURN");
  
  printf("LABEL $$Asc
  PUSHFRAME 
  DEFVAR LF@%retval
  DEFVAR LF@help
  MOVE LF@%retval int@0
  STRLEN LF@help LF@p1
  GT LF@help LF@help LF@p2
JUMPIFEQ $$EndAsc LF@help bool@false 
  GETCHAR LF@%retval LF@p1 LF@p2
  STRI2INT LF@%retval LF@%retval int@0
LABEL $$EndAsc  
  POPFRAME
  RETURN");
  
  printf("LABEL $$Chr
  PUSHFRAME 
  DEFVAR LF@%retval
  MOVE LF@%retval string@
  INT2CHAR LF@%retval LF@p1
  POPFRAME
  RETURN");
}
