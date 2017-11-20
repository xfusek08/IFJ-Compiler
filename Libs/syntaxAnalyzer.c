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

/**
* Precedent statement analyze
*/
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
    printf("First terminal on stack: %d\n", terminal);
  }
}


void syntx_analyzeCode()
{
  //---------init-----------
  statStack = TGrStack_create();

  //---------Parse----------
  syntx_statAnalyze();

}

