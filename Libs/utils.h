/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.h
 * \brief   Liblary providing supportive function and types for the project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    23.10.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _utils
#define _utils

#include "MMng.h"

/**
 * Structure of basic dataset of input language data types,
 * for storing data of constants or literals ect.
 */
typedef struct {
  int intVal;
  double doubleVal;
  char *stringVal;
} SData;

/**
 * Fuction create hard copy of given string
 * \note Memory is allocated here and free has to be called.
 */
char *util_StrHardCopy(const char *str);

#endif // _utils
