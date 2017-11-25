/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    erpxSemanticAnalyzer.h
* \brief   Expression semantic analyze
*
* 
*
* \author  Radim Blaha (xblaha28)
* \date    25.11.2017 - Radim Blaha
*/
/******************************************************************************/

#ifndef _EXPRSEMANTICANALYZER
#define __EXPRSEMANTICANALYZER

#include "grammar.h"

int syntx_getPrecedence(EGrSymb stackSymb, EGrSymb inputSymb, EGrSymb *precRtrn);

SToken syntx_doArithmeticOp(SToken *leftOperand, SToken *oper, SToken *rightOperand);

// void syntx_generateCodeForVarDef(SToken *funcToken, int argIndex, SToken *argValue);

// void syntx_generateCodeForCallFunc(SToken *funcToken, int argIndex, SToken *result);

void syntx_generateCode(SToken *leftOperand, SToken *oper, SToken *rightOperand, SToken *partialResult);

void syntx_testFunction();



#endif