/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    syntaxAnalyzer.c
* \brief   Syntax analyze
* \author  Pavel Vosyka (xvosyk00)
* \date    10.11.2017 - Pavel Vosyka
*/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include "syntaxAnalyzer.h"
#include "grammar.h"
#include "scanner.h"
#include "stacks.h"
#include "appErr.h"
#include "MMng.h"
#include "symtable.h"
#include "erpxSemanticAnalyzer.h"

//=============================== DEBUG MACROS =========================================
#ifdef PRECDEBUG
#define DPRINT(x) fprintf(stderr, x); fprintf(stderr, "\n")
#define DDPRINT(x, y) fprintf(stderr, x, y); fprintf(stderr, "\n")
#define DSPRINT(x, y) fprintf(stderr, x); printEgr(y); fprintf(stderr, "\n")
#define LISTPRINT(x) TTkList_print(x)
#else
#define DPRINT(x) ((void)0)
#define DDPRINT(x,y) ((void)0)
#define DSPRINT(x, y) ((void)0)
#define LISTPRINT(x) ((void)0)
#endif
//======================================================================================

TTkList tlist; //list used as stack in precedent analyze
TPStack identStack; //list of free unused auxiliary variables
unsigned nextTokenIdent;

/**
* returns 1 if symbol is terminal, otherwise 0
*/
int isTerminal(EGrSymb symb)
{
  return symb < 1000;
}

int isBoolResult(SToken *arg2)
{
  if (arg2->type == opEq || arg2->type == opGrt || arg2->type == opLes || arg2->type == opLessEq || arg2->type == opGrtEq || arg2->type == opNotEq)
    return 1;
  return 0;
}

/**
* returns closest terminal to top of the stack (its acually list).
\post Token with terminal in list is set as active item
*/
EGrSymb syntx_getFirstTerminal(TTkList list)
{
  list->activate(list);
  while (list->getActive(list) != NULL) {
    if (isTerminal(list->active->token.type))
      return list->active->token.type;
    list->prev(list);
  }
  return eol;
}


SToken nextToken()
{
  SToken token = scan_GetNextToken();
  if (token.type == eof)
    scan_raiseCodeError(syntaxErr, "File unexpectedly ended.", NULL);
  DDPRINT("new token: %d", token.type);
  return token;
}

void syntx_emptyVarStack()
{
  while (identStack->count > 0)
  {
    identStack->pop(identStack);
  }
}

void syntx_freeVar(SToken *var)
{
  identStack->push(identStack, var->symbol);
}

SToken sytx_getFreeVar()
{
  SToken token;
  token.type = NT_EXPR_TMP;
  //if stack not empty, return ident from stack
  if(identStack->count != 0)
  {
    token.symbol = (TSymbol)identStack->top(identStack);
    identStack->pop(identStack);
    return token;
  }
  if (nextTokenIdent == 1000000) //is that enough?
  {
    apperr_runtimeError("syntx_getFreeVar(): Limit of auxiliary variables reached! Too complicated expression.");
  }
  //generate next ident
  char *ident = mmng_safeMalloc(sizeof(char) * 12); // TF@%T[1-9][0-9]{0,5}EOL = 12
  sprintf(ident, "TF@%%T%d", nextTokenIdent);
  nextTokenIdent++;
  //if not defined, define ident
  token.symbol = symbt_findSymb(ident);
  if (token.symbol == NULL)
  {
    token.symbol = symbt_insertSymbOnTop(ident);
    token.symbol->isTemp = true; // mark symbol as temporaly (deleted on changing frame)
    token.symbol->type = symtVariable;
    printInstruction("DEFVAR %s\n", ident);
  }
  token.symbol->dataType = dtUnspecified;
  mmng_safeFree(ident);
  return token;
}

/* Use syntax rule at the end of the list. If there is no valid combination, returns zero. */
int syntx_useRule(TTkList list)
{
  DPRINT("+++ Entering useRule()");
  list->activate(list);
  while (list->getActive(list)->type != precLes)
  {
    list->prev(list);
    if (list->active == NULL || list->active->token.type == eol)
      return 0;
  }
  list->next(list);

  SToken ret_var; //token for auxiliary variable with result
  switch (list->getActive(list)->type)
  {
  case ident:
    DPRINT("Using rule EXPR --> ident");
    list->getActive(list)->type = NT_EXPR;
    break;
  case NT_EXPR:
  case NT_EXPR_TMP:
    DPRINT("Using rule EXPR --> EXPR op EXPR");
    if (list->active->next == NULL)
      return 0;
    if (list->active->next->next == NULL)
      return 0;
    SToken *arg1 = &list->active->token;
    SToken *arg2 = &list->active->next->token;
    SToken *arg3 = &list->active->next->next->token;
    //symbt_printSymb(arg1->symbol);
    //fprintf(stderr, "op: %d\n", arg2->type);
    //symbt_printSymb(arg3->symbol);
    if (arg1->symbol->type == symtConstant && arg3->symbol->type == symtConstant)
    {
      if (isBoolResult(arg2))
      {
        ret_var = sytx_getFreeVar();
        syntx_generateCode(arg1, arg2, arg3, &ret_var);
        list->postInsert(list, &ret_var);
      }
      else
      {
        SToken t = syntx_doArithmeticOp(arg1, arg2, arg3);
        list->postInsert(list, &t);
      }
      list->next(list);
      list->preDelete(list);
      list->postDelete(list);
      list->postDelete(list);
    }
    else if(arg1->type == NT_EXPR_TMP && arg3->type == NT_EXPR_TMP){
      syntx_generateCode(arg1, arg2, arg3, arg1);
      syntx_freeVar(arg3);
      list->postDelete(list);
      list->postDelete(list);
    }
    else if (arg1->type == NT_EXPR_TMP) {
      syntx_generateCode(arg1, arg2, arg3, arg1);
      list->postDelete(list);
      list->postDelete(list);
    }
    else if (arg3->type == NT_EXPR_TMP) {
      syntx_generateCode(arg1, arg2, arg3, arg3);
      list->postDelete(list);
      list->next(list);
      list->preDelete(list);
    }
    else {
      ret_var = sytx_getFreeVar();
      syntx_generateCode(arg1, arg2, arg3, &ret_var);
      list->postDelete(list);
      list->postDelete(list);
      list->postInsert(list, &ret_var);
      list->next(list);
      list->preDelete(list);
    }
    break;
  case opBoolNot:
    DPRINT("Using rule EXPR --> kwNot EXPR");
    if (list->active->next == NULL)
      return 0;
    ret_var = sytx_getFreeVar();
    syntx_generateCode(&list->active->next->token, &list->active->token, NULL, &ret_var);
    list->postDelete(list);
    list->postInsert(list, &ret_var);
    list->next(list);
    list->preDelete(list);
    break;
  case opPlus:
    DPRINT("Using rule EXPR --> opPlus EXPR");
    //unary plus
    if (list->active->next == NULL)
      return 0;
    list->next(list);
    list->preDelete(list);
    break;
  case opMns:
    DPRINT("Using rule EXPR --> opMns EXPR");
    //unary minus
    if (list->active->next == NULL)
      return 0;
    ret_var = sytx_getFreeVar();
    SToken zeroT;
    zeroT.type = ident;
    zeroT.symbol = mmng_safeMalloc(sizeof(struct Symbol));
    zeroT.symbol->type = symtConstant;
    zeroT.symbol->dataType = dtInt;
    zeroT.symbol->data.intVal = 0;
    syntx_generateCode(&zeroT, &list->active->token, &list->active->next->token, &ret_var);
    list->postDelete(list);
    list->postInsert(list, &ret_var);
    list->next(list);
    list->preDelete(list);
    break;
  case opLeftBrc:
    if (list->active->next == NULL)
      return 0;
    if (list->active->next->next == NULL)
      return 0;
    if(list->active->next->token.type != NT_EXPR && list->active->next->token.type != NT_EXPR_TMP)
      return 0;
    if(list->active->next->next->token.type != opRightBrc)
      return 0;
    list->next(list);
    list->preDelete(list);
    list->postDelete(list);
    break;
  default:
    return 0;
  }
  //test end of stack
  if (list->active->next != NULL)
    return 0;

  //delete <
  list->preDelete(list);
  DPRINT("+++ leaving useRule()");
  return 1;
}

int isExpressionType(EGrSymb type)
{
  return type <= eol && type != opComma;
}

//activate last eol
TTkListItem *findLastEol(TTkList list)
{
  TTkListItem *activeT = list->active;
  list->activate(list);
  while (list->active->token.type != eol)
    list->prev(list);
  TTkListItem *lasteol = list->active;
  list->active = activeT;
  return lasteol;
}

int isExprEnded(TTkList list, SToken *actToken, EGrSymb terminal)
{
  TTkListItem *lastEol = findLastEol(list);
  if (lastEol->next != NULL && lastEol->next->next == NULL)
  {
    if (actToken->type == opRightBrc)
      return 1;
  }
  if (terminal == eol && actToken->type == eol)
  {
    return 1;
  }
  if (!isExpressionType(actToken->type))
  {
    return 1;
  }
  return 0;
}

SToken syntx_parseFunction(SToken *actToken)
{
  SToken funcToken = *actToken;
  *actToken = nextToken();
  if (actToken->type != opLeftBrc)
    scan_raiseCodeError(semanticErr, "Missing '(' after function identifier.", actToken);
  int argNum = 0;
  while (actToken->type != opRightBrc)
  {
    *actToken = nextToken();
    if (actToken->type == opRightBrc)
      break;
    SToken el;
    el.type = eol;
    tlist->insertLast(tlist, &el);
    SToken argToken;
    argToken.type = NT_EXPR;
    argToken.symbol = syntx_processExpression(actToken, NULL);
    tlist->deleteLast(tlist);
    if (actToken->type != opComma && actToken->type != opRightBrc)
    {
      scan_raiseCodeError(syntaxErr, "Function call isn't end with ')'.", actToken);
    }
    syntx_generateCodeForVarDef(&funcToken, argNum, &argToken);
    argNum++;
  }
  SToken returnVal;
  returnVal = sytx_getFreeVar();
  syntx_generateCodeForCallFunc(&funcToken, argNum, &returnVal);
  return returnVal;
}


int syntx_processUnaryOps(TTkList list, SToken *actToken)
{
  EGrSymb last = list->last->token.type;
  if (last == opPlus && actToken->type == opPlus)
  {
    return 1;
  }
  else if (last == opPlus && actToken->type == opMns)
  {
    list->last->token.type = opMns;
    return 1;
  }
  else if (last == opMns && actToken->type == opPlus)
  {
    return 1;
  }
  else if (last == opMns && actToken->type == opMns)
  {
    list->last->token.type = opPlus;
    return 1;
  }
  return 0;
}

void syntx_tableLogic(TTkList list, EGrSymb terminal, SToken *actToken)
{
  if (syntx_processUnaryOps(list, actToken)) //check for unary operator optimalization
  {
    *actToken = nextToken();
    return;
  }
  EGrSymb tablesymb;
  if (!syntx_getPrecedence(terminal, actToken->type, &tablesymb))
  {
    DPRINT("Table: undefined precedence!");
    scan_raiseCodeError(syntaxErr, "Incorrect expression. Undefined precedence.", actToken);
  }
  DDPRINT("Table: %d", tablesymb);

  switch (tablesymb)
  {
  case precEqu:
    list->insertLast(list, actToken);
    *actToken = nextToken();
    break;
  case precLes:
  {
    SToken auxToken;
    auxToken.type = precLes;
    list->postInsert(list, &auxToken);
    list->insertLast(list, actToken);
    *actToken = nextToken();
  }
  break;
  case precGrt:
    if (!syntx_useRule(tlist))
    {
      scan_raiseCodeError(syntaxErr, "Incorrect expression.", NULL);
    }
    break;
  default:
    apperr_runtimeError("SyntaxAnalyzer.c: internal error!");
  }
}

/**
* Precedent statement analyze
*/
TSymbol syntx_processExpression(SToken *actToken, TSymbol symbol)
{
  DPRINT("\n\n+++++++++++++");
  DPRINT("entering expr");
  if (tlist == NULL)
    apperr_runtimeError("syntx_processExpression(): Modul not initialized. Call syntx_init() first!");
  if (!isExpressionType(actToken->type))
    scan_raiseCodeError(syntaxErr, "Symbol is not an expression.", actToken);
  nextTokenIdent = 0; //reset identificator generator
  EGrSymb terminal = syntx_getFirstTerminal(tlist);
  while (1)
  {
    //debug print
    DSPRINT("Analyzing token: ", actToken->type);
    DSPRINT("First terminal on stack: ", terminal);

    if (actToken->symbol != NULL)
    {
      if(actToken->symbol->type == symtFuction)
      {
        //Function call is replaced with aux variable with result.
        //ParseFunction ends on ')', no token will be skiped.
        //note: auxiliary variable will not be freed and reused in expression
        *actToken = syntx_parseFunction(actToken);
        actToken->type = ident;
        terminal = syntx_getFirstTerminal(tlist);
      }else if(actToken->symbol->type == symtUnknown)
      {
        scan_raiseCodeError(semanticErr, "Undefined symbol.", actToken);
      }
    }

    syntx_tableLogic(tlist, terminal, actToken);

    terminal = syntx_getFirstTerminal(tlist);
    DPRINT("--------------------------------");
    LISTPRINT(tlist);
    DPRINT("-------konec-iterace------------\n");
    if (isExprEnded(tlist, actToken, terminal))
      break;
  }

  while (syntx_useRule(tlist)); //parse everything already on stack

  DPRINT("End of expression.");

  //test correct ending
  TTkListItem *lastEol = findLastEol(tlist);
  if(!(lastEol->next != NULL && lastEol->next->next == NULL))
  {
    scan_raiseCodeError(syntaxErr, "Incomplete expression.", NULL);
  }

  //free ident stack
  syntx_emptyVarStack();

  //return result
  SToken resultToken = tlist->last->token;
  if (symbol == NULL)
  {
    //return temporary variable with result
    TSymbol symb = resultToken.symbol;
    //delete last token and return
    tlist->deleteLast(tlist);
    DDPRINT("Result in %s\n", symb->ident);
    return symb;
  }
  else {
    //mov result from temporary variable to requested variable
    SToken retT;
    retT.dataType = NT_EXPR;
    retT.symbol = symbol;
    SToken asgnT;
    asgnT.type = asgn;
    syntx_checkDataTypes(&retT, &asgnT, &resultToken);
    syntx_generateCodeForAsgnOps(&retT, &asgnT, &resultToken, NULL);
    tlist->deleteLast(tlist);
    DDPRINT("Result in %s\n", symbol->ident);
    return symbol;
  }
}

void syntx_init()
{
  tlist = TTkList_create();
  identStack = TPStack_create();
  SToken auxToken;
  auxToken.type = eol;
  tlist->insertLast(tlist, &auxToken);
  DPRINT("precedent syntax init");
}

void syntx_destroy()
{
  tlist->deleteLast(tlist);
  tlist->destroy(tlist);
  while (identStack->count > 0)
    identStack->pop(identStack);
  identStack->destroy(identStack);
}