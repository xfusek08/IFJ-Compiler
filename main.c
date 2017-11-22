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
#include "Libs/Scanner.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();
  Scanner_init();
  
  int i = 0;
  SToken token;
  while(i < 20)
  {
    printf("-----------Dalsi token----------- \n");
    token = scan_GetNextToken();
    printf("Typ tokenu: %d \n",token.type);
    printf("Datovy typ tokenu: %d \n",token.dataType);
    if(token.symbol != NULL)
    {
      printf("Identifikator symbolu: %s \n",token.symbol->ident);
      printf("Typ symbolu: %d \n",token.symbol->type);
      symbt_printSymb(token.symbol);
    }
    //printf("Datovy typ: %d \n",token.dataType);
    printf("--------------------------------- \n");
    i++;
  }
  Scanner_destroy();
  symbt_destroy();
  mmng_freeAll();
  return 0;
}
