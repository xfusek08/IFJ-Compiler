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
#include "Libs/syntaxAnalyzer.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();

  syntx_analyzeCode();

  mmng_freeAll();


  return 0;
}
