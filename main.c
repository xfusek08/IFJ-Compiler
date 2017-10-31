/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    30.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Libs/MMng.h"
#include "Libs/stacks.h"
#include "Libs/symtable.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();
  while (1)
  {
    char ident[50];
    printf("> ");
    scanf(" %50s", ident);
    symbt_findOrInsertSymb(ident);
    printf("\n");
    symbt_print();
    printf("\n");
  }

  symbt_destroy();
  mmng_freeAll();


  return 0;
}
