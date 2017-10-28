/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.h
* \brief   Grammar enumeration
*
* Contains both terminals and non-terminals.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef GRAMMAR
#define GRAMMAR

typedef enum
{
  /*NON-TERMINALS*/
  nwS, nwVarDefinition, nwExpr, nwDimEx,

  /*TERMINALS*/
  kwDim, ident, kwAs, kwtype, asng
}Egrammar;

const char *printEgrammar(Egrammar grammar);

#endif
