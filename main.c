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
#include "Libs/stacks.h"
#include "Libs/symtable.h"
#include "Libs/rParser.h"
#include "Libs/Scanner.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init("$$main");
  scan_init();


  rparser_processProgram();
  /*
  int i = 0;
  SToken token;
  token.type = eol;
  while(token.type != eof)
  {
    token = scan_GetNextToken();
    printf("%d: Token: %s \n", i, TokenTypeStrings[token.type]);
    if(token.symbol != NULL)
      symbt_printSymb(token.symbol);
    i++;
  }
  */
  scan_destroy();
  symbt_destroy();
  mmng_freeAll();
  return 0;
}
