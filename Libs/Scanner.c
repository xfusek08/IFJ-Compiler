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
#include <stdio.h>
#include <stdlib.h>

EGrSymb tokens[] = {ident, opPlus, ident, eol };

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

void scan_raiseCodeError(ErrType typchyby)
{
  fprintf(stderr, "Scanner> some error!\n");
  exit(typchyby);
}
