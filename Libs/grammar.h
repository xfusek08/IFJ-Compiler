/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.h
* \brief   Grammar enumeration
*
* Contains both terminals and non-terminals.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    11.11.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef GRAMMAR
#define GRAMMAR

//note: Be carefull with changing values of enum, isTerminal() rely on them.
typedef enum
{
  /* TERMINALS */
  /* operators */
  opPlus = 0, opMns, opMul, opDiv, opDivFlt, opPlusEq, opMnsEq, opMulEq, opDivEq, opDivFltEq, opEq, opLes, opGrt,
  opLessEq, opGrtEq, opLeftBrc, opRightBrc, opSemcol, opComma,

  /* key words */
  kwAs, kwAsc, kwDeclare, kwDim, kwDo, kwElse, kwEnd, kwFunction, kwIf, kwInput, kwLength, kwLoop,
  kwPrint, kwReturn, kwScope, kwSubStr, kwThen, kwWhile, kwAnd, kwContinue, kwElseif, kwExit, kwFalse, kwFor,
  kwNext, kwNot, kwOr, kwShared, kwStatic, kwTrue, kwTo, kwUntil,

  /* other */
  ident, asng, eol, dataType,

  /* NON-TERMINALS */
  NT_PROG = 1000,          // Program - staritng non-terminal
  NT_ASSINGEXT,     // Assignement (...  [as datatype])
  NT_DD_EXT,        // inicialization of static variable
  NT_SCOPE,         // program body
  NT_STAT_LIST,     // statement list
  NT_STAT,          // one statement
  NT_STAT_DOIN,     // body of do..loop statement
  NT_STAT_DOIN_WU,  // [while/until]
  NT_PARAM_LIST,    // parameter list
  NT_PARAM,         // one parameter
  NT_PARAM_EXT,     // continuous parameter (, param ,pram)
  NT_EXPR_LIST,     // expresion list
  NT_EXPR,          // expresion
  NT_INIF,           // body of if statement

  /* Precedence table symbols */
  priorLess, priorEq, priorGrt
}EGrSymb;



/*

1. NT_PROG -> NT_DD NT_SCOPE

first(NT_DD) = { kwDeclare -> (2); kwFunction -> (3);  kwStatic -> (4); else -> (5 [epsilon]) }
2. NT_DD -> kwDeclare kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_DD
3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
4. NT_DD -> kwStatic kwShared ident kwAs dataType NT_DD_EXT
5. NT_DD -> (epsilon)

first(NT_ASSINGEXT) = { asgn -> (6); else -> (7 [epsilon]) }
6. NT_ASSINGEXT -> asgn NT_EXPR
7. NT_ASSINGEXT -> (epsilon)

first(NT_SCOPE) = { kwScope -> (8); else -> (error) }
8. NT_SCOPE -> kwScope NT_STAT_LIST kwEnd kwScope eol

first(NT_SCOPE) = { first(NT_PARAM) -> (9); else -> (10 [epsilon]) }
9.  NT_PARAM_LIST -> NT_PARAM
10. NT_PARAM_LIST -> (epsilon)

first(NT_PARAM) = { ident -> (11); else -> (error) }
11. NT_PARAM -> ident kwAs dataType NT_PARAM_EXT

first(NT_PARAM_EXT) = { opComma -> (12); else -> (13 [epsilon]) }
12. NT_PARAM_EXT -> opComma NT_PARAM
13. NT_PARAM_EXT -> (epsilon)

first(NT_STAT_LIST) = { first(NT_STAT) -> (14); else -> (15 [epsilon]) }
14. NT_STAT_LIST -> NT_STAT eol NT_STAT_LIST
15. NT_STAT_LIST -> (epsilon)

first(NT_STAT) = {
  kwInput -> (16);
  kwPrint -> (17);
  kwIf -> (18);
  kwDim -> (19);
  ident -> (20);
  kwContinue -> (21);
  kwExit -> (22);
  first(NT_SCOPE) -> (23);
  kwReturn -> (24);
  kwDo -> (25);
  kwFor -> (26);
  else -> (error) }
16. NT_STAT -> kwInput ident
17. NT_STAT -> kwPrint NT_EXPR_LIST
18. NT_STAT -> kwIf NT_EXPR kwThan eol NT_INIF kwEnd kwIf eol
19. NT_STAT -> kwDim iden kwAs dataType NT_ASSINGEXT
20. NT_STAT -> ident asng NT_EXPR
21. NT_STAT -> kwContinue
22. NT_STAT -> kwExit
23. NT_STAT -> NT_SCOPE
24. NT_STAT -> kwReturn NT_EXPR
25. NT_STAT -> kwDo NT_DOIN kwLoop
26. NT_STAT -> kwFor ident NT_ASSINGEXT kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext

first(NT_DOIN) = { first(NT_DOIN_WU) -> (27); eol -> (28) else -> (error)}
27. NT_DOIN -> NT_DOIN_WU NT_EXPR eol NT_STAT_LIST
28. NT_DOIN -> eol NT_STAT_LIST NT_DOIN_WU NT_EXPR

first(NT_DOIN_WU) = { kwWhile -> (29); kwUntil -> (30); else -> (error) }
29. NT_DOIN_WU -> kwWhile
30. NT_DOIN_WU -> kwUntil

first(NT_FORSTEP) = { kwStep -> (31); else -> (32 [epsilon]) }
31. NT_FORSTEP -> kwStep NT_EXPR
32. NT_FORSTEP -> (epsilon)

first(NT_INIF) = { fist(NT_STAT_LIST) -> (33); else -> (32 [epsilon]) }
33. NT_INIF -> NT_STAT_LIST NT_INIF_EXT

first(NT_INIF_EXT) = { kwElsif -> (34); kwElse -> (35); else -> (36 [epsilon]) }
34. NT_INIF_EXT -> kwElsif NT_EXPR kwThan eol NT_INIF
35. NT_INIF_EXT -> kwElse eol NT_STAT_LIST
36. NT_INIF_EXT -> (epsilon)

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
