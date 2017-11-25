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
#include "symtable.h"

EGrSymb tokens[] = {ident, opPlus, ident, eol };

SToken makeIdent(char *identificator, DataType datType) 
{
  SToken token;
  token.type = ident;
  token.symbol = malloc(sizeof(struct Symbol));
  token.symbol->type = symtVariable;
  token.symbol->dataType = datType;
  token.symbol->ident = identificator;
  return token;
}

int scan_GetNextToken(SToken *token)
{
  static int pos = 0;
  if (pos > 11)
  {
    return 0;
  }

  if (pos == 0)
  {
    *token = makeIdent("LF@ident1", dtInt);
  }
  else if (pos == 1) {
    token->type = opPlus;
  }
  else if (pos == 2) {
    token->type = opPlus;
  }
  else if (pos == 3)
  {
    *token = makeIdent("LF@ident3", dtInt);
  }
  else if (pos == 4)
  {
    token->type = eol;
  }
  else if (pos == 5)
  {
    token->type = opMul;
  }
  else if (pos == 6)
  {
    *token = makeIdent("LF@ident4", dtInt);
  }
  else if (pos == 7)
  {
    token->type = opPlus;
  }
  else if (pos == 8)
  {
    *token = makeIdent("LF@ident5", dtInt);
  }
  else if (pos == 9)
  {
    token->type = opMul;
  }
  else if (pos == 10)
  {
    *token = makeIdent("LF@ident6", dtInt);
  }
  else if (pos == 11)
  {
    token->type = eol;
  }
  pos++;
  return 1;
}

void scan_raiseCodeError(ErrType typchyby)
{
  fprintf(stderr, "Scanner> some error!\n");
  exit(typchyby);
}
