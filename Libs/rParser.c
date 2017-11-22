/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    rParser.c
 * \brief   Implementation of main parser using recursive descent
 *
 * Main functionality is based on grammar non-terminal symbols, where each non terminal has it's own
 * fnciton each function checks series of tokens from lexical analyzer and desides if terminal symbols
 * are correct under non-terminal wich function reprezents.
 * This will checks correct syntax of input program and simutalates TO-DOWN analysis.
 * Also some non-terminals have some sematic meaning for exaplme conditins or loops, so
 * Some control code is generated and symbols in sybol table are managed.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    16.11.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "appErr.h"
#include "utils.h"
#include "Scanner.h"
#include "symtable.h"
#include "grammar.h"
#include "rParser.h"
#include "syntaxAnalyzer.h"
#include "stacks.h"

// =============================================================================
// ====================== Supportive function macros  ==========================
// =============================================================================

#define NEXT_TOKEN(T) *T = scan_GetNextToken()
#define CHECK_TOKEN(T, S) if (T->type != S) scan_raiseCodeError(syntaxErr, "\"S\" token expected")
#define NEXT_CHECK_TOKEN(T, S) NEXT_TOKEN(T); CHECK_TOKEN(T, S)


// =============================================================================
// ========== Declaration of recursive Grammar NON-terminal functions ==========
// =============================================================================
void ck_NT_PROG(SToken *actToken); 					// Program - staritng non-terminal
void ck_NT_DD(SToken *actToken); 						// definitions and declarations section
void ck_NT_ASSINGEXT(                       // Assignement (...  [as datatype])
  SToken *actToken,
  const char *frame,
  const char *ident,
  DataType dataType);
void ck_NT_SCOPE(SToken *actToken); 				// Scope statement where local variables can be owerriten.
void ck_NT_PARAM_LIST(                      // list of parameters for function
  SToken *actToken,
  TArgList parList);
void ck_NT_PARAM( 				                  // one or more parameters
  SToken *actToken,
  TArgList parList);
void ck_NT_PARAM_EXT(SToken *actToken); 		// continue of param list
void ck_NT_STAT_LIST(SToken *actToken); 		// list of statements
void ck_NT_STAT(SToken *actToken); 					// one statement
void ck_NT_DOIN(SToken *actToken); 					// body of do..loop statement
void ck_NT_DOIN_WU(SToken *actToken); 			// until or while neterminal
void ck_NT_FORSTEP(SToken *actToken); 			// step of for
void ck_NT_INIF_EXT(SToken *actToken); 			// extension of body of if statement
void ck_NT_EXPR_LIST(SToken *actToken); 		// list of expression for print function
void ck_NT_EXPR(SToken *actToken); 					// one expresion
void ck_NT_ARGUMENT_LIS(SToken *actToken); 	// list of expression separated by comma


// =============================================================================
// ========================== Support functions  ===============================
// =============================================================================

// function for defining function
// 3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
void checkFunctionDefinition(SToken *actToken)
{
  TSymbol actSymbol = NULL;
  bool isDeclared = false;
  DataType retType = dtUnspecified;
  TArgList parList = TArgList_create();

  NEXT_CHECK_TOKEN(actToken, kwFunction);
  NEXT_CHECK_TOKEN(actToken, ident);

  // load ident to symbtable
  actSymbol = actToken->symbol;
  if (actSymbol->type == symtUnknown)
  {
    actSymbol->type = symtFuction;
    actSymbol->data.funcData.label = util_StrHardCopy(actSymbol->ident); // TODO: prefix ?
  }
  else if (actSymbol->type == symtFuction)
  {
    isDeclared = true;
    if (actSymbol->data.funcData.isDefined) // redefinition is not allowed
      scan_raiseCodeError(semanticErr, "Redefinition of function.");
  }
  else // attempt of redeclaration
    scan_raiseCodeError(semanticErr, "Symbol is defined as different type.");

  NEXT_CHECK_TOKEN(actToken, opLeftBrc);
  NEXT_TOKEN(actToken);
  // checks params of function
  ck_NT_PARAM_LIST(actToken, parList);
  CHECK_TOKEN(actToken, opRightBrc); // param list ends on right bracket
  NEXT_CHECK_TOKEN(actToken, kwAs);
  NEXT_CHECK_TOKEN(actToken, dataType);
  retType = actToken->dataType; // token is return type
  NEXT_CHECK_TOKEN(actToken, eol);

  if (isDeclared)
  {
    // check if definition responds to declaration
    if (!(parList->equals(parList, actSymbol->data.funcData.arguments)) ||
        retType != actSymbol->data.funcData.returnType)
      scan_raiseCodeError(semanticErr, "Function definition does not match with declaration.");

    // TODO: specify error ... ?

    TArgList_destroy(parList); // parameters aleready exists
  }
  else
  {
    actSymbol->data.funcData.returnType = retType;
    actSymbol->data.funcData.arguments = parList;
  }
  actSymbol->data.funcData.isDefined = true;

  // body of function
  symbt_pushFrame(false); // lets create local variable frame for function
  // fill frame with argument symbols
  for (int i = 0; i < parList->count; i++)
  {
    TArgument actArg = parList->get(parList, i);
    TSymbol symb = symbt_insertSymbOnTop(actArg->ident);
    symb->type = symtVariable;
    symb->dataType = actArg->dataType;
  }
  printf("LABEL %s\n", actSymbol->data.funcData.label);
  printf("PUSHFRAME\n");
  printf("DEFVAR LF@%%retval\n");
  NEXT_TOKEN(actToken);
  ck_NT_STAT_LIST(actToken);
  CHECK_TOKEN(actToken, kwEnd); // statement list must ends on end key word
  NEXT_CHECK_TOKEN(actToken, kwFunction);
  NEXT_CHECK_TOKEN(actToken, eol);
  printf("LABEL %s$epilog\n", actSymbol->data.funcData.label);
  printf("POPFRAME\n");
  printf("RETURN\n");
  symbt_popFrame();
}

// =============================================================================
// ========== Definition of recursive Grammar NON-terminal functions ===========
// =============================================================================

/**
 * All functions returns last token on wich they ended and is not part of thein monterminal structure
 * And all fuctions  taken first token as parameter on wich they suppose to start.
 */

// Program - staritng non-terminal
void ck_NT_PROG(SToken *actToken)
{
  // 1. NT_PROG -> NT_DD NT_SCOPE eof
  ck_NT_DD(actToken);
  ck_NT_SCOPE(actToken);
  CHECK_TOKEN(actToken, eof);
}

// definitions and declarations section
// first(NT_DD) = { kwDeclare -> (2); kwFunction -> (3);  kwStatic -> (4); else -> (5 [epsilon]) }
void ck_NT_DD(SToken *actToken)
{
  TSymbol actSymbol = NULL;
  switch (actToken->type)
  {
    // 2. NT_DD -> kwDeclare kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_DD
    case kwDeclare:
      NEXT_CHECK_TOKEN(actToken, kwFunction);
      NEXT_CHECK_TOKEN(actToken, ident);

      // load ident to symbtable
      actSymbol = actToken->symbol;
      if (actSymbol->type == symtUnknown)
        actSymbol->type = symtFuction;
      else // attempt of redeclaration
        scan_raiseCodeError(semanticErr, "Redeclaration of function is not allowed.");

      actSymbol->data.funcData.label = util_StrHardCopy(actSymbol->ident); // TODO: prefix ?
      actSymbol->data.funcData.isDefined = false;

      NEXT_CHECK_TOKEN(actToken, opLeftBrc);
      NEXT_TOKEN(actToken);
      // checks params of function
      TArgList parList = TArgList_create();
      ck_NT_PARAM_LIST(actToken, parList); // params are filled
      CHECK_TOKEN(actToken, opRightBrc); // param list ends on right bracket
      actSymbol->data.funcData.arguments = parList;
      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      actSymbol->data.funcData.returnType = actToken->dataType; // token is return type
      NEXT_CHECK_TOKEN(actToken, eol);
      ck_NT_DD(actToken);
      break;
    // 3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
    case kwFunction:
      checkFunctionDefinition(actToken); // too long to be here
      ck_NT_DD(actToken);
      break;
    // 4. NT_DD -> kwStatic kwShared ident kwAs dataType NT_ASSINGEXT eol NT_DD
    case kwStatic:
      NEXT_CHECK_TOKEN(actToken, kwShared);
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      if (actSymbol->type == symtUnknown)
        actSymbol->type = symtVariable;
      else
        scan_raiseCodeError(semanticErr, "Redefinition of static variable is not allowed.");
      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      actSymbol->dataType = actToken->dataType;
      printf("DEFVAR GF@%s\n", actSymbol->ident);
      NEXT_TOKEN(actToken);
      ck_NT_ASSINGEXT(actToken, "GL", actSymbol->ident, actToken->dataType);
      NEXT_CHECK_TOKEN(actToken, eol);
      ck_NT_DD(actToken);
      break;
    // 5. NT_DD -> (epsilon)
    default:
      // let it be
      break;
  }
}

// Assignement (...  [as datatype])
// first(NT_ASSINGEXT) = { asgn -> (6); else -> (7 [epsilon]) }
void ck_NT_ASSINGEXT(SToken *actToken, const char *frame, const char *ident, DataType dataType)
{
  switch (actToken->type)
  {
    // 6. NT_ASSINGEXT -> asgn NT_EXPR
    case asng:
      NEXT_CHECK_TOKEN(actToken, asng);
      NEXT_TOKEN(actToken);
      syntx_processExpression(actToken, frame, ident, dataType);
      break;
    // 7. NT_ASSINGEXT -> (epsilon)
    default:
      // let it be
      break;
  }
}

// Scope statement where local variables can be owerriten.
// first(NT_SCOPE) = { kwScope -> (8); else -> (error) }
void ck_NT_SCOPE(SToken *actToken)
{
  switch (actToken->type)
  {
    // 8. NT_SCOPE -> kwScope NT_STAT_LIST kwEnd kwScope eol
    case kwScope:
      symbt_pushFrame(true);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      CHECK_TOKEN(actToken, kwEnd); // statement list must ends on end key word
      NEXT_CHECK_TOKEN(actToken, kwScope);
      NEXT_CHECK_TOKEN(actToken, eol);
      symbt_popFrame();
      break;
    default:
      scan_raiseCodeError(syntaxErr, "\"scope\" token expected.");
      break;
  }
}

// list of parameters for function
// first(NT_PARAM_LIST) = { first(NT_PARAM) -> (9); else -> (10 [epsilon]) }
void ck_NT_PARAM_LIST(SToken *actToken, TArgList parList)
{
  switch (actToken->type)
  {
    // 9.  NT_PARAM_LIST -> NT_PARAM
    case ident:
      ck_NT_PARAM(actToken, parList);
      break;
    // 10. NT_PARAM_LIST -> (epsilon)
    default:
      // let it be
      break;
  }
}

// one or more parameters
// first(NT_PARAM) = { ident -> (11); else -> (error) }
void ck_NT_PARAM(SToken *actToken, TArgList parList)
{
  char *id;
  DataType dt;
  switch (actToken->type)
  {
    // 9.  NT_PARAM_LIST -> NT_PARAM
    case ident:
      id = actToken->symbol->ident;
      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      dt = actToken->dataType;
      parList->insert(id, dt);
      NEXT_TOKEN(actToken);
      ck_NT_PARAM_EXT(actToken, parList);
      break;
    // 11. NT_PARAM -> ident kwAs dataType NT_PARAM_EXT
    default:
      scan_raiseCodeError("identifier token expected.");
      break;
  }
}

// continue of param list
// first(NT_PARAM_EXT) = { opComma -> (12); else -> (13 [epsilon]) }
void ck_NT_PARAM_EXT(SToken *actToken, TArgList parList)
{
  switch (actToken->type)
  {
    // 12. NT_PARAM_EXT -> opComma NT_PARAM
    case opComma:
      NEXT_TOKEN(actToken);
      ck_NT_PARAM(actToken, parList);
      break;
    // 13. NT_PARAM_EXT -> (epsilon)
    default:
      // let it be
      break;
  }
}

// list of statements
// first(NT_STAT_LIST) = { first(NT_STAT) -> (14); else -> (15 [epsilon]) }
void ck_NT_STAT_LIST(SToken *actToken)
{
  switch (actToken->type)
  {
    // 14. NT_STAT_LIST -> NT_STAT eol NT_STAT_LIST
    case kwInput:
    case kwPrint:
    case kwIf:
    case kwDim:
    case ident:
    case kwContinue:
    case kwExit:
    case kwScope:
    case kwReturn:
    case kwDo:
    case kwFor:
      ck_NT_STAT(actToken);
      NEXT_CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      break;
    // 15. NT_STAT_LIST -> (epsilon)
    default:
      // let it be
      break;
  }
}

// one statement
// first(NT_STAT) = {
//   kwInput -> (16);
//   kwPrint -> (17);
//   kwIf -> (18);
//   kwDim -> (19);
//   ident -> (20);
//   kwContinue -> (21);
//   kwExit -> (22);
//   first(NT_SCOPE) -> (23);
//   kwReturn -> (24);
//   kwDo -> (25);
//   kwFor -> (26);
//   else -> (error) }
void ck_NT_STAT(SToken *actToken)
{
  (void)actToken;
  // 16. NT_STAT -> kwInput ident
  // 17. NT_STAT -> kwPrint NT_EXPR_LIST
  // 18. NT_STAT -> kwIf NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT kwEnd kwIf eol
  // 19. NT_STAT -> kwDim iden kwAs dataType NT_ASSINGEXT
  // 20. NT_STAT -> ident asng NT_EXPR
  // 21. NT_STAT -> kwContinue
  // 22. NT_STAT -> kwExit
  // 23. NT_STAT -> NT_SCOPE
  // 24. NT_STAT -> kwReturn NT_EXPR
  // 25. NT_STAT -> kwDo NT_DOIN kwLoop
  // 26. NT_STAT -> kwFor ident NT_ASSINGEXT kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext
}

// body of do..loop statement
// first(NT_DOIN) = { first(NT_DOIN_WU) -> (27); eol -> (28) else -> (error)}
void ck_NT_DOIN(SToken *actToken)
{
  (void)actToken;
  // 27. NT_DOIN -> NT_DOIN_WU NT_EXPR eol NT_STAT_LIST
  // 28. NT_DOIN -> eol NT_STAT_LIST NT_DOIN_WU NT_EXPR
}

// until or while neterminal
// first(NT_DOIN_WU) = { kwWhile -> (29); kwUntil -> (30); else -> (error) }
void ck_NT_DOIN_WU(SToken *actToken)
{
  (void)actToken;
  // 29. NT_DOIN_WU -> kwWhile
  // 30. NT_DOIN_WU -> kwUntil
}

// step of for
// first(NT_FORSTEP) = { kwStep -> (31); else -> (32 [epsilon]) }
void ck_NT_FORSTEP(SToken *actToken)
{
  (void)actToken;
  // 31. NT_FORSTEP -> kwStep NT_EXPR
  // 32. NT_FORSTEP -> (epsilon)
}

// extension of body of if statement
// first(NT_INIF_EXT) = { kwElsif -> (33); kwElse -> (34); else -> (35 [epsilon]) }
void ck_NT_INIF_EXT(SToken *actToken)
{
  (void)actToken;
  // 33. NT_INIF_EXT -> kwElsif NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT
  // 34. NT_INIF_EXT -> kwElse eol NT_STAT_LIST
  // 35. NT_INIF_EXT -> (epsilon)
}

// list of expression for print function
// first(NT_EXPR_LIST) = { first(NT_EXPR) -> (36); else -> (37 [epsilon]) }
void ck_NT_EXPR_LIST(SToken *actToken)
{
  (void)actToken;
  // 36. NT_EXPR_LIST -> NT_EXPR opSemcol NT_EXPR_LIST
  // 37. NT_EXPR_LIST -> (epsilon)
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

void rparser_processProgram()
{
  printf(".IFJcode17\n");
  SToken token = scan_GetNextToken();
  ck_NT_PROG(&token);
}