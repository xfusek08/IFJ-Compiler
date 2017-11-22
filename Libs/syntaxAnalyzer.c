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
#include <math.h>
#include "syntaxAnalyzer.h"
#include "grammar.h"
#include "Scanner.h"
#include "stacks.h"
#include "appErr.h"
#include "MMng.h"

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

  if( fmod(inputNum,1) == 0.5 && fmod(floorNum,2) == 0.0){ //rest 0.5 and even
    return floorNum;
  }else if(fmod(inputNum,1) == 0.5 && fmod(floorNum,2) == 1.0){   //rest 0.5 and odd
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

  if(token->symbol->type == symtConstant){ // converts only constant symbols
    token->symbol->data.doubleVal = syntx_intToDouble(token->symbol->data.intVal);
  }else if(token->symbol->type == symtVariable){ // converts variable
    printf("INT2FLOAT %s %s", token->symbol->ident, token->symbol->ident);
  }

  token->symbol->dataType = dtFloat;

}

/**
 * Implicitly converts token from double to int
 */
void syntx_doubleToIntToken(SToken *token){

  if(token->symbol->type == symtConstant){ // converts only constant symbols
    token->symbol->data.intVal = syntx_doubleToInt(token->symbol->data.doubleVal);
  }else if(token->symbol->type == symtVariable){ // converts variable
    printf("FLOAT2R2EINT %s %s", token->symbol->ident, token->symbol->ident); // half to even
  }

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
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> double - double

    syntx_intToDoubleToken(rightOperand);
    return 0; //TODO: maybe 1 - not clear from task

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> double - double

    syntx_intToDoubleToken(leftOperand);

    return 0;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of opDiv (integer division)
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfDiv(SToken *leftOperand, SToken *rightOperand){
  // TODO: really cast here - doubleToInt?
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> int - int

    syntx_doubleToIntToken(leftOperand);
    return 0;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> int - int

    syntx_doubleToIntToken(rightOperand);
    return 0;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of assignment operators
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
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> double - double

    syntx_intToDoubleToken(rightOperand);
    return 0;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> int - int

    syntx_doubleToIntToken(rightOperand);
    return 0;
  }

  return 0; //other combinations are not allowed
}

/**
 * Checks data types of boolean operators
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfBoolOps(SToken *leftOperand, SToken *rightOperand){

  // enabled data types pairs
  if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    return 1;
  }

  return 0; //other combinations are not allowed
}

/**
 * Checks data types, implicitly converts constants and generates code for implicit convertion of variables
 */
void syntx_checkDataTypes(SToken *leftOperand, SToken *operator, SToken *rightOperand){

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
  }else if( // if operators are assignment operators
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
  }else if( // if operator is boolean operator (except opBoolNot)
     operator->type == opBoolAnd ||
     operator->type == opBoolOr
  ){ 
    if(syntx_checkDataTypesOfBoolOps(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr);  // prints error
    }
  }

  apperr_runtimeError("syntaxAnalyzer.c, syntx_checkDataTypes(SToken *leftOperand, SToken *operator, SToken *rightOperand): unknown operator");

}

/**
 * Checks data types of boolean operators
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypeOfBool(SToken *boolOperand){

  // enabled data types pairs
  if(boolOperand->symbol->dataType == dtBool){  // bool
    return 1;
  }

  return 0; //other combinations are not allowed
}

/**
 * Optimalization function - do operation with constants (+, -, *, /, \, + for concat strings)
 * Always returns filled token, token.type = NT_EXPR, token.symbol->type
 * if everythng is OK, is setted token.symbol->dataType, token.symbol->data
 * else token.symbol->dataType = dtUnspecified and wrong token.symbol->data
 */
SToken syntx_doArithmeticOp(SToken *leftOperand, SToken *operator, SToken *rightOperand){

  SToken token;
  token.type = NT_EXPR;
  token.symbol = mmng_safeMalloc(sizeof(struct Symbol));
  token.symbol->type = symtConstant;
  token.symbol->dataType = dtUnspecified;


  if(leftOperand->symbol->type == symtConstant && rightOperand->symbol->type == symtConstant){  // if is possible do operation

    // by dataType choose right type from union, do implicit conversion and do operation
    if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){

      token.symbol->dataType = dtInt;

      if(operator->type == opPlus){
        token.symbol->data.intVal = leftOperand->symbol->data.intVal + rightOperand->symbol->data.intVal; // adds two integers
      }else if(operator->type == opMns){
        token.symbol->data.intVal = leftOperand->symbol->data.intVal - rightOperand->symbol->data.intVal; // subs two integers
      }else if(operator->type == opMul){
        token.symbol->data.intVal = leftOperand->symbol->data.intVal * rightOperand->symbol->data.intVal; // muls two integers
      }else if(operator->type == opDivFlt){
        token.symbol->data.doubleVal = leftOperand->symbol->data.intVal / rightOperand->symbol->data.intVal; // float divides two integers
        token.symbol->dataType = dtFloat; // result/dataType after divide is DOUBLE
      }else if(operator->type == opDiv){
        token.symbol->data.intVal = leftOperand->symbol->data.intVal / rightOperand->symbol->data.intVal; // integer divides two integers
      }

    }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int

      // integer division
      if(operator->type == opDiv){
        syntx_doubleToIntToken(leftOperand); // -> int - int
        token.symbol->data.intVal = leftOperand->symbol->data.intVal / rightOperand->symbol->data.intVal; // integer divides two doubles
        token.symbol->dataType = dtInt;
        return token;
      }

      //TODO: again! implicit conversion - return some value or not?
      syntx_intToDoubleToken(rightOperand); // -> double - double

      if(operator->type == opPlus){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal + rightOperand->symbol->data.doubleVal; // adds two doubles
      }else if(operator->type == opMns){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal - rightOperand->symbol->data.doubleVal; // subs two doubles
      }else if(operator->type == opMul){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal * rightOperand->symbol->data.doubleVal; // muls two doubles
      }else if(operator->type == opDivFlt){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal / rightOperand->symbol->data.doubleVal; // float divides two doubles
      }

      token.symbol->dataType = dtFloat;

    }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> double - double



      // integer division
      if(operator->type == opDiv){
        syntx_doubleToIntToken(rightOperand); // -> int - int
        token.symbol->data.intVal = leftOperand->symbol->data.intVal / rightOperand->symbol->data.intVal; // integer divides two doubles
        token.symbol->dataType = dtInt;
        return token;
      }

      //TODO: again! implicit conversion - return some value or not?
      syntx_intToDoubleToken(leftOperand); // -> double - double

      if(operator->type == opPlus){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal + rightOperand->symbol->data.doubleVal; // adds two doubles
      }else if(operator->type == opMns){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal - rightOperand->symbol->data.doubleVal; // subs two doubles
      }else if(operator->type == opMul){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal * rightOperand->symbol->data.doubleVal; // muls two doubles
      }else if(operator->type == opDivFlt){
        token.symbol->data.doubleVal = leftOperand->symbol->data.doubleVal / rightOperand->symbol->data.doubleVal; // float divides two doubles
      }

      token.symbol->dataType = dtFloat;

    }else if(leftOperand->symbol->dataType == dtString && rightOperand->symbol->dataType == dtString){  // string - string

      if(operator->type == opPlus){
        //CHECKME:
        token.symbol->data.stringVal = strcat(leftOperand->symbol->data.stringVal, rightOperand->symbol->data.stringVal); // adds two strings
        token.symbol->dataType = dtString;
      }

    }
  }

  // function was invoked with wrong arguments - typeCompatibilityErr
  if(token.symbol->dataType == dtUnspecified){
    scan_raiseCodeError(typeCompatibilityErr);  // prints error
  }

  return token;
}

/**
 * Prints int@constant, float@constant, bool@constant, string@constant or variable
 */
void syntx_printIdent(SToken *token){

  if(token->symbol->type == symtConstant){  // constant
    if(token->symbol->dataType == dtInt){
      printf("int@%d", token->symbol->data.intVal);
    }else if(token->symbol->dataType == dtFloat){
      printf("float@%g", token->symbol->data.doubleVal);
    }else if(token->symbol->dataType == dtString){
      printf("string@%s", token->symbol->data.stringVal);
    }else if(token->symbol->dataType == dtBool){
      printf("bool@%s", token->symbol->data.boolVal ? "true" : "false");
    }
  }else if(token->symbol->type == symtVariable){  // variable
    printf("%s", token->symbol->ident);
  }
}

/**
 * Generates instructions
 */
void syntx_generateInstruction(char *instrName, SToken *leftOperand, SToken *rightOperand, SToken *partialResult){
      printf("%s ", instrName);
      syntx_printIdent(partialResult);
      printf(" ");
      syntx_printIdent(leftOperand);
      printf(" ");
      syntx_printIdent(rightOperand);
      printf("\n");
}

/**
 * Generates code for basic operations
 */
void syntx_generateCodeForBasicOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
  
  switch(operator->type){
    case opPlus:
      syntx_generateInstruction("ADD", leftOperand, rightOperand, partialResult);
      break;
    case opMns:
      syntx_generateInstruction("SUB", leftOperand, rightOperand, partialResult);
      break;
    case opMul:
      syntx_generateInstruction("MUL", leftOperand, rightOperand, partialResult);
      break;
    case opDivFlt:
      syntx_generateInstruction("DIV", leftOperand, rightOperand, partialResult);
      break;
    default:
      break;
  }
}


/**
 * Main function for code generation
 * leftOperand expects constant or ident
 * operator expects one of available operators, otherwise throws apperr_runtimeError (in subfunction)
 * rightOperand expects constant, ident or NULL in case boolean NOT operator
 * partialResult reference variable to return result from part of expression back to the SyntaxAnalyzer
 */
void syntx_generateCode(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){

  // checks data types, implicitly converts constants and generates code for implicit convertion of variables
  if(rightOperand != NULL && operator->type != opBoolNot){
    syntx_checkDataTypes(leftOperand, operator, rightOperand);
  }else if(rightOperand == NULL && operator->type == opBoolNot){
    syntx_checkDataTypeOfBool(leftOperand);
  }else{  //for example: not string, not float, etc.
    scan_raiseCodeError(typeCompatibilityErr);  // prints error
  }

  // here are all data in right form



  partialResult->dataType = dtUnspecified;  // due to testing

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

