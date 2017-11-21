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

#define DPRINT(x) fprintf(stderr, x); fprintf(stderr, "\n")
#define DDPRINT(x, y) fprintf(stderr, x, y); fprintf(stderr, "\n")

SToken token; //loaded token, we might want to have array/list of tokens for code generation
TTkList tlist; //list used as stack in syntx_processExpression

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

/**
 * Converts double to integer
 */
int syntx_doubleToInt(double inputNum){
  int floorNum = inputNum;

  if( remainder(inputNum,1) == 0.5 && remainder(floorNum,2) == 0.0){ //rest 0.5 and even
    return floorNum;
  }else if(remainder(inputNum,1) == 0.5 && remainder(floorNum,2) == 1.0){   //rest 0.5 and odd
    return floorNum + 1;
  }else{
    return round(inputNum);
  }
}

/**
 * Converts integer to double
 */
double syntx_intToDouble(int inputNum){
  return inputNum;
}

/**
 * Implicitly converts token from int to double
 */
void syntx_intToDoubleToken(SToken *token){

  token->symbol->data.doubleVal = syntx_intToDouble(token->symbol->data.intVal);
  token->symbol->dataType = dtFloat;

}

/**
 * Implicitly converts token from double to int
 */
void syntx_doubleToIntToken(SToken *token){

  token->symbol->data.intVal = syntx_doubleToInt(token->symbol->data.doubleVal);
  token->symbol->dataType = dtInt;

}

/**
 * Checks data types of two operands
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfBasicOp(SToken *leftOperand, SToken *rightOperand){
  //TODO: optimalization
  // enabled data types pairs
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtFloat){  // double - double
    return 1;
  }else if(leftOperand->symbol->dataType == dtString && rightOperand->symbol->dataType == dtString){  // string - string
    return 1;
  }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int
    syntx_intToDoubleToken(rightOperand);
    return 0;
  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double
    syntx_intToDoubleToken(leftOperand);
    return 0;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of opDiv
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfDiv(SToken *leftOperand, SToken *rightOperand){
  // TODO: cast here?
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int
    syntx_doubleToIntToken(leftOperand);
    return 0;
  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double
    syntx_doubleToIntToken(rightOperand);
    return 0;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of asigns operators
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfAgnOps(SToken *leftOperand, SToken *rightOperand){
  //TODO: optimalization

  // enabled data types pairs
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtFloat){  // double - double
    return 1;
  }else if(leftOperand->symbol->dataType == dtString && rightOperand->symbol->dataType == dtString){  // string - string
    return 1;
  }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> int - int
    syntx_intToDoubleToken(rightOperand);
    return 0;
  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> double - double
    syntx_doubleToIntToken(rightOperand);
    return 0;
  }

  return 0; //other combinations are not allowed
}

/**
 * Checks data types
 */
void checkDataTypes(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){

  // if operator type is +, -, *, /, <, >, <=, >=, =, <>, +=, -=, /=, \=, :=
  if(operator->type == opPlus || 
     operator->type == opMns || 
     operator->type == opMul || 
     operator->type == opDivFlt ||
     operator->type == opLes ||
     operator->type == opGrt ||
     operator->type == opLessEq ||
     operator->type == opGrtEq ||
     operator->type == opEq ||
     operator->type == opNotEq
     ){
      //checks compalibility of data types
      if(syntx_checkDataTypesOfBasicOp(leftOperand, rightOperand) == 0){
        scan_raiseCodeError(typeCompatibilityErr);  // prints error
      }
  }else if(operator->type == opDiv){  // if operator type is '\'
    if(syntx_checkDataTypesOfDiv(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr);  // prints error
    }
  }else if(
     operator->type == opPlusEq ||
     operator->type == opMnsEq ||
     operator->type == opMulEq ||
     operator->type == opDivFlt ||
     operator->type == opDiv ||
     operator->type == asng
  ){
    if(syntx_checkDataTypesOfAgnOps(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr);  // prints error
    }
  }

}

/**
 * Main function for code generation
 */
void syntx_generateCode(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
  checkDataTypes(leftOperand, operator, rightOperand, partialResult);
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

/**
* Semantic statement analyze
*/
void statSemantic(int ruleNum)
{
  DDPRINT("Semantic statement analyze: Received rule number %d", ruleNum);
}

SToken nextToken()
{
  if (!scan_GetNextToken(&token))
  {
    fprintf(stderr, "File unexpectedly ended.");
    scan_raiseCodeError(syntaxErr);
  }
  DDPRINT("new token: %d", token.type);
  return token;
}

/* Use syntax rule at the end of the list. If there is no valid combination, returns zero. */
int syntx_useRule(TTkList list)
{
  DPRINT("Entering useRule()");
  list->activate(list);
  while (list->getActive(list)->type != precLes)
  {
    list->prev(list);
    if (list->getActive(list) == NULL)
      return 0;
  }
  list->next(list);
  //SToken nonTerm;
  switch (list->getActive(list)->type)
  {
  case ident:
    list->getActive(list)->type = NT_EXPR;
    //list->prev(list);
    //list->postDelete(list);
    //nonTerm.type = NT_EXPR;
    //list->postInsert(list, nonTerm)
    break;
  //case NT_EXPR:

  default:
    return 0;
  }
  //test end of stack
  if (list->active->next != NULL)
    return 0;

  //delete <
  list->preDelete(list);
  DPRINT("leaving useRule()");
  return 1;
}

/**
* Precedent statement analyze
*/
void syntx_processExpression(SToken *actToken, const char *frame, const char *ident, DataType datatype)
{
  SToken auxToken;
  auxToken.type = eol;
  tlist->insertLast(tlist, &auxToken);
  
  DPRINT("po pushnuti eol");

  *actToken = nextToken();
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
      auxToken.type = precLes;
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
    DPRINT("po jedne iteraci");
  } while (!(terminal == eol && actToken->type == eol));
  DPRINT("konec vyrazu.");
  (void)ident;
  (void)frame;
  (void)datatype;
}


void syntx_init()
{
  tlist = TTkList_create();
  DPRINT("precedent syntax init");
}

