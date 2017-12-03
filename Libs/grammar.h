/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    grammar.h
* \brief   Grammar enumeration
*
* This file contains enummeration of grammar (terminal + non-terminals) and definition of token provided by scanner and processed by parsers.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    21.11.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef _Grammar
#define _Grammar

//note: Be carefull with changing values of enum, isTerminal() rely on them.
//note: eol is used to limit expression symbols ( (symbol <= eol) ===> symbol is valid expression symbol )
typedef enum
{
  /* TERMINALS */

  /* operators and other*/
  /* DON'T CHANGE IT PART - used to index the array */
  opPlus = 0, opMns, opMul, opDivFlt, opDiv, opLeftBrc, opRightBrc, ident, opComma,
  opEq, opNotEq, opLes, opLessEq, opGrt, opGrtEq, asgn, opPlusEq, opMnsEq, opMulEq, opDivEq, opDivFltEq,
  /*boolean operators*/
  opBoolNot, opBoolAnd, opBoolOr,

  eol,
  /* END OF DON'T CHANGE IT PART */
  opSemcol, dataType, eof,

  /* key words */
  kwAs, kwDeclare, kwDim, kwDo, kwElse, kwEnd, kwFunction, kwIf, kwInput, kwLoop,
  kwPrint, kwReturn, kwScope, kwThen, kwWhile, kwContinue, kwElseif, kwExit, kwFalse, kwFor,
  kwNext, kwShared, kwStatic, kwTrue, kwTo, kwUntil, kwStep,

  /* NON-TERMINALS */
  NT_PROG = 1000,          // Program - staritng non-terminal
  NT_DD,            // definitions and declarations section
  NT_ASSINGEXT,     // Assignement (...  [as datatype])
  NT_SCOPE,         // Scope statement where local variables can be owerriten.
  NT_PARAM_LIST,    // list of parameters
  NT_PARAM,         // one parameter
  NT_PARAM_EXT,     // continue of param list
  NT_STAT_LIST,     // list of statements
  NT_STAT,          // one statement
  NT_DOIN,          // body of do..loop statement
  NT_DOIN_WU,       // until or while neterminal
  NT_FORSTEP,       // step of for
  NT_INIF_EXT,      // extension of body of if statement
  NT_PRINT_LIST,    // list of expression for print function
  NT_EXPR,          // one expresion
  NT_ARGUMENT_LIST, // list of expression separated by comma
  NT_EXPR_TMP,      // one expresion, indicates result in auxiliary variable (for code optimization)
  NT_CYCLE_NESTS,   // extension of contuinue and exit
  NT_CYCLES_DO,     // list of do kws
  NT_CYCLES_FOR,    // list of for kws
  /* Precedence table symbols */
  precLes, precEqu, precGrt, precUnd
} EGrSymb;

/* GRAMMAR RULES:

1. NT_PROG -> NT_DD NT_SCOPE

first(NT_DD) = { kwDeclare -> (2); kwFunction -> (3);  kwStatic -> (4); else -> (5 [epsilon]) }
2. NT_DD -> kwDeclare kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_DD
3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
4. NT_DD -> kwStatic kwShared ident kwAs dataType NT_ASSINGEXT
5. NT_DD -> (epsilon)

first(NT_ASSINGEXT) = { asgn -> (6); else -> (7 [epsilon]) }
6. NT_ASSINGEXT -> asgn NT_EXPR
7. NT_ASSINGEXT -> (epsilon)

first(NT_SCOPE) = { kwScope -> (8); else -> (error) }
8. NT_SCOPE -> kwScope eol NT_STAT_LIST kwEnd kwScope eol

first(NT_PARAM_LIST) = { first(NT_PARAM) -> (9); else -> (10 [epsilon]) }
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
17. NT_STAT -> kwPrint NT_PRINT_LIST
18. NT_STAT -> kwIf NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT kwEnd kwIf
19. NT_STAT -> kwDim ident kwAs dataType NT_ASSINGEXT
20. NT_STAT -> ident opEq NT_EXPR
21. NT_STAT -> kwContinue NT_CYCLE_NESTS
22. NT_STAT -> kwExit NT_CYCLE_NESTS
23. NT_STAT -> NT_SCOPE
24. NT_STAT -> kwReturn NT_EXPR
25. NT_STAT -> kwDo NT_DOIN
26. NT_STAT -> kwFor ident [as datatype] NT_ASSINGEXT kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext

first(NT_DOIN) = { first(NT_DOIN_WU) -> (27); eol -> (28) else -> (error)}
27. NT_DOIN -> NT_DOIN_WU eol NT_STAT_LIST kwLoop
28. NT_DOIN -> eol NT_STAT_LIST kwLoop NT_DOIN_WU

first(NT_DOIN_WU) = { kwWhile -> (29); kwUntil -> (30); else -> (31 [epsilon]) }
29. NT_DOIN_WU -> kwWhile NT_EXPR
30. NT_DOIN_WU -> kwUntil NT_EXPR
31. NT_DOIN_WU -> (epsilon)

first(NT_FORSTEP) = { kwStep -> (31); else -> (32 [epsilon]) }
32. NT_FORSTEP -> kwStep NT_EXPR
33. NT_FORSTEP -> (epsilon)

first(NT_INIF_EXT) = { kwElseif -> (33); kwElse -> (34); else -> (35 [epsilon]) }
34. NT_INIF_EXT -> kwElseif NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT
35. NT_INIF_EXT -> kwElse eol NT_STAT_LIST
36. NT_INIF_EXT -> (epsilon)

first(NT_PRINT_LIST) = { first(NT_EXPR) -> (36); else -> (37 [epsilon]) }
37. NT_PRINT_LIST -> NT_EXPR opSemcol NT_PRINT_LIST
38. NT_PRINT_LIST -> (epsilon)

first(NT_CYCLE_NESTS) = {kwDo -> 38; kwFor -> 39; else -> (40 [epsilon])}
39. NT_CYCLE_NESTS -> kwDo NT_CYCLES_DO
40. NT_CYCLE_NESTS -> kwFor NT_CYCLES_FOR
41. NT_CYCLE_NESTS -> (epsilon)

first(NT_CYCLES_DO) = {opComma -> 41; else -> (42 [epsilon])}
42. NT_CYCLES_DO -> , kwDo NT_CYCLES_DO
43. NT_CYCLES_DO -> (epsilon)

first(NT_CYCLES_FOR) = {opComma -> 43; else -> (44 [epsilon])}
44. NT_CYCLES_FOR -> opComma kwFor NT_CYCLES_FOR
45. NT_CYCLES_FOR -> (epsilon)

// following rules does not contain epsilon rules and it will be process by another algorithm

46. NT_EXPR -> ident
47. NT_EXPR -> NT_EXPR opPlus NT_EXPR
48. NT_EXPR -> NT_EXPR opMns NT_EXPR
49. NT_EXPR -> NT_EXPR opMul NT_EXPR
50. NT_EXPR -> NT_EXPR opDiv NT_EXPR
51. NT_EXPR -> NT_EXPR opDivFlt NT_EXPR
52. NT_EXPR -> NT_EXPR opPlusEq NT_EXPR
53. NT_EXPR -> NT_EXPR opMnsEq NT_EXPR
54. NT_EXPR -> NT_EXPR opMulEq NT_EXPR
55. NT_EXPR -> NT_EXPR opDivEq NT_EXPR
56. NT_EXPR -> NT_EXPR opDivFltEq NT_EXPR
57. NT_EXPR -> NT_EXPR opEq NT_EXPR
58. NT_EXPR -> NT_EXPR opLes NT_EXPR
59. NT_EXPR -> NT_EXPR opGrt NT_EXPR
60. NT_EXPR -> NT_EXPR opLesEq NT_EXPR
61. NT_EXPR -> NT_EXPR opGrtEq NT_EXPR
62. NT_EXPR -> ident kwLeftBrt NT_ARGUMENT_LIST kwRightBrt
63. NT_EXPR -> kwLeftBrc NT_EXPR kwRightBrc

64. NT_ARGUMENT_LIST -> NT_EXPR opComma NT_ARGUMENT_LIST
65. NT_ARGUMENT_LIST -> NT_EXPR
66. NT_ARGUMENT_LIST -> (epsilon)

*/

#endif // _Grammar
