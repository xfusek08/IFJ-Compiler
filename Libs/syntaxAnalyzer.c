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

#define DPRINT(x) printf x

SToken token; //loaded token, we might want to have array/list of tokens for code generation
TGrStack statStack; //stack used in syntx_statAnalyze()

//radim******
/**
* Returns 1 if symbol is not defined
*/
int Table(EGrSymb a, EGrSymb b, EGrSymb *ref)
{

}      
//radim konec*************

/**
* returns 1 if symbol is terminal, otherwise 0
*/
int isTerminal(EGrSymb symb)
{
  return symb < 1000 ? 1 : 0;
}

/**
* returns closest terminal to top of the stack
*/
EGrSymb syntx_getFirstTerminal(TGrStack stack, int *position)
{
  for (int i = 0; i < stack->count; i++)
  {
    EGrSymb symb = stack->grArray[i];
    if (isTerminal(symb))
    {
      *position = i;
      return symb;
    }
  }                                                                                 
  scan_raiseCodeError(syntaxErr);
  return eol;
}

/**
* Semantic statement analyze
*/
void statSemantic(int ruleNum)
{
  DPRINT(("Semantic statement analyze: Received rule number %d", ruleNum));
}

SToken *nextToken()
{
  if (!scan_GetNextToken(&token))
  {
    fprintf(stderr, "File unexpectedly ended.");
    scan_raiseCodeError(syntaxErr);
  }
  return token;
}

int syntx_useRule(TGrStack stack, int *usedRule)
{
  return 1;
}

/**
* Precedent statement analyze
*/
void syntx_statAnalyze()
{
  statStack->push(statStack, eol);
  SToken *token - nextToken();

  do {
    int terminalPosition;
    EGrSymb terminal = syntx_getFirstTerminal(statStack, &terminalPosition);

    //debug print
    DPRINT(("Analyzing token: %d\n", token->type));
    DPRINT(("First terminal on stack: %d\n", terminal));

    EGrSymb tablesymb;
    if (!Table(terminal, token->type, &tablesymb))
    {
      scan_raiseCodeError(syntaxErr);
    }

    switch (tablesymb)
    {
    case priorEq:
      statStack->push(statStack, token->type);
      token = nextToken();
      break;
    case priorLess:
      TGrStack_postInsert(statStack, terminalPosition, priorLess);
      statStack->push(statStack, token->type);
      token = nextToken();
      break;
    case priorGrt:
      int rule;
      if (syntx_useRule(statStack, &rule))
      {
        DPRINT(("SyntaxAnalyzer: used rule number %d", rule));
      }
      else {
        scan_raiseCodeError(syntaxErr);
      }
      break;
    default:
      apperr_runtimeError("SyntaxAnalyzer.c: unsupported symbol!");
    }
  }while(terminal == eol && token->type == eol)
}


void syntx_analyzeCode()
{
  //---------init-----------
  statStack = TGrStack_create();

  //---------Parse----------
  syntx_statAnalyze();

}

