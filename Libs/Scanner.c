/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    Scanner.c
* \brief   This is test module
*
*
*
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#include "Scanner.h"
#include "grammar.h"

Egrammar tokens[] = { kwDim, ident, kwAs, kwtype };

int scan_GetNextToken(SToken *token)
{
  static int pos = 0;
  if (pos > 3)
  {
    return 0;
  }
  token->type = tokens[pos++];
  return 1;
}