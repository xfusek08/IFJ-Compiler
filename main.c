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

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();

  syntx_init();
  SToken token;
  syntx_processExpression(&token, NULL, NULL, dtUnspecified);

  symbt_destroy();
  mmng_freeAll();
  return 0;
}
