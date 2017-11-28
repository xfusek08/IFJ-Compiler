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
#include "scanner.h"
#include "MMng.h"
#include "syntaxAnalyzer.h"
#include "erpxSemanticAnalyzer.h"
#include "utils.h"

void raiseUnexpToken(SToken *actToken, EGrSymb expected);

// =============================================================================
// ====================== Supportive function macros  ==========================
// =============================================================================

#define NEXT_TOKEN(T) *T = scan_GetNextToken()
#define CHECK_TOKEN(T, S) if (T->type != S) raiseUnexpToken(T, S)
#define NEXT_CHECK_TOKEN(T, S) NEXT_TOKEN(T); CHECK_TOKEN(T, S)

// semantic errors
// on second definition of function
#define ERR_FUNC_REDEF() scan_raiseCodeError(semanticErr, "Redefinition of function is not allowed.")
// when defining symbol and it is already defined
#define ERR_SYMB_REDEF() scan_raiseCodeError(semanticErr, "Redefinition of defined symbol is not allowed.")
// unexpected token
#define ERR_UNEXP_TOKEN() scan_raiseCodeError(syntaxErr, "Uexpected token.")
// result of condition is not bool
#define ERR_COND_TYPE() scan_raiseCodeError(semanticErr, "Condition expression does not return boolean value.");

// =============================================================================
// ========================== Global variables =================================
// =============================================================================

unsigned int GLBFrameCnt = 0;

// =============================================================================
// ========== Declaration of recursive Grammar NON-terminal functions ==========
// =============================================================================
void ck_NT_PROG(SToken *actToken); 					// Program - staritng non-terminal
void ck_NT_DD(SToken *actToken); 						// definitions and declarations section
void ck_NT_ASSINGEXT(                       // Assignement (...  [as datatype])
  SToken *actToken,
  TSymbol symbol);
void ck_NT_SCOPE(SToken *actToken); 				// Scope statement where local variables can be owerriten.
void ck_NT_PARAM_LIST(                      // list of parameters for function
  SToken *actToken,
  TArgList parList);
void ck_NT_PARAM( 				                  // one or more parameters
  SToken *actToken,
  TArgList parList);
void ck_NT_PARAM_EXT( 		                  // continue of param list
  SToken *actToken,
  TArgList parList);
void ck_NT_STAT_LIST(SToken *actToken); 		// list of statements
void ck_NT_STAT(SToken *actToken); 					// one statement
void ck_NT_DOIN(SToken *actToken); 					// body of do..loop statement
void ck_NT_DOIN_WU(SToken *actToken); 			// until or while neterminal
TSymbol ck_NT_FORSTEP(SToken *actToken); 	  // step of for
void ck_NT_INIF_EXT(SToken *actToken); 			// extension of body of if statement
void ck_NT_EXPR_LIST(SToken *actToken); 		// list of expression for print function
void ck_NT_EXPR(SToken *actToken); 					// one expresion
void ck_NT_ARGUMENT_LIS(SToken *actToken); 	// list of expression separated by comma


// =============================================================================
// ========================== Support functions  ===============================
// =============================================================================

// adds frame as prefix to symbol identifier
void addPrefixToSymbolIdent(const char *prefix, TSymbol symbol)
{
  char *preident = symbol->ident;
  symbol->ident = util_StrConcatenate(prefix, symbol->ident);
  mmng_safeFree(preident);
}

void printSymbolToOperand(TSymbol symbol)
{
  if (symbol->type == symtConstant)
  {
    switch (symbol->dataType)
    {
      case dtInt: printf("int@%d", symbol->data.intVal); break;
      case dtFloat: printf("float@%g", symbol->data.doubleVal); break;
      case dtString: printf("string@%s", symbol->data.stringVal); break;
      case dtBool: printf("bool@%s", (symbol->data.boolVal) ? "true" : "false"); break;
      default:
        apperr_runtimeError("Invalid symbol data type. (internal structure error)");
      break;
    }
  }
  else if (symbol->type == symtVariable)
    printf("%s", symbol->ident);
}

// function for defining function
// 3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
void processFunction(SToken *actToken)
{
  TSymbol actSymbol = NULL;
  bool isDeclared = false;
  DataType retType = dtUnspecified;
  TArgList parList = TArgList_create();

  CHECK_TOKEN(actToken, kwFunction);
  NEXT_CHECK_TOKEN(actToken, ident);

  // load ident to symbtable
  actSymbol = actToken->symbol;
  if (actSymbol->type == symtUnknown)
  {
    actSymbol->type = symtFuction;
    actSymbol->data.funcData.label = util_StrConcatenate("$", actSymbol->ident);
  }
  else if (actSymbol->type == symtFuction)
  {
    isDeclared = true;
    if (actSymbol->data.funcData.isDefined) // redefinition is not allowed
      ERR_FUNC_REDEF();
  }
  else // attempt of redeclaration
    ERR_SYMB_REDEF();

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
  parList = actSymbol->data.funcData.arguments;

  // body of function
  symbt_pushFrame(actSymbol->data.funcData.label, false, false); // lets create local variable frame for function

  // define return variable and params in symbol table
  TSymbol tmpSymb = symbt_insertSymbOnTop("%retval");
  tmpSymb->type = symtVariable;
  tmpSymb->dataType = actSymbol->data.funcData.returnType;
  addPrefixToSymbolIdent("LF@", tmpSymb);

  // fill frame with argument symbols
  for (int i = 0; i < parList->count; i++)
  {
    TArgument actArg = parList->get(parList, i);
    tmpSymb = symbt_insertSymbOnTop(actArg->ident);
    addPrefixToSymbolIdent("LF@", tmpSymb);
    tmpSymb->type = symtVariable;
    tmpSymb->dataType = actArg->dataType;
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

void writeExpression(SToken *actToken)
{
  if (actToken->type == ident || actToken->type == opLeftBrc) // NT_EXPR
  {
    // evaluate expression
    //printf("token %s\n", grammarToString(actToken->type));
    //symbt_printSymb(actToken->symbol);
    TSymbol actSymbol = syntx_processExpression(actToken, NULL);
    symbt_printSymb(actSymbol);
    printf("WRITE ");
    printSymbolToOperand(actSymbol);
    printf("\n");
  }
  else
    scan_raiseCodeError(semanticErr, "Expression expected.");
}

void raiseUnexpToken(SToken *actToken, EGrSymb expected)
{
  char *message = mmng_safeMalloc(sizeof (char) * 100);
  sprintf(message, "\"%s\" token expected, \"%s\" got.", grammarToString(expected), grammarToString((*actToken).type));
  scan_raiseCodeError(syntaxErr, message);
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
  printf("LABEL %s\n", symbt_getActFuncLabel()); // main function
  ck_NT_SCOPE(actToken);
  if (actToken->type != eof)
  {
    CHECK_TOKEN(actToken, eol);
    NEXT_CHECK_TOKEN(actToken, eof);
  }
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
      NEXT_TOKEN(actToken);
      ck_NT_DD(actToken);
      break;
    // 3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
    case kwFunction:
      processFunction(actToken); // too long to be here
      NEXT_TOKEN(actToken);
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
        ERR_SYMB_REDEF();

      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      actSymbol->dataType = actToken->dataType;
      // add frame fo identifier
      addPrefixToSymbolIdent("GF@", actSymbol);
      printf("DEFVAR %s\n", actSymbol->ident);
      NEXT_TOKEN(actToken);
      ck_NT_ASSINGEXT(actToken, actSymbol);
      CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
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
void ck_NT_ASSINGEXT(SToken *actToken, TSymbol symbol)
{
  switch (actToken->type)
  {
    // 6. NT_ASSINGEXT -> asgn NT_EXPR
    case opEq:
      NEXT_TOKEN(actToken);
      // result will be stored in symbol
      syntx_processExpression(actToken, symbol);
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
      NEXT_CHECK_TOKEN(actToken, eol);
      char *label = symbt_getNewLocalLabel();
      symbt_pushFrame(label, true, false);
      mmng_safeFree(label);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);

      CHECK_TOKEN(actToken, kwEnd); // statement list must ends on end key word
      NEXT_CHECK_TOKEN(actToken, kwScope);
      NEXT_TOKEN(actToken);
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
    // 11. NT_PARAM -> ident kwAs dataType NT_PARAM_EXT
    case ident:
      id = actToken->symbol->ident;
      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      dt = actToken->dataType;
      parList->insert(parList, id, dt);
      NEXT_TOKEN(actToken);
      ck_NT_PARAM_EXT(actToken, parList);
      break;
    default:
      scan_raiseCodeError(syntaxErr, "Identifier token expected.");
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
      CHECK_TOKEN(actToken, eol);
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
  TSymbol actSymbol = NULL;
  switch (actToken->type)
  {
    // 16. NT_STAT -> kwInput ident
    case kwInput:
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      if (actSymbol->type != symtVariable)
        scan_raiseCodeError(semanticErr, "Symbol is not defined variable.");
      printf("READ %s %s\n", actSymbol->ident, util_dataTypeToString(actSymbol->dataType));
      NEXT_TOKEN(actToken);
      break;
    // 17. NT_STAT -> kwPrint NT_EXPR opSemcol NT_EXPR_LIST
    case kwPrint:
      NEXT_TOKEN(actToken);
      writeExpression(actToken);
      NEXT_CHECK_TOKEN(actToken, opSemcol);
      NEXT_TOKEN(actToken);
      ck_NT_EXPR_LIST(actToken);
      break;
    // 18. NT_STAT -> kwIf NT_EXPR kwThen eol NT_STAT_LIST NT_INIF_EXT kwEnd kwIf
    case kwIf:
      NEXT_TOKEN(actToken);
      TSymbol symbol = syntx_processExpression(actToken, NULL);
      if (symbol->dataType != dtBool)
        ERR_COND_TYPE();

      char *iflabel = symbt_getNewLocalLabel();
      CHECK_TOKEN(actToken, kwThen);
      NEXT_CHECK_TOKEN(actToken, eol);
      printf("JUMPIFNEQ %s$else %s bool@true\n", iflabel, symbol->ident);
      symbt_pushFrame(iflabel, true, false);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      printf("JUMP %s$endif\n", iflabel);
      printf("LABEL %s$else\n", iflabel);
      ck_NT_INIF_EXT(actToken);
      CHECK_TOKEN(actToken, kwEnd);
      NEXT_CHECK_TOKEN(actToken, kwIf);
      printf("LABEL %s$endif\n", iflabel);
      symbt_popFrame();
      mmng_safeFree(iflabel);
      NEXT_TOKEN(actToken);
      break;
    // 19. NT_STAT -> kwDim ident kwAs dataType NT_ASSINGEXT
    case kwDim:
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      if (actSymbol->type == symtUnknown)
        actSymbol->type = symtVariable;
      else
        ERR_SYMB_REDEF();

      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      actSymbol->dataType = actToken->dataType;
      // add frame fo identifier
      addPrefixToSymbolIdent("LF@", actSymbol);
      printf("DEFVAR %s\n", actSymbol->ident);
      NEXT_TOKEN(actToken);
      ck_NT_ASSINGEXT(actToken, actSymbol);
      break;
    // 20. NT_STAT -> ident opEq NT_EXPR // plus detection of function calling
    case ident:
      actSymbol = actToken->symbol;
      SToken leftOperand;
      leftOperand.type = ident;
      leftOperand.symbol = actSymbol;
      NEXT_TOKEN(actToken);
      SToken tokAsgn;
      if (actToken->type == opEq ||
          actToken->type == opPlusEq ||
          actToken->type == opMnsEq ||
          actToken->type == opMulEq ||
          actToken->type == opDivEq ||
          actToken->type == opDivFltEq
      )
        tokAsgn.type = (actToken->type == opEq) ? asgn : actToken->type;
      else
        scan_raiseCodeError(syntaxErr, "Assignment token expected.");

      NEXT_TOKEN(actToken);
      SToken rightOperand;
      rightOperand.type = ident;
      rightOperand.symbol = syntx_processExpression(actToken, NULL);
      syntx_generateCode(&leftOperand, &tokAsgn, &rightOperand, NULL);
      break;
    // 21. NT_STAT -> kwContinue
    case kwContinue:
      if (symbt_getActLoopLabel() != NULL)
        printf("JUMP %s$loop", symbt_getActLoopLabel());
      else
        scan_raiseCodeError(semanticErr, "\"Continue\" command can be used only in loop.");
      break;
    // 22. NT_STAT -> kwExit
    case kwExit:
      if (symbt_getActLoopLabel() != NULL)
        printf("JUMP %s$loopend", symbt_getActLoopLabel());
      else
        scan_raiseCodeError(semanticErr, "\"Exit\" command can be used only in loop.");
      break;
    // 23. NT_STAT -> NT_SCOPE
    case kwScope:
      ck_NT_SCOPE(actToken);
      break;
    // 24. NT_STAT -> kwReturn NT_EXPR
    case kwReturn:
      if (symbt_cntFuncFrames() > 1)
      {
        NEXT_TOKEN(actToken);
        TSymbol symbol = symbt_findOrInsertSymb("%retval");
        syntx_processExpression(actToken, symbol);
        printf("JUMP %s$epilog\n", symbt_getActFuncLabel());
      }
      else
        scan_raiseCodeError(anotherSemanticErr, "Return can not be used outside of function.");
      break;
    // 25. NT_STAT -> kwDo NT_DOIN kwLoop
    case kwDo:
      NEXT_TOKEN(actToken);
      ck_NT_DOIN(actToken);
      break;
    // 26. NT_STAT -> kwFor ident [as datatype] NT_ASSINGEXT kwTo NT_EXPR NT_STEP eol NT_STAT_LIST kwNext
    case kwFor:
      (void)0;
      char *forlabel = symbt_getNewLocalLabel();
      symbt_pushFrame(forlabel, true, true);
      // iter variable
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      NEXT_TOKEN(actToken);
      // [as datatype]
      if (actToken->type == kwAs)
      {
        // check if symbol needs to be redefined
        if (actSymbol->type != symtUnknown)
          actSymbol = symbt_insertSymbOnTop(actSymbol->key);

        addPrefixToSymbolIdent("$", actSymbol);
        addPrefixToSymbolIdent(forlabel, actSymbol);
        addPrefixToSymbolIdent("LF@", actSymbol);
        printf("DEFVAR %s\n", actSymbol->ident);
        actSymbol->type = symtVariable;
        NEXT_CHECK_TOKEN(actToken, dataType);
        actSymbol->dataType = actToken->dataType;
        NEXT_TOKEN(actToken);
      }
      if (actSymbol->dataType != dtInt && actSymbol->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "Iterator value has no valid data type. Only double or integer is allowed.");

      // if its =
      CHECK_TOKEN(actToken, opEq);
      NEXT_TOKEN(actToken);
      syntx_processExpression(actToken, actSymbol);
      CHECK_TOKEN(actToken, kwTo);
      NEXT_TOKEN(actToken);

      // set TO value
      TSymbol toSymb = syntx_processExpression(actToken, NULL);
      if (toSymb->dataType != dtInt && toSymb->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "To value has no valid data type. Only double or integer is allowed.");

      // balance comparing data types
      if (actSymbol->dataType == dtInt && toSymb->dataType == dtFloat)
      {
        actSymbol->dataType = dtFloat;
        if (actSymbol->type == symtConstant) // is constant
          actSymbol->data.doubleVal = syntx_intToDouble(actSymbol->data.intVal);
        else // is variable
          printf("INT2FLOAT %s %s", actSymbol->ident, actSymbol->ident);
      }
      if (actSymbol->dataType == dtFloat && toSymb->dataType == dtInt)
      {
        toSymb->dataType = dtFloat;
        if (toSymb->type == symtConstant) // is constant
          toSymb->data.doubleVal = syntx_intToDouble(toSymb->data.intVal);
        else // is variable
          printf("INT2FLOAT %s %s", toSymb->ident, toSymb->ident);
      }

      // set STEP Value
      TSymbol stepSymb = ck_NT_FORSTEP(actToken);
      symbt_printSymb(stepSymb);
      if (stepSymb->dataType != dtInt && stepSymb->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "Step value has no valid data type. Only double or integer is allowed.");
      if (actSymbol->dataType == dtInt && stepSymb->dataType == dtFloat)
        scan_raiseCodeError(semanticErr, "Step value has incompatible datatype with iterator.");
      if (actSymbol->dataType == dtFloat && stepSymb->dataType == dtInt)
      {
        stepSymb->dataType = dtInt;
        if (stepSymb->type == symtConstant) // is constant
          stepSymb->data.intVal = syntx_doubleToInt(stepSymb->data.doubleVal);
        else // is variable
          printf("FLOAT2INT %s %s\n", stepSymb->ident, stepSymb->ident);
      }

      CHECK_TOKEN(actToken, eol);
      // end of for initialization

      NEXT_TOKEN(actToken);
      // check condition
      printf("DEFVAR LF@%%forisless\n");
      printf("LABEL %s$loop\n", forlabel);
      printf("GT LF@%%forisless %s ", actSymbol->ident);
      printSymbolToOperand(toSymb);
      printf("\n");
      printf("JUMPIFEQ %s$loopend LF@%%forisless bool@true\n", forlabel);

      // inner statements
      ck_NT_STAT_LIST(actToken);

      // increment iterator
      printf("ADD %s %s ", actSymbol->ident, actSymbol->ident);
      printSymbolToOperand(stepSymb);
      printf("\n");

      printf("JUMP %s$loop\n", forlabel);
      printf("LABEL %s$loopend\n", forlabel);
      CHECK_TOKEN(actToken, kwNext);
      NEXT_TOKEN(actToken);
      symbt_popFrame();
      mmng_safeFree(forlabel);
      break;
    // else -> error
    default:
      ERR_UNEXP_TOKEN();
      break;
  }
}

// body of do..loop statement
// first(NT_DOIN) = { first(NT_DOIN_WU) -> (27); eol -> (28) else -> (error)}
void ck_NT_DOIN(SToken *actToken)
{
  char *dolabel = symbt_getNewLocalLabel();
  TSymbol cond = NULL;
  bool isUntil = false;
  symbt_pushFrame(dolabel, true, true);
  printf("LABEL %s$loop\n", dolabel);
  switch (actToken->type)
  {
    // 27. NT_DOIN -> NT_DOIN_WU NT_EXPR eol NT_STAT_LIST
    case kwWhile:
    case kwUntil:
      ck_NT_DOIN_WU(actToken);
      isUntil = actToken->type == kwUntil;
      NEXT_TOKEN(actToken);
      cond = syntx_processExpression(actToken, NULL);
      if (cond->dataType != dtBool)
        ERR_COND_TYPE();

      if (isUntil)
        printf("JUMPIFEQ %s$loopend %s bool@true\n", dolabel, cond->ident);
      else
        printf("JUMPIFEQ %s$loopend %s bool@false\n", dolabel, cond->ident);

      CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      CHECK_TOKEN(actToken, kwLoop);
      NEXT_TOKEN(actToken);
      printf("JUMP %s$loop\n", dolabel);
      break;
    // 28. NT_DOIN -> eol NT_STAT_LIST NT_DOIN_WU NT_EXPR
    case eol:
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);

      ck_NT_DOIN_WU(actToken);
      isUntil = actToken->type == kwUntil;

      cond = syntx_processExpression(actToken, NULL);
      if (cond->dataType != dtBool)
        ERR_COND_TYPE();

      if (isUntil)
        printf("JUMPIFEQ %s$loop %s bool@false\n", dolabel, cond->ident);
      else
        printf("JUMPIFEQ %s$loop %s bool@true\n", dolabel, cond->ident);
      break;
    default:
      ERR_UNEXP_TOKEN();
      break;
  }
  printf("LABEL %s$loopend\n", dolabel);
  symbt_popFrame();
  mmng_safeFree(dolabel);
}

// until or while neterminal
// first(NT_DOIN_WU) = { kwWhile -> (29); kwUntil -> (30); else -> (error) }
void ck_NT_DOIN_WU(SToken *actToken)
{
  switch (actToken->type)
  {
    // 29. NT_DOIN_WU -> kwWhile
    // 30. NT_DOIN_WU -> kwUntil
    case kwWhile:
    case kwUntil:
      // ok
      break;
    default:
      ERR_UNEXP_TOKEN();
      break;
  }
}

// step of for
// first(NT_FORSTEP) = { kwStep -> (31); else -> (32 [epsilon]) }
TSymbol ck_NT_FORSTEP(SToken *actToken)
{
  TSymbol result = NULL;
  switch (actToken->type)
  {
    // 31. NT_FORSTEP -> kwStep NT_EXPR
    case kwStep:
      NEXT_TOKEN(actToken);
      result = syntx_processExpression(actToken, NULL);
      break;
    // 32. NT_FORSTEP -> (epsilon)
    default:
      result = symbt_insertSymbOnTop("%step");
      result->type = symtConstant;
      result->dataType = dtInt;
      result->data.intVal = 1;
      break;
  }
  return result;
}

// extension of body of if statement
// first(NT_INIF_EXT) = { kwElseif -> (33); kwElse -> (34); else -> (35 [epsilon]) }
void ck_NT_INIF_EXT(SToken *actToken)
{
  switch (actToken->type)
  {
    // 33. NT_INIF_EXT -> kwElseif NT_EXPR kwThen eol NT_STAT_LIST NT_INIF_EXT
    case kwElseif:
      NEXT_TOKEN(actToken);
      TSymbol symbol = syntx_processExpression(actToken, NULL);
      if (symbol->dataType != dtBool)
        ERR_COND_TYPE();

      char *iflabel = symbt_getNewLocalLabel();
      CHECK_TOKEN(actToken, kwThen);
      NEXT_CHECK_TOKEN(actToken, eol);
      printf("JUMPIFNEQ %s$else %s bool@true\n", iflabel, symbol->ident);
      NEXT_TOKEN(actToken);
      symbt_pushFrame(iflabel, true, false);
      ck_NT_STAT_LIST(actToken);
      symbt_popFrame();
      printf("JUMP %s$endif\n", symbt_getActLocalLabel());
      printf("LABEL %s$else\n", iflabel);
      mmng_safeFree(iflabel);
      ck_NT_INIF_EXT(actToken);
      break;
    // 34. NT_INIF_EXT -> kwElse eol NT_STAT_LIST
    case kwElse:
      NEXT_CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
      char *frameLabel = symbt_getNewLocalLabel();
      symbt_pushFrame(frameLabel, true, false);
      mmng_safeFree(frameLabel);
      ck_NT_STAT_LIST(actToken);
      symbt_popFrame();
      mmng_safeFree(frameLabel);
      break;
    // 35. NT_INIF_EXT -> (epsilon)
    default:
      // let it be
      break;
  }
}

// list of expression for print function
// first(NT_EXPR_LIST) = { first(NT_EXPR) -> (36); else -> (37 [epsilon]) }
void ck_NT_EXPR_LIST(SToken *actToken)
{
  switch (actToken->type)
  {
    // 36. NT_EXPR_LIST -> NT_EXPR opSemcol NT_EXPR_LIST
    case opLeftBrc:
    case ident:
      NEXT_TOKEN(actToken);
      writeExpression(actToken);
      CHECK_TOKEN(actToken, opSemcol);
      NEXT_TOKEN(actToken);
      ck_NT_EXPR_LIST(actToken);
      break;
    // 37. NT_EXPR_LIST -> (epsilon)
    default:
        // let if be
      break;
  }
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

void rparser_processProgram()
{
  // start compile to code
  printf(".IFJcode17\n");
  printf("JUMP %s\n", symbt_getActFuncLabel());
  SToken token = scan_GetNextToken();
  ck_NT_PROG(&token);
}