/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.h
* \brief   Grammar enumeration
*
* Contains both terminals and non-terminals.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    8.11.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef GRAMMAR
#define GRAMMAR

typedef enum
{
  /*OPERATORS*/
  opPlus, opMns, opMul, opDiv, opDivFlt, opPlusEq, opMnsEq, opMulEq, opDivEq, opDivFltEq, opEq, opLes, opGrt, opLessEq, opGrtEq, opLeftBrc, opRightBrc, opSemcol, opComma,
  /*KEY WORDS*/
  kwAnd, kwAs, kwAsc, kwDeclare, kwDim, kwDo, kwElse, kwEnd, kwFunction, kwIf, kwInput, kwLength, kwLoop, kwPrint, kwReturn, kwScope, kwSubStr, kwThen, kwWhile, kwAnd, kwContinue, kwElseif, kwExit, kwFalse, kwFor, kwNext, kwNot, kwOr, kwShared, kwStatic, kwTrue, kwTo, kwUntil,
  /*OTHER*/
  ident, asng, eol, dataType,
  /*NON-TERMINALS*/
  NTPROG, //Program non-terminal
  NTDD, //Declaration and definition
  NTEOL, //1...n eols
  NTSCOPE, //program body
  NT_STAT_LIST, //statement list
  NT_PARAM_LIST, //paramether list
  NT_PARAM,
  NT_STAT, //statement
  NT_EXPR,
  NT_EXPR_LIST,
  NT_INIF, //body of if statement
}EGrSymb;

/*

2 PROG -> NTDD NTSCOPE
NTSCOPE -> kwScope NT_STAT_LIST kwEnd kwScope eol
NTDD -> (epsilon)
NTDD -> kwDeclare kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NTDD //programuj chytre
NTDD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NTDD
TODO: NTDD -> statick, shared
NT_PARAM_LIST -> (epsilon)
NT_PARAM_LIST -> NT_PARAM
NT_PARAM -> ident kwAs dataType
NT_PARAM -> ident kwAs dataType opComma NT_PARAM
NT_STAT_LIST -> (epsilon)
NT_STAT_LIST -> NT_STAT eol NT_STAT_LIST
NT_EXPR_LIST -> NT_EXPR opSemcol NT_EXPR_LIST
NT_STAT -> ident asng NT_EXPR
NT_STAT -> kwInput ident
NT_STAT -> kwPrint NT_EXPR_LIST
NT_STAT -> kwIf NT_EXPR kwThan eol NT_INIF kwEnd kwIf eol
NT_STAT -> kwDo kwWhile NT_EXPR eol NT_STAT_LIST kwLoop
NT_STAT -> kwDo kwUntil NT_EXPR eol NT_STAT_LIST kwLoop
NT_STAT -> kwDo eol NT_STAT_LIST kwWhile NT_EXPR kwLoop
NT_STAT -> kwDo eol NT_STAT_LIST kwUntil NT_EXPR kwLoop

NT_STAT -> kwContinue
NT_STAT -> kwExit

NT_STAT -> kwFor NT_FOR_ITER asgn NT_EXPR kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext // ?? kwTo pridano
NT_FOR_ITER -> ident
NT_FOR_ITER -> ident kwAs dataType
NT_STEP -> (epsilon)
NT_STEP -> kwStep NT_EXPR

NT_STAT -> kwReturn NT_EXPR
NT_INIF -> NT_STAT_LIST
NT_INIF -> NT_STAT_LIST kwElse eol NT_STAT_LIST
NT_INIF -> NT_STAT_LIST kwElsif NT_EXPR kwThan eol NT_INIF


*/

#endif
