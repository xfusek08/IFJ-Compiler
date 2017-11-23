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

// testovaci vypis
#ifdef DEBUG
const char *TokenTypeStrings[] = {
  "opPlus", "opMns", "opMul", "opDiv", "opDivFlt", "opPlusEq", "opMnsEq", "opMulEq", "opDivEq", "opDivFltEq", "opEq", "opNotEq", "opLes", "opGrt",
  "opLessEq", "opGrtEq", "opLeftBrc", "opRightBrc", "opSemcol", "opComma",
  /*boolean operators*/
  "opBoolNot", "opBoolAnd", "opBoolOr",

  /* key words */
  "kwAs", "kwAsc", "kwDeclare", "kwDim", "kwDo", "kwElse", "kwEnd", "kwFunction", "kwIf", "kwInput", "kwLength", "kwLoop",
  "kwPrint", "kwReturn", "kwScope", "kwSubStr", "kwThen", "kwWhile", "kwContinue", "kwElseif", "kwExit", "kwFalse", "kwFor",
  "kwNext", "kwShared", "kwStatic", "kwTrue", "kwTo", "kwUntil",

  /* other */
  "ident", "asng", "eol", "eof", "dataType"
};
#endif
>>>>>>> master

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();
  scan_init();

  rparser_processProgram();

  scan_destroy();
  symbt_destroy();
  mmng_freeAll();
  return 0;
}
