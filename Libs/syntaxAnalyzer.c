/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    syntaxAnalyzer.c
* \brief   Syntax analyze
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#include <stdio.h>
#include "syntaxAnalyzer.h"
#include "Scanner.h"


void syntx_analyzeCode()
{
  SToken token;
  while (1)
  {
    if (!scan_GetNextToken(&token))
    {
      break;
    }
    printf("Analyzing token: %s\n", printEgrammar(token.type));
  }
}

