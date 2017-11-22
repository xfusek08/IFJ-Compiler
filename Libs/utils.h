
/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    utils.h
 * \brief   Liblary providing supportive function and types for the project
 *
 * \author  Petr Fusek (xfusek08)
 * \date    14.11.2017 - Petr Fusek
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

/**
 * Fuction returns concatenated str1 and str2 (str = str1 + str2) Inputed strings are just readed and new string is created
 * \note Memory is allocated here and free has to be called.
 */
char *util_StrConcatenate(const char *str1, const char *str2);

/**
 * Fuction converts data type enum to constant string
 */
const char *util_dataTypeToString(const DataType dataType);

#endif // _utils
