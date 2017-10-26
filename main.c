/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    3.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Libs/MMng.h"
#include "Libs/stacks.h"
#include "Libs/symtable.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();

  // ... spousta mallocu a frees ...

  symbt_destroy();
  mmng_freeAll();


  return 0;
}
