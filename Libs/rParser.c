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
#include <string.h>
#include "appErr.h"
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
// when defining symbol and it is already defined
#define ERR_SYMB_REDEF() scan_raiseCodeError(semanticErr, "Redefinition of defined symbol is not allowed.", actToken)
// unexpected token
#define ERR_UNEXP_TOKEN() scan_raiseCodeError(syntaxErr, "Uexpected token.", actToken)
// result of condition is not bool
#define ERR_COND_TYPE() scan_raiseCodeError(typeCompatibilityErr, "Condition expression does not return boolean value.", actToken);

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
void ck_NT_DOIN_WU(                         // until or while neterminal
  SToken *actToken,
  char *doLabel,
  bool isOnEnd);
TSymbol ck_NT_FORSTEP(SToken *actToken);    // step of for
void ck_NT_INIF_EXT(SToken *actToken); 			// extension of body of if statement
void ck_NT_PRINT_LIST(SToken *actToken); 		// list of expression for print function
void ck_NT_EXPR(SToken *actToken); 					// one expresion
void ck_NT_ARGUMENT_LIS(SToken *actToken); 	// list of expression separated by comma
char *ck_NT_CYCLE_NESTS(                    // extension of contuinue and exit
  SToken *actToken,
  bool isExit);
void ck_NT_CYCLES_DO(                       // list of do kws
  SToken *actToken,
  int *doCnt);
void ck_NT_CYCLES_FOR(                      // list of for kws
  SToken *actToken,
  int *forCnt);



// =============================================================================
// ========================== Support functions  ===============================
// =============================================================================

// adds frame as prefix to symbol identifier
void addPrefixToSymbolIdent(char *prefix, TSymbol symbol)
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
      case dtInt: printInstruction("int@%d", symbol->data.intVal); break;
      case dtFloat: printInstruction("float@%g", symbol->data.doubleVal); break;
      case dtString: printLongInstruction(strlen(symbol->data.stringVal), "string@%s", symbol->data.stringVal); break;
      case dtBool: printInstruction("bool@%s", (symbol->data.boolVal) ? "true" : "false"); break;
      default:
        apperr_runtimeError("Invalid symbol data type. (internal structure error)");
      break;
    }
  }
  else if (symbol->type == symtVariable)
    printInstruction("%s", symbol->ident);
}

void setDefautValue(char *varIdent, DataType dt, bool directPrint)
{
  char *typeconst;
  switch (dt)
  {
    case dtInt: typeconst = "int@0"; break;
    case dtFloat: typeconst = "float@0"; break;
    case dtString: typeconst = "string@"; break;
    case dtBool: typeconst = "bool@false"; break;
    default: return;
  }

  if (directPrint)
    printf("MOVE %s %s\n", varIdent, typeconst);
  else
    printInstruction("MOVE %s %s\n", varIdent, typeconst);
}

// balance numeric symbol types
void balanceNumTypes(TSymbol symb1, TSymbol symb2)
{
  if (symb1->dataType == dtInt && symb2->dataType == dtFloat)
  {
    symb2->dataType = dtInt;
    if (symb2->type == symtConstant) // is constant
      symb2->data.intVal = syntx_doubleToInt(symb1->data.doubleVal);
    else // is variable
      printInstruction("FLOAT2R2EINT %s %s\n", symb2->ident, symb2->ident);
  }
  else if (symb1->dataType == dtFloat && symb2->dataType == dtInt)
  {
    symb2->dataType = dtFloat;
    if (symb2->type == symtConstant) // is constant
      symb2->data.doubleVal = syntx_intToDouble(symb2->data.intVal);
    else // is variable
      printInstruction("INT2FLOAT %s %s\n", symb2->ident, symb2->ident);
  }
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

  if (util_isBuildInFunc(actSymbol->key))
    scan_raiseCodeError(syntaxErr, "Redefinition of build-in function is not allowed.", actToken);

  if (actSymbol->type == symtUnknown)
  {
    actSymbol->type = symtFuction;
    actSymbol->data.funcData.label = util_StrConcatenate("$", actSymbol->ident);
  }
  else if (actSymbol->type == symtFuction)
  {
    isDeclared = true;
    if (actSymbol->data.funcData.isDefined) // redefinition is not allowed
      scan_raiseCodeError(semanticErr, "Redefinition of function is not allowed.", actToken);
  }
  else // attempt of redeclaration
    ERR_SYMB_REDEF();

  // body of function
  symbt_pushFrame(actSymbol->data.funcData.label, false, false, false); // lets create local variable frame for function

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
      scan_raiseCodeError(semanticErr, "Function definition does not match with declaration.", actToken);
  }
  else
  {
    actSymbol->data.funcData.returnType = retType;
    actSymbol->data.funcData.arguments = parList;
  }
  actSymbol->data.funcData.isDefined = true;

  // define return variable and params in symbol table
  TSymbol tmpSymb = symbt_insertSymbOnTop("%retval");
  tmpSymb->type = symtVariable;
  tmpSymb->dataType = actSymbol->data.funcData.returnType;
  addPrefixToSymbolIdent("LF@", tmpSymb);

  // fill frame with argument symbols
  for (int i = 0; i < parList->count; i++)
  {
    // make sure that in definition of fucntion is definition argument identifier used as key to symb table
    TArgument definitionArg = parList->get(parList, i);
    tmpSymb = symbt_findSymb(definitionArg->ident);
    // if fuctions was already declared ve have to remap definition symbol identifiers to declaration symbol identifiers
    if (isDeclared)
    {
      TArgument declarationArg = actSymbol->data.funcData.arguments->get(actSymbol->data.funcData.arguments, i);
      char *preident = tmpSymb->ident;
      tmpSymb->ident = util_StrConcatenate("LF@", declarationArg->ident);
      mmng_safeFree(preident);
    }
    else
      addPrefixToSymbolIdent("LF@", tmpSymb);
    tmpSymb->type = symtVariable;
    tmpSymb->dataType = definitionArg->dataType;
  }

  // we destroy list of definition params because we need to use declaration identifiers in calls
  if (isDeclared) // if function was declared
    TArgList_destroy(parList);


  printf("LABEL %s\n", actSymbol->data.funcData.label);
  printf("PUSHFRAME\n");
  printf("DEFVAR LF@%%retval\n");
  setDefautValue("LF@%retval", actSymbol->data.funcData.returnType, true);
  NEXT_TOKEN(actToken);
  ck_NT_STAT_LIST(actToken);
  CHECK_TOKEN(actToken, kwEnd); // statement list must ends on end key word
  NEXT_CHECK_TOKEN(actToken, kwFunction);
  NEXT_CHECK_TOKEN(actToken, eol);
  printInstruction("LABEL %s$epilog\n", actSymbol->data.funcData.label);
  printInstruction("POPFRAME\n");
  printInstruction("RETURN\n");
  symbt_popFrame();
}

// definition or redefinition as variable
void defOrRedefVariable(TSymbol symbolVar)
{
  if (symbolVar->type == symtUnknown)
  {
    symbolVar->type = symtVariable;
    addPrefixToSymbolIdent("LF@", symbolVar);
    if (!symbt_isVarDefined(symbolVar->key))
    {
      symbt_defVarIdent(symbolVar->key);
      printf("DEFVAR %s\n", symbolVar->ident);
    }
  }
  else
  {
    if (symbolVar->type == symtVariable)
    {
      if (!symbt_pushRedefinition(symbolVar))
        scan_raiseCodeError(semanticErr, "Cannot redefine variable in the same scope.", NULL);
    }
    else if (symbolVar->type == symtConstant)
      scan_raiseCodeError(syntaxErr, "Cannot redefine constant as variable.", NULL);
    else // function remains
      scan_raiseCodeError(semanticErr, "Cannot redefine function as variable.", NULL);
  }
}

void raiseUnexpToken(SToken *actToken, EGrSymb expected)
{
  char *message = mmng_safeMalloc(sizeof (char) * 100);
  sprintf(message, "\"%s\" token expected, \"%s\" got.", grammarToString(expected), grammarToString((*actToken).type));
  scan_raiseCodeError(syntaxErr, message, actToken);
}

// definitions of symbols of build in functions
void defineBuildInFuncSymbols()
{
  TSymbol func = NULL;
  TArgList params = NULL;

  // Length
  func = symbt_findOrInsertSymb("length");
  func->type = symtFuction;
  func->data.funcData.label = util_StrHardCopy("$$Length");
  func->data.funcData.returnType = dtInt;
  func->data.funcData.isDefined = true;
  params = TArgList_create();
  params->insert(params, "p1", dtString);
  func->data.funcData.arguments = params;

  // Substr
  func = symbt_findOrInsertSymb("substr");
  func->type = symtFuction;
  func->data.funcData.label = util_StrHardCopy("$$SubStr");
  func->data.funcData.returnType = dtString;
  func->data.funcData.isDefined = true;
  params = TArgList_create();
  params->insert(params, "p1", dtString);
  params->insert(params, "p2", dtInt);
  params->insert(params, "p3", dtInt);
  func->data.funcData.arguments = params;

  // Asc
  func = symbt_findOrInsertSymb("asc");
  func->type = symtFuction;
  func->data.funcData.label = util_StrHardCopy("$$Asc");
  func->data.funcData.returnType = dtInt;
  func->data.funcData.isDefined = true;
  params = TArgList_create();
  params->insert(params, "p1", dtString);
  params->insert(params, "p2", dtInt);
  func->data.funcData.arguments = params;

  // Chr
  func = symbt_findOrInsertSymb("chr");
  func->type = symtFuction;
  func->data.funcData.label = util_StrHardCopy("$$Chr");
  func->data.funcData.returnType = dtString;
  func->data.funcData.isDefined = true;
  params = TArgList_create();
  params->insert(params, "p1", dtInt);
  func->data.funcData.arguments = params;
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
  printf("CREATEFRAME\n");
  printf("PUSHFRAME\n");
  printf("CREATEFRAME\n");

  // check if all declared functions are defined
  char *udenfFuncIdent = symbt_getUndefinedFunc();
  if (udenfFuncIdent != NULL)
  {
    char *message = util_StrConcatenate("Function \"", udenfFuncIdent);
    message = util_StrConcatenate(message, "\" is declared but never defined.");
    scan_raiseCodeError(semanticErr, message, actToken); // error clears memmory anyway
  }

  ck_NT_SCOPE(actToken);
  if (actToken->type != eof)
  {
    CHECK_TOKEN(actToken, eol);
    NEXT_CHECK_TOKEN(actToken, eof);
  }
}

// definitions and declarations section
// first(NT_DD) = { kwDeclare -> (2); kwFunction -> (3); else -> (epsilon) }
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

      if (util_isBuildInFunc(actSymbol->key))
        scan_raiseCodeError(syntaxErr, "Redeclaration of build-in function is not allowed.", actToken);

      if (actSymbol->type == symtUnknown)
        actSymbol->type = symtFuction;
      else // attempt of redeclaration
        scan_raiseCodeError(semanticErr, "Redeclaration of function is not allowed.", actToken);

      actSymbol->type = symtFuction;
      actSymbol->data.funcData.label = util_StrConcatenate("$", actSymbol->ident);
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
      // clean symbtable from arguments
      for (int i = 0; i < parList->count; i++)
        symbt_deleteSymb(parList->get(parList, i)->ident);
      ck_NT_DD(actToken);
      break;
    // 3. NT_DD -> kwFunction ident opLeftBrc NT_PARAM_LIST opRightBrc kwAs dataType eol NT_STAT_LIST kwEnd kwFunction eol NT_DD
    case kwFunction:
      processFunction(actToken); // too long to be here
      NEXT_TOKEN(actToken);
      ck_NT_DD(actToken);
      break;
    // NT_DD -> (epsilon)
    default:
      // let it be
      break;
  }
}

// Assignement (...  [as datatype])
// first(NT_ASSINGEXT) = { asgn -> (4); else -> (epsilon) }
void ck_NT_ASSINGEXT(SToken *actToken, TSymbol symbol)
{
  switch (actToken->type)
  {
    // 4. NT_ASSINGEXT -> asgn NT_EXPR
    case opEq:
      NEXT_TOKEN(actToken);
      // result will be stored in symbol
      syntx_processExpression(actToken, symbol);
      break;
    // NT_ASSINGEXT -> (epsilon)
    default:
      setDefautValue(symbol->ident, symbol->dataType, false);
      // let it be
      break;
  }
}

// Scope statement where local variables can be owerriten.
// first(NT_SCOPE) = { kwScope -> (5); else -> (error) }
void ck_NT_SCOPE(SToken *actToken)
{
  switch (actToken->type)
  {
    // 5. NT_SCOPE -> kwScope NT_STAT_LIST kwEnd kwScope eol
    case kwScope:
      NEXT_CHECK_TOKEN(actToken, eol);
      char *label = symbt_getNewLocalLabel();
      symbt_pushFrame(label, true, false, false);
      mmng_safeFree(label);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);

      CHECK_TOKEN(actToken, kwEnd); // statement list must ends on end key word
      NEXT_CHECK_TOKEN(actToken, kwScope);
      NEXT_TOKEN(actToken);
      symbt_popFrame();
      break;
    // else -> error
    default:
      scan_raiseCodeError(syntaxErr, "\"scope\" token expected.", actToken);
      break;
  }
}

// list of parameters for function
// first(NT_PARAM_LIST) = { first(NT_PARAM) -> (6); else -> (epsilon) }
void ck_NT_PARAM_LIST(SToken *actToken, TArgList parList)
{
  switch (actToken->type)
  {
    // 6.  NT_PARAM_LIST -> NT_PARAM
    case ident:
      ck_NT_PARAM(actToken, parList);
      break;
    // NT_PARAM_LIST -> (epsilon)
    default:
      // let it be
      break;
  }
}

// one or more parameters
// first(NT_PARAM) = { ident -> (7); else -> (error) }
void ck_NT_PARAM(SToken *actToken, TArgList parList)
{
  char *id;
  DataType dt;
  switch (actToken->type)
  {
    // 7. NT_PARAM -> ident kwAs dataType NT_PARAM_EXT
    case ident:
      if (actToken->symbol->type != symtUnknown)
        scan_raiseCodeError(semanticErr, "Symbol with this identifier already exists.", actToken);
      actToken->symbol->type = symtVariable;
      id = actToken->symbol->ident;
      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      dt = actToken->dataType;
      parList->insert(parList, id, dt);
      NEXT_TOKEN(actToken);
      ck_NT_PARAM_EXT(actToken, parList);
      break;
    // else -> error
    default:
      scan_raiseCodeError(syntaxErr, "Identifier token expected.", actToken);
      break;
  }
}

// continue of param list
// first(NT_PARAM_EXT) = { opComma -> (8); else -> (epsilon) }
void ck_NT_PARAM_EXT(SToken *actToken, TArgList parList)
{
  switch (actToken->type)
  {
    // 8. NT_PARAM_EXT -> opComma NT_PARAM
    case opComma:
      NEXT_TOKEN(actToken);
      ck_NT_PARAM(actToken, parList);
      break;
    // NT_PARAM_EXT -> (epsilon)
    default:
      // let it be
      break;
  }
}

// list of statements
// first(NT_STAT_LIST) = { first(NT_STAT) -> (9); else -> (epsilon) }
void ck_NT_STAT_LIST(SToken *actToken)
{
  switch (actToken->type)
  {
    // 9. NT_STAT_LIST -> NT_STAT eol NT_STAT_LIST
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
    // NT_STAT_LIST -> (epsilon)
    default:
      // let it be
      break;
  }
}

// one statement
// first(NT_STAT) = {
//   kwInput -> (10);
//   kwPrint -> (11);
//   kwIf -> (12);
//   kwDim -> (13);
//   ident -> (14);
//   kwContinue -> (15);
//   kwExit -> (16);
//   first(NT_SCOPE) -> (17);
//   kwReturn -> (18);
//   kwDo -> (19);
//   kwFor -> (20);
//   else -> (error) }
void ck_NT_STAT(SToken *actToken)
{
  TSymbol actSymbol = NULL;
  bool isExit = false; // used in continue and exit
  switch (actToken->type)
  {
    // 10. NT_STAT -> kwInput ident
    case kwInput:
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      if (actSymbol->type != symtVariable)
        scan_raiseCodeError(semanticErr, "Symbol is not defined variable.", actToken);
      printInstruction("WRITE string@?\\032\n");
      printInstruction("READ %s %s\n", actSymbol->ident, util_dataTypeToString(actSymbol->dataType));
      NEXT_TOKEN(actToken);
      break;
    // 11. NT_STAT -> kwPrint NT_PRINT_LIST
    case kwPrint:
      NEXT_TOKEN(actToken);
      ck_NT_PRINT_LIST(actToken);
      break;
    // 12. NT_STAT -> kwIf NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT kwEnd kwIf
    case kwIf:
      NEXT_TOKEN(actToken);
      char *iflabel = symbt_getNewLocalLabel();
      symbt_pushFrame(iflabel, true, false, false);
      TSymbol symbol = syntx_processExpression(actToken, NULL);
      if (symbol->dataType != dtBool)
        ERR_COND_TYPE();

      CHECK_TOKEN(actToken, kwThen);
      NEXT_CHECK_TOKEN(actToken, eol);
      printInstruction("JUMPIFNEQ %s$else %s bool@true\n", iflabel, symbol->ident);
      symbt_pushFrame(iflabel, true, false, false);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      symbt_popFrame();
      printInstruction("JUMP %s$endif\n", iflabel);
      printInstruction("LABEL %s$else\n", iflabel);
      symbt_pushFrame(iflabel, true, false, false);
      ck_NT_INIF_EXT(actToken);
      CHECK_TOKEN(actToken, kwEnd);
      NEXT_CHECK_TOKEN(actToken, kwIf);
      symbt_popFrame();
      printInstruction("LABEL %s$endif\n", iflabel);
      mmng_safeFree(iflabel);
      symbt_popFrame();
      NEXT_TOKEN(actToken);
      break;
    // 13. NT_STAT -> kwDim ident kwAs dataType NT_ASSINGEXT
    case kwDim:
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;

      if (util_isBuildInFunc(actSymbol->key))
        scan_raiseCodeError(syntaxErr, "Redefinition of build-in function is not allowed.", actToken);

      defOrRedefVariable(actSymbol);

      NEXT_CHECK_TOKEN(actToken, kwAs);
      NEXT_CHECK_TOKEN(actToken, dataType);
      actSymbol->type = symtVariable;
      actSymbol->dataType = actToken->dataType;
      NEXT_TOKEN(actToken);
      ck_NT_ASSINGEXT(actToken, actSymbol);
      break;
    // 14. NT_STAT -> ident opEq NT_EXPR (if ident is not function)
    // 14. NT_STAT -> NT_EXPR            (if line starts with function ident)
    case ident:
      actSymbol = actToken->symbol;
      if (actSymbol->type == symtFuction) // function call
        syntx_processExpression(actToken, NULL);
      else
      {
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
          scan_raiseCodeError(syntaxErr, "Assignment token expected.", actToken);

        NEXT_TOKEN(actToken);
        SToken rightOperand;
        rightOperand.type = ident;
        rightOperand.symbol = syntx_processExpression(actToken, NULL);

        syntx_generateCode(&leftOperand, &tokAsgn, &rightOperand, NULL);
      }
      break;
    // 15. NT_STAT -> kwContinue NT_CYCLE_NESTS
    // 16. NT_STAT -> kwExit NT_CYCLE_NESTS
    case kwContinue:
    case kwExit:
      isExit = actToken->type == kwExit;
      NEXT_TOKEN(actToken);
      char *label = ck_NT_CYCLE_NESTS(actToken, isExit);
      if (label != NULL)
        printInstruction("JUMP %s\n", label);
      else
      {
        if (symbt_getNthDoLoopLabel(0) == NULL && symbt_getNthForLoopLabel(0) == NULL) // not in cycle at all
          scan_raiseCodeError(semanticErr, "\"Continue\" and \"Exit\" commands can be used only in loop.", actToken);
        else
          scan_raiseCodeError(semanticErr, "Cannot find coressponding loop.", actToken);
      }
      mmng_safeFree(label);
      break;
    // 17. NT_STAT -> NT_SCOPE
    case kwScope:
      ck_NT_SCOPE(actToken);
      break;
    // 18. NT_STAT -> kwReturn NT_EXPR
    case kwReturn:
      if (symbt_cntFuncFrames() > 1)
      {
        NEXT_TOKEN(actToken);
        TSymbol symbol = symbt_findOrInsertSymb("%retval");
        syntx_processExpression(actToken, symbol);
        printInstruction("JUMP %s$epilog\n", symbt_getActFuncLabel());
      }
      else
        scan_raiseCodeError(syntaxErr, "Return can not be used outside of function.", actToken);
      break;
    // 19. NT_STAT -> kwDo NT_DOIN
    case kwDo:
      NEXT_TOKEN(actToken);
      ck_NT_DOIN(actToken);
      break;
    // 20. NT_STAT -> kwFor ident [as datatype] NT_ASSINGEXT kwTo NT_EXPR NT_FORSTEP eol NT_STAT_LIST kwNext [ident]
    case kwFor:
      // iter variable
      NEXT_CHECK_TOKEN(actToken, ident);
      actSymbol = actToken->symbol;
      NEXT_TOKEN(actToken);

      char *forlabel = symbt_getNewLocalLabel();
      symbt_pushFrame(forlabel, true, false, false);
      // [as datatype]
      if (actToken->type == kwAs)
      {
        defOrRedefVariable(actSymbol);
        NEXT_CHECK_TOKEN(actToken, dataType);
        actSymbol->dataType = actToken->dataType;
        NEXT_TOKEN(actToken);
      }
      if (actSymbol->dataType != dtInt && actSymbol->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "Iterator value has no valid data type. Only double or integer is allowed.", actToken);

      // if its =
      CHECK_TOKEN(actToken, opEq);
      NEXT_TOKEN(actToken);
      syntx_processExpression(actToken, actSymbol);
      CHECK_TOKEN(actToken, kwTo);
      NEXT_TOKEN(actToken);

      TSymbol toSymb = NULL;
      TSymbol stepSymb = NULL;

      // set TO value
      TSymbol tmpToSymb = syntx_processExpression(actToken, NULL);
      if (tmpToSymb->dataType != dtInt && tmpToSymb->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "To value has no valid data type. Only double or integer is allowed.", actToken);
      if (tmpToSymb->type == symtConstant)
      {
        toSymb = tmpToSymb;
        balanceNumTypes(actSymbol, toSymb);
      }
      else // variable
      {
        toSymb = symbt_findOrInsertSymb("%to");
        defOrRedefVariable(toSymb);
        toSymb->dataType = tmpToSymb->dataType;
        balanceNumTypes(actSymbol, toSymb);
        printInstruction("MOVE %s %s\n", toSymb->ident, tmpToSymb->ident);
      }

      // set STEP Value
      TSymbol tmpStepSymb = ck_NT_FORSTEP(actToken);
      if (tmpStepSymb->dataType != dtInt && tmpStepSymb->dataType != dtFloat)
        scan_raiseCodeError(semanticErr, "Step value has no valid data type. Only double or integer is allowed.", actToken);
      if (tmpStepSymb->type == symtConstant)
      {
        stepSymb = tmpStepSymb;
        balanceNumTypes(actSymbol, stepSymb);
      }
      else // variable
      {
        stepSymb = symbt_findOrInsertSymb("%step");
        defOrRedefVariable(stepSymb);
        stepSymb->dataType = tmpStepSymb->dataType;
        balanceNumTypes(actSymbol, stepSymb);
        printInstruction("MOVE %s %s\n", stepSymb->ident, tmpStepSymb->ident);
      }

      CHECK_TOKEN(actToken, eol);
      // end of for initialization

      NEXT_TOKEN(actToken);
      // check condition
      printInstruction("LABEL %s$loop\n", forlabel);
      symbt_pushFrame(forlabel, true, true, false);
      printInstruction("DEFVAR TF@%%forisless\n");
      printInstruction("GT TF@%%forisless %s ", actSymbol->ident);
      //symbt_printSymb(toSymb);
      printSymbolToOperand(toSymb);
      printInstruction("\n");
      printInstruction("JUMPIFEQ %s$loopend TF@%%forisless bool@true\n", forlabel);

      // inner statements
      ck_NT_STAT_LIST(actToken);

      printInstruction("LABEL %s$loopinc\n", forlabel);

      // increment iterator
      printInstruction("ADD %s %s ", actSymbol->ident, actSymbol->ident);
      printSymbolToOperand(stepSymb);
      printInstruction("\n");

      symbt_popFrame();
      printInstruction("JUMP %s$loop\n", forlabel);
      printInstruction("LABEL %s$loopend\n", forlabel);
      CHECK_TOKEN(actToken, kwNext);
      NEXT_TOKEN(actToken);
      if (actToken->type == ident)
      {
        if (actToken->symbol != actSymbol)
          scan_raiseCodeError(syntaxErr, "Invalid iterator.", actToken);
        NEXT_TOKEN(actToken);
      }
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
// first(NT_DOIN) = { first(NT_DOIN_WU) -> (21); eol -> (22) else -> (error)}
void ck_NT_DOIN(SToken *actToken)
{
  char *dolabel = symbt_getNewLocalLabel();
  printInstruction("LABEL %s$loop\n", dolabel);
  symbt_pushFrame(dolabel, true, false, true);
  switch (actToken->type)
  {
    // 21. NT_DOIN -> NT_DOIN_WU eol NT_STAT_LIST kwLoop
    case kwWhile:
    case kwUntil:
      ck_NT_DOIN_WU(actToken, dolabel, false);
      symbt_pushFrame(dolabel, true, false, false);
      CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      CHECK_TOKEN(actToken, kwLoop);
      NEXT_TOKEN(actToken);
      symbt_popFrame();
      printInstruction("JUMP %s$loop\n", dolabel);
      break;
    // 22. NT_DOIN -> eol NT_STAT_LIST kwLoop NT_DOIN_WU
    case eol:
      symbt_pushFrame(dolabel, true, false, false);

      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      CHECK_TOKEN(actToken, kwLoop);
      NEXT_TOKEN(actToken);
      ck_NT_DOIN_WU(actToken, dolabel, true);
      symbt_popFrame();
      break;
    // else -> error
    default:
      ERR_UNEXP_TOKEN();
      break;
  }
  printInstruction("LABEL %s$loopend\n", dolabel);
  symbt_popFrame();
  mmng_safeFree(dolabel);
}

// until or while neterminal
// first(NT_DOIN_WU) = { kwWhile -> (23); kwUntil -> (24); else -> (epsilon) }
void ck_NT_DOIN_WU(SToken *actToken, char *doLabel, bool isOnEnd)
{
  TSymbol cond = NULL;
  switch (actToken->type)
  {
    // 23. NT_DOIN_WU -> kwWhile NT_EXPR
    case kwWhile:
      NEXT_TOKEN(actToken);
      cond = syntx_processExpression(actToken, NULL);
      if (cond->dataType != dtBool)
        ERR_COND_TYPE();

      printInstruction("%s %s$%s ",
        (isOnEnd) ? "JUMPIFEQ" : "JUMPIFNEQ",
        doLabel,
        (isOnEnd) ? "loop" : "loopend");
      printSymbolToOperand(cond);
      printInstruction(" bool@true\n");
      break;
    // 24. NT_DOIN_WU -> kwUntil NT_EXPR
    case kwUntil:
      NEXT_TOKEN(actToken);
      cond = syntx_processExpression(actToken, NULL);
      if (cond->dataType != dtBool)
        ERR_COND_TYPE();

      printInstruction("%s %s$%s ",
        (isOnEnd) ? "JUMPIFNEQ" : "JUMPIFEQ",
        doLabel,
        (isOnEnd) ? "loop" : "loopend");
      printSymbolToOperand(cond);
      printInstruction(" bool@true\n");
      break;
    // NT_DOIN_WU -> (epsilon)
    default:
      if (isOnEnd)
        printInstruction("JUMP %s$loop\n", doLabel);
      break;
  }
}

// step of for
// first(NT_FORSTEP) = { kwStep -> (25); else -> (epsilon) }
TSymbol ck_NT_FORSTEP(SToken *actToken)
{
  TSymbol result = NULL;
  switch (actToken->type)
  {
    // 25. NT_FORSTEP -> kwStep NT_EXPR
    case kwStep:
      NEXT_TOKEN(actToken);
      result = syntx_processExpression(actToken, NULL);
      break;
    // NT_FORSTEP -> (epsilon)
    default:
      result = symbt_findOrInsertSymb("1");
      if (result->type != symtConstant)
      {
        result->type = symtConstant;
        result->dataType = dtInt;
        result->data.intVal = 1;
      }
      break;
  }
  return result;
}

// extension of body of if statement
// first(NT_INIF_EXT) = { kwElseif -> (26); kwElse -> (27); else -> (epsilon) }
void ck_NT_INIF_EXT(SToken *actToken)
{
  switch (actToken->type)
  {
    // 26. NT_INIF_EXT -> kwElseif NT_EXPR kwThan eol NT_STAT_LIST NT_INIF_EXT
    case kwElseif:
      NEXT_TOKEN(actToken);
      char *endiflabel = symbt_getActLocalLabel();
      char *iflabel = symbt_getNewLocalLabel();

      TSymbol symbol = syntx_processExpression(actToken, NULL);
      if (symbol->dataType != dtBool)
        ERR_COND_TYPE();
      CHECK_TOKEN(actToken, kwThen);
      NEXT_CHECK_TOKEN(actToken, eol);
      printInstruction("JUMPIFNEQ %s$else %s bool@true\n", iflabel, symbol->ident);
      symbt_pushFrame(iflabel, true, false, false);
      NEXT_TOKEN(actToken);
      ck_NT_STAT_LIST(actToken);
      symbt_popFrame();
      printInstruction("JUMP %s$endif\n", endiflabel);
      printInstruction("LABEL %s$else\n", iflabel);
      mmng_safeFree(iflabel);
      ck_NT_INIF_EXT(actToken);
      break;
    // 27. NT_INIF_EXT -> kwElse eol NT_STAT_LIST
    case kwElse:
      NEXT_CHECK_TOKEN(actToken, eol);
      NEXT_TOKEN(actToken);
      char *frameLabel = symbt_getNewLocalLabel();
      symbt_pushFrame(frameLabel, true, false, false);
      mmng_safeFree(frameLabel);
      ck_NT_STAT_LIST(actToken);
      symbt_popFrame();
      break;
    // NT_INIF_EXT -> (epsilon)
    default:
      // let it be
      break;
  }
}

// list of expression for print function
// first(NT_PRINT_LIST) = { first(NT_EXPR) -> (28); else -> (epsilon) }
void ck_NT_PRINT_LIST(SToken *actToken)
{
  TSymbol actSymbol = NULL;
  switch (actToken->type)
  {
    // 28. ck_NT_PRINT -> NT_EXPR opSemcol NT_PRINT_LIST
    case opMns:
    case opLeftBrc:
    case ident:
      // evaluate expression
      actSymbol = syntx_processExpression(actToken, NULL);
      printInstruction("WRITE ");
      printSymbolToOperand(actSymbol);
      printInstruction("\n");
      CHECK_TOKEN(actToken, opSemcol);
      NEXT_TOKEN(actToken);
      ck_NT_PRINT_LIST(actToken);
      break;
    // NT_PRINT_LIST -> (epsilon)
    default:
        // let if be
      break;
  }
}

// extension of contuinue and exit
// first(NT_CYCLE_NESTS) = {kwDo -> (29); kwFor -> (30); else -> (epsilon)}
char *ck_NT_CYCLE_NESTS(SToken *actToken, bool isExit)
{
  int cnt = 0;
  bool isfor = false;
  switch (actToken->type)
  {
    // 29. NT_CYCLE_NESTS -> kwDo NT_CYCLES_DO
    case kwDo:
      NEXT_TOKEN(actToken);
      ck_NT_CYCLES_DO(actToken, &cnt);
      break;
    // 30. NT_CYCLE_NESTS -> kwFor NT_CYCLES_FOR
    case kwFor:
      NEXT_TOKEN(actToken);
      ck_NT_CYCLES_FOR(actToken, &cnt);
      isfor = true;
      break;
    // NT_CYCLE_NESTS -> (epsilon)
    default:
    break;
  }
  char *loclabel = NULL;
  char *complabel = NULL;
  if (!isfor)
    loclabel = symbt_getNthDoLoopLabel(cnt);
  else
    loclabel = symbt_getNthForLoopLabel(cnt);

  if (isExit)
    complabel = util_StrConcatenate(loclabel, "$loopend");
  else if (isfor) // continue for
    complabel = util_StrConcatenate(loclabel, "$loopinc");
  else // cotinue do
    complabel = util_StrConcatenate(loclabel, "$loop");

  return complabel;
}

// list of do kws
// first(NT_CYCLES_DO) = {opComma -> (31); else -> (epsilon)}
void ck_NT_CYCLES_DO(SToken *actToken, int *doCnt)
{
  switch (actToken->type)
  {
    // 31. NT_CYCLES_DO -> opComma kwDo NT_CYCLES_DO
    case opComma:
      NEXT_CHECK_TOKEN(actToken, kwDo);
      (*doCnt)++;
      NEXT_TOKEN(actToken);
      ck_NT_CYCLES_DO(actToken, doCnt);
      break;
    // NT_CYCLES_DO -> (epsilon)
    default:
        // let if be
      break;
  }
}

// list of for kws
// first(NT_CYCLES_FOR) = {opComma -> 43; else -> (44 [epsilon])}
void ck_NT_CYCLES_FOR(SToken *actToken, int *forCnt)
{
  switch (actToken->type)
  {
    // 32. NT_CYCLES_FOR -> opComma kwFor NT_CYCLES_FOR
    case opComma:
      NEXT_CHECK_TOKEN(actToken, kwFor);
      (*forCnt)++;
      NEXT_TOKEN(actToken);
      ck_NT_CYCLES_FOR(actToken, forCnt);
      break;
    // NT_CYCLES_FOR -> (epsilon)
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
  util_printBuildFunc();
  defineBuildInFuncSymbols();
  SToken token = scan_GetNextToken();
  ck_NT_PROG(&token);
  flushCode();
}