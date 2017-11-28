/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    syntaxAnalyzer.h
* \brief   Syntax analyze
*
*
*
* \author  Pavel Vosyka (xvosyk00)
* \date    28.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef _SYNTAXANALYZER
#define _SYNTAXANALYZER

#include "grammar.h"
#include "symtable.h"
#include "scanner.h"

void syntx_freeVar(SToken *var);

SToken sytx_getFreeVar();

TSymbol syntx_processExpression(SToken *actToken, TSymbol symbol);

void syntx_init();

#endif // _SYNTAXANALYZER