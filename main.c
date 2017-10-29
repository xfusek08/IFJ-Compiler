/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    29.10.2017 - Petr Fusek
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
    char keychar;
    printf(">");
    scanf(" %c", &keychar);
    char keystring[2];
    keystring[0] = keychar;
    keystring[1] = '\0';
    symbt_findOrInsertSymb(keystring);
    symbt_print();
  }

  symbt_destroy();
  mmng_freeAll();


  return 0;
}
