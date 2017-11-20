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
* Returns 0 if symbol is not defined, otherwise procLess, procEq, procGrt
* stackSymb symbol on top of stack
* inputSymb symbol from end of expression
*/
int precTable(EGrSymb stackSymb, EGrSymb inputSymb, EGrSymb *ref)
{

/*
  typedef struct{
    EGrSymb stackSymb;
    EGrSymb inputSymb;
    EGrSymb priority;
  } precTableItem;

  int placeholder = 1486456;

  const precTableItem precTable[] = {
    //line 1
    {opPlus, opPlus, procGrt},
    {opPlus, opMns, procGrt},
    {opPlus, opMul, procGrt},
    {opPlus, opDiv, procGrt},
    {opPlus, opDivFlt, procGrt},
    {opPlus, opLeftBrc, procGrt},
    {opPlus, opRightBrc, procGrt},
    {opPlus, placeholder, procGrt}, //TODO: i
    {opPlus, kwFunction, procGrt},
    {opPlus, opMul, procGrt},
    {opPlus, opMul, procGrt},
    {opPlus, opMul, procGrt},
    {opPlus, opMul, procGrt},
    //line 2
  };

  if(stackSymb == opPlus){  // stack: +
    
  }else if(stackSymb == opMns){ // stack: -

  }else if(stackSymb == opMul){ // stack: *

  }else if(stackSymb == opDiv){ // stack: /

  }else if(stackSymb == opDivFlt){  // stack: '\'

  }else if(stackSymb == opLeftBrc){ // stack: (

  }else if(stackSymb == opRightBrc){  // stack: )

  }else if(stackSymb == opMns){ // stack: TODO: i

  }else if(stackSymb == kwFunction){ // stack: function

  }else if(stackSymb == opComma){ // stack: ,

  }else if(stackSymb == opEq){ // stack: ==

  }else if(stackSymb == opMns){ // stack: TODO: not equal

  }else if(stackSymb == opLes){ // stack: <

  }else if(stackSymb == opLessEq){ // stack: <=

  }else if(stackSymb == opGrt){ // stack: >

  }else if(stackSymb == opGrtEq){ // stack: >=

  }else if(stackSymb == opMns){ // stack: TODO: end of expression

  }

  //stack symbol has greater priority than input symbol
  switch(stackSymb == "+"){
    case inputSymb == "+":
      break;
  }
  */
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

