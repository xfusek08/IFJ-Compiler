/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    10.11.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "Libs/MMng.h"
#include "Libs/syntaxAnalyzer.h"
#include "Libs/stacks.h"
#include "Libs/symtable.h"
#include "Libs/Scanner.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();

  syntx_init();
  SToken token;
  scan_GetNextToken(&token);

  TSymbol symbol = mmng_safeMalloc(sizeof(struct Symbol));
  symbol->ident = "LF@myVar";
  symbol->type = symtVariable;
  syntx_processExpression(&token, symbol);

  symbt_destroy();
  mmng_freeAll();
  return 0;
}
