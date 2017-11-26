
#include <stdlib.h>
#include <stdio.h>
#include "MMng.h"
#include "syntaxAnalyzer.h"
#include "symtable.h"

int cnt = 0;

TSymbol syntx_processExpression(SToken *actToken, TSymbol outsymb)
{
  (void)actToken;
  if (outsymb != NULL)
    return outsymb;
  char *symbname = mmng_safeMalloc(sizeof(char) * 14);
  sprintf(symbname, "LF@%%T%d", cnt++);
  TSymbol symbol = symbt_findOrInsertSymb(symbname);
  mmng_safeFree(symbname);
  printf("DEFVAR %s\n", symbol->ident);
  printf("MOVE %s %s\n", symbol->ident, "bool@true");
  return symbol;
}
