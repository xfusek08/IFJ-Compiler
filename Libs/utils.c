/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.c
 * \brief   Liblary providing supportive function for project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    18.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include "utils.h"

// hard string copy
char *util_StrHardCopy(const char *str)
{
  return strcpy(mmng_safeMalloc(sizeof(char) * strlen(str)), str);
}
