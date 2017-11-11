/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    syntaxAnalyzer.c
* \brief   Syntax analyze
* \author  Pavel Vosyka (xvosyk00)
* \date    10.11.2017 - Pavel Vosyka
*/
/******************************************************************************/

#include <stdio.h>
#include "syntaxAnalyzer.h"
#include "grammar.h"
#include "Scanner.h"
#include "stacks.h"
#include "appErr.h"


SToken token; //loaded token, we might want to have array/list of tokens for code generation
TGrStack statStack; //stack used in syntx_statAnalyze()

//returns closest terminal to top of the stack
EGrSymb syntx_getFirstTerminal(TGrStack stack)
{
  for (int i = 0; i < stack->count; i++)
  {
    EGrSymb symb = stack->grArray[i];
    if (isTerminal(symb))
      return symb;
  }                                                                                 
  apperr_runtimeError("syntaxAnalyzer: Terminal not found!"); //TODO: call codeError
  return eol;
}

//Precedent statement analyze
void syntx_statAnalyze()
{
  statStack->push(statStack, eol);
  while (1)
  {
    EGrSymb terminal = syntx_getFirstTerminal(statStack);
    if (!scan_GetNextToken(&token))
    {
      break;
    }
    printf("Analyzing token: %d\n", token.type);

  }
}

void syntx_analyzeCode()
{
  //---------init-----------
  statStack = TGrStack_create();

  //---------Parse----------
  syntx_statAnalyze();

}

