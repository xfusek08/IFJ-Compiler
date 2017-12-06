/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    6.12.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "Libs/mmng.h"
#include "Libs/symtable.h"
#include "Libs/rparser.h"
#include "Libs/scanner.h"
#include "Libs/syntaxanalyzer.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init("$$main");
  scan_init();
  syntx_init();
  rparser_processProgram();
  syntx_destroy();
  scan_destroy();
  symbt_destroy();
  mmng_freeAll();
  return 0;
}
