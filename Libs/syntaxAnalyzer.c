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
#include "syntaxAnalyzer.h"
#include "grammar.h"
#include "Scanner.h"
#include "stacks.h"
#include "appErr.h"
#include "MMng.h"
#include "symtable.h"

#define DPRINT(x) fprintf(stderr, x); fprintf(stderr, "\n")
#define DDPRINT(x, y) fprintf(stderr, x, y); fprintf(stderr, "\n")

TTkList tlist; //list used as stack in syntx_processExpression
TPStack identStack; //list of free unused identificators
unsigned nextTokenIdent;
//radim******
/**
* Returns 0 if symbols are out of range or relation between symbols is not defined, otherwise 1
* stackSymb symbol on top of stack
* inputSymb symbol from end of expression
* precRtrn returns precedence between two symbols (vals: precLes, precEq, precGrt)
*/
int syntx_getPrecedence(EGrSymb stackSymb, EGrSymb inputSymb, EGrSymb *precRtrn)
{
  // check range
  if(stackSymb > 21 || inputSymb > 21){
    return 0;
  }

  EGrSymb precTable[22][22] = {
             /* + */   /* - */  /* * */   /* / */  /* \ */  /* ( */  /* ) */  /* i */  /* , */  /* = */  /* <> */         /* < */  /* <= */ /* > */  /* >= */ /* += */ /* -= */ /* *= */ /* /= */ /* \= */ /* := */ /* $ */
    /* +  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* +  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* -  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* -  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* *  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* *  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* /  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* /  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* \  */ {precGrt, precGrt, precLes, precLes, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* \  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* (  */ {precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precEqu, precLes, precLes, /* (  */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd},
    /* )  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* )  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* i  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* i  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* ,  */ {precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precLes, /* ,  */ precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes},
    /* =  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* =  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* <> */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <> */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* <  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* <= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <= */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* >  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* >= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >= */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* += */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* += */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* -= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* -= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* *= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* *= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* /= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* /= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* \= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* \= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* := */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* := */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt},
    /* $  */ {precLes, precLes, precLes, precLes, precLes, precLes, precUnd, precLes, precLes, precLes, precLes, /* $  */ precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precUnd}
  };

  // check symbols relation
  if(precTable[stackSymb][inputSymb] == precUnd){
    return 0;
  }

  *precRtrn = precTable[stackSymb][inputSymb]; // save to reference variable

  return 1;
}
//radim konec*************

/**
* returns 1 if symbol is terminal, otherwise 0
*/
int isTerminal(EGrSymb symb)
{
  return symb < 1000 ? 1 : 0;
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
  SToken token;
  if (!scan_GetNextToken(&token))
  {
    fprintf(stderr, "File unexpectedly ended.");
    scan_raiseCodeError(syntaxErr);
  }
  DDPRINT("new token: %d", token.type);
  return token;
}

void syntx_emptyVarStack() 
{
  while (identStack->count > 0)
  {
    mmng_safeFree(identStack->top(identStack));
    identStack->pop(identStack);
  }
}

void syntx_freeVar(SToken *var)
{
  identStack->push(identStack, var->symbol->ident);
}

SToken sytx_getFreeVar()
{
  SToken token;
  token.type = NT_EXPR_TMP;
  token.symbol = mmng_safeMalloc(sizeof(struct Symbol));
  token.symbol->type = symtVariable;

  //if stack not empty, return ident from stack
  if (identStack->count != 0)
  {
    token.symbol->ident = (char *)identStack->top(identStack);
    identStack->pop(identStack);
    return token;
  }
  if (nextTokenIdent == 1000)
  {
    apperr_runtimeError("syntx_getFreeVar(): Limit of auxiliary variables reached! Too complicated expression.");
  }
  //generate next ident
  char *ident = mmng_safeMalloc(sizeof(char) * 9); // LF@%T[1-9][0-9]{0,2}EOL = 9
  sprintf(ident, "LF@\%T%d", nextTokenIdent);
  //if not defined, define ident
  if (symbt_findSymb(ident) == NULL)
  {
    symbt_insertSymbOnTop(ident);
    printf("DEFVAR %s", ident);
  }
  token.symbol->ident = ident;
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
    if (list->getActive(list) == NULL)
      return 0;
  }
  list->next(list);
  
  switch (list->getActive(list)->type)
  {
  case ident:
    list->getActive(list)->type = NT_EXPR;
    break;
  case NT_EXPR:
  case NT_EXPR_TMP:
    SToken ref_var;
    //test next->next...
    if (list->active->next == NULL)
      return 0;
    if (list->active->next->next == NULL)
      return 0;
    SToken *arg1 = list->active;
    SToken *arg2 = list->active->next;
    SToken *arg3 = list->active->next->next;
    if (arg1->symbol->type == symtConstant && arg3->symbol->type == symtConstant)
    {
      //radim2(..., &ref_nahrada)
    }
    else if(arg1->type == NT_EXPR_TMP && arg3->type == NT_EXPR_TMP){
       //radim(active, next, next->next, &arg1)
       //syntx_freeVar(arg3);
       //list->postDelete(list);
       //list->postDelete(list);
    }
    else if (arg1->type == NT_EXPR_TMP) {
      //radim(..., &arg1)
      //list->postDelete(list);
      //list->postDelete(list);
    }
    else if (arg3->type == NT_EXPR_TMP) {
      //radim(..., &arg3)
      //list->postDelete(list);
      //list->next(list);
      //list->preDelete(list);
    }
    else {
      ref_var = sytx_getFreeVar();
      //radim(..., &ref_var)
      //list->postDelete(list);
      //list->postDelete(list);
      //list->postInsert(list, ref_var);
      //list->next(list);
      //list->preDelete(list);
    }
    break;
  //case not: radim(not, null)
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

/**
* Precedent statement analyze
*/
Symbol syntx_processExpression(SToken *actToken)
{
  SToken auxToken;
  auxToken.type = eol;
  tlist->insertLast(tlist, &auxToken);
  nextTokenIdent = 0; //reset ident generator
  DPRINT("po pushnuti eol");

  EGrSymb terminal;
  do {
     terminal = syntx_getFirstTerminal(tlist);

    //debug print
    DDPRINT("Analyzing token: %d", actToken->type);
    DDPRINT("First terminal on stack: %d", terminal);

    EGrSymb tablesymb;
    //TODO: handle function identifier somehow
    if (!syntx_getPrecedence(terminal, actToken->type, &tablesymb))
    {
      scan_raiseCodeError(syntaxErr);
    }
    DDPRINT("Table: %d", tablesymb);
    switch (tablesymb)
    {
    case precEqu:
      tlist->insertLast(tlist, actToken);
      *actToken = nextToken();
      break;
    case precLes:
      auxToken->type = precLes;
      tlist->postInsert(tlist, &auxToken);
      tlist->insertLast(tlist, actToken);
      *actToken = nextToken();
      break;
    case precGrt:
      if (!syntx_useRule(tlist))
      {
        scan_raiseCodeError(syntaxErr);
      }
      break;
    default:
      apperr_runtimeError("SyntaxAnalyzer.c: internal error!");
    }
    DPRINT("--------------------------------");
    DPRINT("-------konec-iterace------------");
    getchar();
  } while (!(terminal == eol && actToken->type == eol) && *actToken->type <= 24);
  DPRINT("konec vyrazu.");

  //TODO free ident stack
  syntx_emptyVarStack();
  //TODO return symbol
}


void syntx_init()
{
  tlist = TTkList_create();
  identStack = TPStack_create();
  DPRINT("precedent syntax init");
}

