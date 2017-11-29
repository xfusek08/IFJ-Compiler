
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
  printf("LABEL $$Length \nPUSHFRAME \nDEFVAR LF@\037retval \nMOVE LF@\037retval int@0 \nSTRLEN LF@\037retval LF@p1 \nPOPFRAME \nRETURN \n");
  
  printf("\nLABEL $$SubStr \nPUSHFRAME \nDEFVAR LF@\037retval \nDEFVAR LF@len \nDEFVAR LF@help \nSTRLEN LF@len LF@p1 \nMOVE LF@\037retval string@\n");
  printf("JUMPIFEQ $$EndSubStr LF@\037retval LF@p1 \nGT LF@help int@1 LF@p2 \nJUMPIFEQ $$EndSubStr LF@help bool@true \nGT LF@help int@0 LF@p3\n");
  printf("JUMPIFEQ $$SubStrExtra1 LF@help bool@true \nSUB LF@help LF@len LF@p2 \nGT LF@help LF@p3 LF@help \nJUMPIFEQ $$SubStrExtra2 LF@help bool@true\n"); 
  printf("JUMP $$SubStrStart \nLABEL $$SubStrExtra1 \nMOVE LF@p2 LF@len \nJUMP $$SubStrStart \nLABEL $$SubStrExtra2 \nMOVE LF@p3 LF@len\n");
  printf("SUB LF@p3 LF@p3 LF@p2 \nLABEL $$SubStrStart \nADD LF@p3 LF@p2 LF@p3 \nLABEL $$CycleSubStr \nGETCHAR LF@help LF@p1 LF@p2\n");
  printf("CONCAT LF@\037retval LF@\037retval LF@help \nADD LF@p2 LF@p2 int@1 \nJUMPIFNEQ $$CycleSubStr LF@p2 LF@p3 \nLABEL $$EndSubStr \nPOPFRAME \nRETURN \n");
  
  printf("\nLABEL $$Asc \nPUSHFRAME \nDEFVAR LF@\037retval \nDEFVAR LF@help \nMOVE LF@\037retval int@0 \nSTRLEN LF@help LF@p1 \nGT LF@help LF@help LF@p2\n");
  printf("JUMPIFEQ $$EndAsc LF@help bool@false \nGETCHAR LF@\037retval LF@p1 LF@p2 \nSTRI2INT LF@\037retval LF@\037retval int@0 \nLABEL $$EndAsc \nPOPFRAME\n");
  printf("RETURN\n");
  
  printf("\nLABEL $$Chr \nPUSHFRAME \nDEFVAR LF@\037retval \nMOVE LF@\037retval string@ \nINT2CHAR LF@\037retval LF@p1 \nPOPFRAME \nRETURN");
}
