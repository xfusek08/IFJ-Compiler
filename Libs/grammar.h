/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.h
* \brief   Grammar enumeration
*
* Contains both terminals and non-terminals.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    10.11.2017 - Petr Fusek
*/
/******************************************************************************/

#ifndef GRAMMAR
#define GRAMMAR

typedef enum
{
  /* TERMINALS */
  /* operators */
  opPlus, opMns, opMul, opDiv, opDivFlt, opPlusEq, opMnsEq, opMulEq, opDivEq, opDivFltEq, opEq, opLes, opGrt,
  opLessEq, opGrtEq, opLeftBrc, opRightBrc, opSemcol, opComma,

  /* key words */
  kwAnd, kwAs, kwAsc, kwDeclare, kwDim, kwDo, kwElse, kwEnd, kwFunction, kwIf, kwInput, kwLength, kwLoop,
  kwPrint, kwReturn, kwScope, kwSubStr, kwThen, kwWhile, kwAnd, kwContinue, kwElseif, kwExit, kwFalse, kwFor,
  kwNext, kwNot, kwOr, kwShared, kwStatic, kwTrue, kwTo, kwUntil,

  /* other */
  ident, asng, eol, dataType,

  /* NON-TERMINALS */
  NT_PROG,        // Program - staritng non-terminal
  NT_DD,          // Declaration and definition
  NT_SCOPE,       // program body
  NT_STAT_LIST,   // statement list
  NT_STAT,        // one statement
  NT_PARAM_LIST,  // parameter list
  NT_PARAM,       // one parameter
  NT_EXPR_LIST,   // expresion list
  NT_EXPR,        // expresion
  NT_INIF         // body of if statement
}EGrSymb;

/*

NT_PROG -> NT_DD NT_SCOPE
NT_SCOPE -> kwScope NT_STAT_LIST kwEnd kwScope eol

NT_DD -> (epsilon)
NT_DD -> kwDeclare kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_DD //programuj chytre
NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
NT_DD -> kwStatic kwShared ident kwAs dataType
NT_DD -> kwStatic kwShared ident kwAs dataType asgn NT_EXPR

NT_PARAM_LIST -> (epsilon)
NT_PARAM_LIST -> NT_PARAM

NT_PARAM -> ident kwAs dataType
NT_PARAM -> ident kwAs dataType opComma NT_PARAM

NT_STAT_LIST -> (epsilon)
NT_STAT_LIST -> NT_STAT eol NT_STAT_LIST

NT_STAT -> kwDim iden kwAs dataType
NT_STAT -> kwDim iden kwAs dataType asgn NT_EXPR
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
NT_STAT -> NT_SCOPE
NT_STAT -> kwReturn NT_EXPR

NT_STAT -> kwFor NT_FOR_ITER asgn NT_EXPR kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext // ?? kwTo pridano

NT_FOR_ITER -> ident
NT_FOR_ITER -> ident kwAs dataType

NT_STEP -> (epsilon)
NT_STEP -> kwStep NT_EXPR

NT_INIF -> NT_STAT_LIST
NT_INIF -> NT_STAT_LIST kwElse eol NT_STAT_LIST
NT_INIF -> NT_STAT_LIST kwElsif NT_EXPR kwThan eol NT_INIF

NT_EXPR_LIST -> (epsilon)
NT_EXPR_LIST -> NT_EXPR opSemcol NT_EXPR_LIST

NT_EXPR -> ident
NT_EXPR -> NT_EXPR opPlus NT_EXPR
NT_EXPR -> NT_EXPR opMns NT_EXPR
NT_EXPR -> NT_EXPR opMul NT_EXPR
NT_EXPR -> NT_EXPR opDiv NT_EXPR
NT_EXPR -> NT_EXPR opDivFlt NT_EXPR
NT_EXPR -> NT_EXPR opPlusEq NT_EXPR
NT_EXPR -> NT_EXPR opMnsEq NT_EXPR
NT_EXPR -> NT_EXPR opMulEq NT_EXPR
NT_EXPR -> NT_EXPR opDivEq NT_EXPR
NT_EXPR -> NT_EXPR opDivFltEq NT_EXPR
NT_EXPR -> NT_EXPR opEq NT_EXPR
NT_EXPR -> NT_EXPR opLes NT_EXPR
NT_EXPR -> NT_EXPR opGrt NT_EXPR
NT_EXPR -> NT_EXPR opLesEq NT_EXPR
NT_EXPR -> NT_EXPR opGrtEq NT_EXPR
NT_EXPR -> ident kwLeftBrt NT_ARGUMENT_LIST kwRightBrt
NT_EXPR -> kwLeftBrc NT_EXPR kwRightBrc

NT_ARGUMENT_LIST -> NT_EXPR opComma NT_ARGUMENT_LIST
NT_ARGUMENT_LIST -> NT_EXPR

*/

#endif
