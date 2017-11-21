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

#include "Scanner.h"

void syntx_init();
void syntx_processExpression(SToken *actToken, const char *frame, const char *ident, DataType datatype);



#endif