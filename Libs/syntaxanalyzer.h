/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    syntaxAnalyzer.h
* \brief   Syntax analyze
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

/**
* Returns token with unused auxiliary variable, if there is no unused variable, new is crated.
* \return Variable with unspecified type. Token is NT_EXPR_TMP type.
*/
SToken sytx_getFreeVar();

/**
* Mark auxiliary variable as unused and prepare for another use.
*/
void syntx_freeVar(SToken *var);

/**
 * Start precedent analyze of epression.
 * \param actToken is first token of expression
 * \param symbol is variable where result will be, if null, result variable/constant is created
 * \return Function returns symbol. If symbol is null, returns variable/constant with result of expression.
 */
TSymbol syntx_processExpression(SToken *actToken, TSymbol symbol);

/**
 * Initialize precedent analyzer module. 
 */
void syntx_init();

/**
 * Free all resources of precedent analyzer module.
 * Call this only when syntx_init() has already been called.
 */
void syntx_destroy();

#endif // _SYNTAXANALYZER