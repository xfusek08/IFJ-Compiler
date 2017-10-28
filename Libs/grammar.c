/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.c
* \brief   Grammar support functions
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/
#include "grammar.h"

const char *printEgrammar(Egrammar grammar)
{
  switch (grammar)
  {
  //nonterminal
  case nwS:
    return "nwS";
  case nwVarDefinition:
    return "nwVarDefinition";
  case nwExpr:
    return "nwExpr";
  case nwDimEx:
    return "nwDimEx";
  //terminals
  case kwDim:
    return "kwDim";
  case ident:
    return "ident";
  case kwAs:
    return "kwAs";
  case kwtype:
    return "kwtype";
  case asng:
    return "asng";
  default:
    return "undefined grammar type";
  }
}