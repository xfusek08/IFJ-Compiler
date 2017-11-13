/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.h
 * \brief   Liblary providing supportive function and types for the project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    13.11.2017 - Petr Fusek
 */
/******************************************************************************/

#ifndef _utils
#define _utils

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

/**
 * Fuction create hard copy of given string
 * \note Memory is allocated here and free has to be called.
 */
char *util_StrHardCopy(const char *str);

#endif // _utils
