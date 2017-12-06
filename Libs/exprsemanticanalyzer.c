/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    exprsemanticanalyzer.c
* \brief   Expression semantic
* \author  Radim Blaha (xblaha28)
* \date    6.12.2017 - Radim Blaha
*/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "grammar.h"
#include "scanner.h"
#include "apperr.h"
#include "mmng.h"
#include "symtable.h"
#include "exprsemanticanalyzer.h"
#include "syntaxanalyzer.h"
#include "utils.h"

// variable for passing the symbol with the converted value between the functions
// is used for example for expression: A(int) = A(int) + A(double)
// int variable A is converted to double and then is
TSymbol convertedSymbol;
int isMyTemp = 0;

/**
* Returns 0 if symbols are out of range or relation between symbols is not defined, otherwise 1
* stackSymb symbol on top of stack
* inputSymb symbol from end of expression
* precRtrn returns precedence between two symbols (vals: precLes, precEq, precGrt)
*/
int syntx_getPrecedence(EGrSymb stackSymb, EGrSymb inputSymb, EGrSymb *precRtrn)
{
  // check range
  if(stackSymb > 24 || inputSymb > 24){
    return 0;
  }

  EGrSymb precTable[25][25] = {
             /* + */   /* - */  /* * */  /* / */  /* \ */  /* ( */  /* ) */  /* i */  /* , */  /* = */  /* <> */          /* < */  /* <= */ /* > */  /* >= */ /*asgn*/ /* += */ /* -= */ /* *= */ /* /= */ /* \= */ /* NOT*/ /* AND*/ /* OR */ /* $ */
    /* +  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* +  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* -  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* -  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* *  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* *  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* /  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* /  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* \  */ {precGrt, precGrt, precLes, precLes, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* \  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* (  */ {precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precEqu, precLes, precLes, /* (  */ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precLes, precLes, precLes, precUnd},
    /* )  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* )  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* i  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* i  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* ,  */ {precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precEqu, precLes, precLes, /* ,  */ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precLes, precLes, precLes, precUnd},
    /* =  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* =  */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* <> */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <> */ precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* <  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <  */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* <= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* >  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >  */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* >= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /*asgn*/ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* := */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* += */ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* += */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* -= */ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* -= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* *= */ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* *= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* /= */ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* /= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* \= */ {precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, /* \= */ precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd},
    /* NOT*/ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* NOT*/ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precGrt, precGrt, precGrt, precGrt},
    /* AND*/ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* AND*/ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precLes, precLes, precLes, precGrt},
    /* OR */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* OR */ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precLes, precLes, precLes, precGrt},
    /* $  */ {precLes, precLes, precLes, precLes, precLes, precLes, precUnd, precLes, precUnd, precLes, precLes, /* $  */ precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd, precUnd, precUnd, precLes, precLes, precLes, precUnd}
  };

  // check symbols relation
  if(precTable[stackSymb][inputSymb] == precUnd){
    return 0;
  }

  *precRtrn = precTable[stackSymb][inputSymb]; // save to reference variable

  return 1;
}

/**
 * Deeply copies token
 */
SToken syntx_deepCopyToken(SToken *token){
  if (token->symbol->type == symtFuction)
    apperr_runtimeError("Can not make deep copy of function symbol");

  SToken tokenCopy;

  tokenCopy.dataType = token->dataType;
  //tokenCopy.symbol = token->symbol;
  tokenCopy.type = token->type;

  tokenCopy.symbol = symbt_getUniqeTmpSymb();
  if (token->symbol->type == symtConstant)
  {
    if (token->symbol->dataType == dtString)
      tokenCopy.symbol->data.stringVal = util_StrHardCopy(token->symbol->data.stringVal);
    else
      tokenCopy.symbol->data = token->symbol->data;
  }

  tokenCopy.symbol->dataType = token->symbol->dataType;
  mmng_safeFree(tokenCopy.symbol->ident);
  tokenCopy.symbol->ident = util_StrHardCopy(token->symbol->ident);
  tokenCopy.symbol->type = token->symbol->type;

  return tokenCopy;
}

/**
 * Converts double to integer - method half to even
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

    // changes copied symbol to converted symbol
    SToken temp = sytx_getFreeVar();

    printInstruction("INT2FLOAT %s %s\n", temp.symbol->ident, token->symbol->ident);
    temp.symbol->dataType = dtFloat;

    token->symbol = temp.symbol; // change symbol of token to converted symbol
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

    // changes copied symbol to converted symbol
    SToken temp = sytx_getFreeVar();

    printInstruction("FLOAT2R2EINT %s %s\n", temp.symbol->ident, token->symbol->ident); // half to even
    temp.symbol->dataType = dtInt;

    token->symbol = temp.symbol; // change symbol of token to converted symbol
  }

  token->symbol->dataType = dtInt;

}

/**
 * Checks data types of two operands
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfBasicOp(SToken *leftOperand, SToken *operator, SToken *rightOperand){

  //TODO: optimalization
  // enabled data types pairs
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int

    if(operator->type == opDivFlt){ // DIV can not work with integers: int - int -> double - double
      syntx_intToDoubleToken(leftOperand);
      syntx_intToDoubleToken(rightOperand);
    }

    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtFloat){  // double - double
    return 1;
  }else if(leftOperand->symbol->dataType == dtString && rightOperand->symbol->dataType == dtString){  // string - string
    return 1;
  }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    if(operator->type == opEq || operator->type == opNotEq){  //operator must be =, <>
      return 1;
    }
    return 0;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> double - double

    syntx_intToDoubleToken(rightOperand);
    return 1; //TODO: maybe 0 - not clear from task

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> double - double

    syntx_intToDoubleToken(leftOperand);

    return 1;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of opDiv (integer division)
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfIntegerDiv(SToken *leftOperand, SToken *rightOperand){
  // CHECKME: really cast here - doubleToInt?
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int

    // DIV can not work with integers: int - int -> double - double
    syntx_intToDoubleToken(leftOperand);
    syntx_intToDoubleToken(rightOperand);

    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtFloat){  // double - double
    // round
    syntx_doubleToIntToken(leftOperand);
    syntx_doubleToIntToken(rightOperand);

    // DIV can not work with integers: int - int -> double - double
    syntx_intToDoubleToken(rightOperand);
    syntx_intToDoubleToken(leftOperand);

    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> round(double) - int -> double - double

    syntx_doubleToIntToken(leftOperand);  // round

    // DIV can not work with integers: int - int -> double - double
    syntx_intToDoubleToken(rightOperand);
    syntx_intToDoubleToken(leftOperand);
    return 1;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> round(double) - int -> double - double

    syntx_doubleToIntToken(rightOperand);

    // DIV can not work with integers: int - int -> double - double
    syntx_intToDoubleToken(rightOperand);
    syntx_intToDoubleToken(leftOperand);
    return 1;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of assignment operators
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfAsgnOps(SToken *leftOperand, SToken *oper, SToken *rightOperand){
  //TODO: optimalization

  if(leftOperand->symbol->type == symtConstant){  // if asigns to the constant -> error
    return 0;
  }

  // enabled data types pairs
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtFloat){  // double - double
    return 1;
  }else if(leftOperand->symbol->dataType == dtString && rightOperand->symbol->dataType == dtString){  // string - string
    if(oper->type == asgn || oper->type == opPlus || oper->type == opPlusEq){
      return 1;
    }
    return 0;
  }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> double - double

    syntx_intToDoubleToken(rightOperand);
    return 1;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> int - int

    syntx_doubleToIntToken(rightOperand);
    return 1;
  }else if(leftOperand->symbol->dataType == dtUnspecified){ // undeclared
    scan_raiseCodeError(semanticErr, "Attempt to assign to undeclared variable.", NULL);  // prints error
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
      if(syntx_checkDataTypesOfBasicOp(leftOperand, operator, rightOperand) == 0){
        scan_raiseCodeError(typeCompatibilityErr, "Error during arithmetic or relational operation.", NULL);  // prints error
      }
  }else if(operator->type == opDiv){  // if operator type is '\'
    if(syntx_checkDataTypesOfIntegerDiv(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr, "Error during integer division.", NULL);  // prints error
    }
  }else if( // if operators are assignment operators
     operator->type == asgn ||
     operator->type == opPlusEq ||
     operator->type == opMnsEq ||
     operator->type == opMulEq ||
     operator->type == opDivFltEq
  ){
    if(syntx_checkDataTypesOfAsgnOps(leftOperand, operator, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr, "Error during assignment operation.", NULL);  // prints error
    }
  }else if(operator->type == opDivEq){ // if operator is \=
    if(syntx_checkDataTypesOfIntegerDiv(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr, "Error during integer division and assignment.", NULL);  // prints error
    }
  }else if( // if operator is boolean operator (except opBoolNot)
     operator->type == opBoolAnd ||
     operator->type == opBoolOr
  ){
    if(syntx_checkDataTypesOfBoolOps(leftOperand, rightOperand) == 0){
      scan_raiseCodeError(typeCompatibilityErr, "One or both operands do not have boolean type.", NULL);  // prints error
    }
  }else{  // unknown operator
    apperr_runtimeError("syntaxAnalyzer.c, syntx_checkDataTypes(SToken *leftOperand, SToken *operator, SToken *rightOperand): unknown operator");
  }

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
 * Checks if operation with booleans is syntacally correct
 */
int checkBoolOpSyntax(SToken *leftOperand, SToken *oper, SToken *rightOperand){

  if(leftOperand->symbol->type == symtConstant && rightOperand == NULL){  // NOT bool
    if(leftOperand->symbol->dataType == dtBool){
      if(oper->type == opBoolNot){
        return 1;
      }
    }
  }else if(leftOperand->symbol->type == symtConstant && rightOperand->symbol->type == symtConstant){  // if operation is possible to do
    if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){
      if(oper->type == opEq){
        return 1;
      }else if(oper->type == opNotEq){
        return 1;
      }else if(oper->type == opBoolAnd){
        return 1;
      }else if(oper->type == opBoolOr){
        return 1;
      }
    }
  }

  return 0;
}

/**
 * Checks if operation with booleans is semantically correct
 */
int checkBoolOpSemantics(SToken *leftOperand, SToken *oper, SToken *rightOperand){
  if(oper->type == opBoolNot){
    if(leftOperand->symbol->dataType == dtBool){
      return 1;
    }
  }else if(oper->type == opEq){
    if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){
      return 1;
    }
  }else if(oper->type == opNotEq){
    if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){
      return 1;
    }
  }else if(oper->type == opBoolAnd){
    if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){
      return 1;
    }
  }else if(oper->type == opBoolOr){
    if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){
      return 1;
    }
  }

  return 0;
}

/**
 * Optimalization function - do operation with constants (+, -, *, /, \, + for concat strings and extra =, <> with bool - bool)
 * Always returns filled token, token.type = NT_EXPR, token.symbol->type
 * if everythng is OK, is setted token.symbol->dataType, token.symbol->data
 * else token.symbol->dataType = dtUnspecified and wrong token.symbol->data -> ends program with typeCompatibilityErr
 */
SToken syntx_doArithmeticOp(SToken *leftOperand, SToken *oper, SToken *rightOperand){

  SToken token;
  token.type = NT_EXPR;
  token.symbol = symbt_getUniqeTmpSymb();
  token.symbol->type = symtConstant;
  token.symbol->dataType = dtUnspecified;


  SToken leftOperandCopy = syntx_deepCopyToken(leftOperand);

  if(rightOperand != NULL){

    SToken rightOperandCopy = syntx_deepCopyToken(rightOperand);


    if(leftOperandCopy.symbol->type == symtConstant && rightOperandCopy.symbol->type == symtConstant){  // if operation is possible to do

      // checks dividing by zero
      if(oper->type == opDivFlt || oper->type == opDiv){
        if(rightOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->data.intVal == 0){
          scan_raiseCodeError(anotherSemanticErr, "Dividing by zero integer.", NULL);  // prints error
        }else if(rightOperandCopy.symbol->dataType == dtFloat && rightOperandCopy.symbol->data.doubleVal == 0.0){
          scan_raiseCodeError(anotherSemanticErr, "Dividing by zero double.", NULL);  // prints error
        }
      }

      // by dataType choose right type from union, do implicit conversion and do operation
      if(leftOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->dataType == dtInt){

        token.symbol->dataType = dtInt;

        if(oper->type == opPlus){
          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal + rightOperandCopy.symbol->data.intVal; // adds two integers
        }else if(oper->type == opMns){
          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal - rightOperandCopy.symbol->data.intVal; // subs two integers
        }else if(oper->type == opMul){
          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal * rightOperandCopy.symbol->data.intVal; // muls two integers
        }else if(oper->type == opDivFlt){
          syntx_intToDoubleToken(&leftOperandCopy);
          syntx_intToDoubleToken(&rightOperandCopy);
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal / rightOperandCopy.symbol->data.doubleVal; // float divides two doubles
          token.symbol->dataType = dtFloat; // result/dataType after divide is DOUBLE
        }else if(oper->type == opDiv){
          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal / rightOperandCopy.symbol->data.intVal; // integer divides two integers
        }

      }else if(leftOperandCopy.symbol->dataType == dtFloat && rightOperandCopy.symbol->dataType == dtFloat){  // double - double

        // integer division
        if(oper->type == opDiv){
          // -> int - int
          syntx_doubleToIntToken(&leftOperandCopy);
          syntx_doubleToIntToken(&rightOperandCopy);

          // result after rounding can be zero
          if(rightOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->data.intVal == 0){
            scan_raiseCodeError(anotherSemanticErr, "Dividing by zero integer.", NULL);  // prints error
          }

          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal / rightOperandCopy.symbol->data.intVal; // integer divides two doubles
          token.symbol->dataType = dtInt;
          return token;
        }

        if(oper->type == opPlus){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal + rightOperandCopy.symbol->data.doubleVal; // adds two doubles
        }else if(oper->type == opMns){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal - rightOperandCopy.symbol->data.doubleVal; // subs two doubles
        }else if(oper->type == opMul){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal * rightOperandCopy.symbol->data.doubleVal; // muls two doubles
        }else if(oper->type == opDivFlt){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal / rightOperandCopy.symbol->data.doubleVal; // float divides two doubles
        }

        token.symbol->dataType = dtFloat;

      }else if(leftOperandCopy.symbol->dataType == dtFloat && rightOperandCopy.symbol->dataType == dtInt){  // double - int

        // integer division
        if(oper->type == opDiv){
          syntx_doubleToIntToken(&leftOperandCopy); // -> int - int

          // result after rounding can be zero
          if(rightOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->data.intVal == 0){
            scan_raiseCodeError(anotherSemanticErr, "Dividing by zero integer.", NULL);  // prints error
          }

          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal / rightOperandCopy.symbol->data.intVal; // integer divides two doubles
          token.symbol->dataType = dtInt;
          return token;
        }

        //TODO: again! implicit conversion - return some value or not?
        syntx_intToDoubleToken(&rightOperandCopy); // -> double - double

        if(oper->type == opPlus){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal + rightOperandCopy.symbol->data.doubleVal; // adds two doubles
        }else if(oper->type == opMns){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal - rightOperandCopy.symbol->data.doubleVal; // subs two doubles
        }else if(oper->type == opMul){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal * rightOperandCopy.symbol->data.doubleVal; // muls two doubles
        }else if(oper->type == opDivFlt){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal / rightOperandCopy.symbol->data.doubleVal; // float divides two doubles
        }

        token.symbol->dataType = dtFloat;

      }else if(leftOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->dataType == dtFloat){  // int - double -> double - double



        // integer division
        if(oper->type == opDiv){
          syntx_doubleToIntToken(&rightOperandCopy); // -> int - int

          // result after rounding can be zero
          if(rightOperandCopy.symbol->dataType == dtInt && rightOperandCopy.symbol->data.intVal == 0){
            scan_raiseCodeError(anotherSemanticErr, "Dividing by zero integer.", NULL);  // prints error
          }

          token.symbol->data.intVal = leftOperandCopy.symbol->data.intVal / rightOperandCopy.symbol->data.intVal; // integer divides two doubles
          token.symbol->dataType = dtInt;
          return token;
        }

        //TODO: again! implicit conversion - return some value or not?
        syntx_intToDoubleToken(&leftOperandCopy); // -> double - double

        if(oper->type == opPlus){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal + rightOperandCopy.symbol->data.doubleVal; // adds two doubles
        }else if(oper->type == opMns){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal - rightOperandCopy.symbol->data.doubleVal; // subs two doubles
        }else if(oper->type == opMul){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal * rightOperandCopy.symbol->data.doubleVal; // muls two doubles
        }else if(oper->type == opDivFlt){
          token.symbol->data.doubleVal = leftOperandCopy.symbol->data.doubleVal / rightOperandCopy.symbol->data.doubleVal; // float divides two doubles
        }

        token.symbol->dataType = dtFloat;

      }else if(leftOperandCopy.symbol->dataType == dtString && rightOperandCopy.symbol->dataType == dtString){  // string - string

        if(oper->type == opPlus){
          token.symbol->data.stringVal = util_StrConcatenate(leftOperandCopy.symbol->data.stringVal, rightOperandCopy.symbol->data.stringVal);
          token.symbol->dataType = dtString;
        }

      }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
        if(oper->type == opEq){
          token.symbol->data.boolVal = leftOperand->symbol->data.boolVal == rightOperand->symbol->data.boolVal; // bool = bool
          token.symbol->dataType = dtBool;
        }else if(oper->type == opNotEq){
          token.symbol->data.boolVal = leftOperand->symbol->data.boolVal != rightOperand->symbol->data.boolVal; // bool <> bool
          token.symbol->dataType = dtBool;
        }else if(oper->type == opBoolAnd){
          token.symbol->data.boolVal = leftOperand->symbol->data.boolVal && rightOperand->symbol->data.boolVal; // bool AND bool
          token.symbol->dataType = dtBool;
        }else if(oper->type == opBoolOr){
          token.symbol->data.boolVal = leftOperand->symbol->data.boolVal || rightOperand->symbol->data.boolVal; // bool OR bool
          token.symbol->dataType = dtBool;
        }
      }
    }

    // end of rightOperand != NULL
  } else if(rightOperand == NULL){
    if(leftOperand->symbol->type == symtConstant){  // NOT bool
      if(leftOperand->symbol->dataType == dtBool){
        if(oper->type == opBoolNot){
          token.symbol->data.boolVal = !leftOperand->symbol->data.boolVal;
          token.symbol->dataType = dtBool;
        }
      }
    }
  }// end of rightOperand == NULL

  // function was invoked with wrong arguments - typeCompatibilityErr
  if(token.symbol->dataType == dtUnspecified){
    scan_raiseCodeError(typeCompatibilityErr, "Error during arithmetic operation with two constants.", NULL);  // prints error
  }

  return token;
}

/**
 * Generates int@constant, float@constant, bool@constant, string@constant or variable
 */
void syntx_generateIdent(SToken *token){
  if(token->symbol->type == symtConstant){  // constant
    if(token->symbol->dataType == dtInt){
      printInstruction("int@%d", token->symbol->data.intVal);
    }else if(token->symbol->dataType == dtFloat){
      printInstruction("float@%g", token->symbol->data.doubleVal);
    }else if(token->symbol->dataType == dtString){
      printLongInstruction(strlen(token->symbol->data.stringVal), "string@%s", token->symbol->data.stringVal);
    }else if(token->symbol->dataType == dtBool){
      printInstruction("bool@%s", token->symbol->data.boolVal ? "true" : "false");
    }
  }else if(token->symbol->type == symtVariable){  // variable
    printInstruction("%s", token->symbol->ident);
  }
}

/**
 * Generates instructions
 * op3 can be NULL for two-operands instructions
 */
void syntx_generateInstruction(char *instrName, SToken *op1, SToken *op2, SToken *op3){
      printInstruction("%s ", instrName);
      if (op1 != NULL)
        syntx_generateIdent(op1);
      else
        syntx_generateIdent(op2);

      printInstruction(" ");
      syntx_generateIdent(op2);
      if(op3 != NULL){ //if instruction has only two operands
        printInstruction(" ");
        syntx_generateIdent(op3);
      }
      printInstruction("\n");
}

/**
 * Generates instructions where first argument in instruction is not Token but pointer to char
 * op3 can be NULL for two-operands instructions
 */
void syntx_generateInstructionFstPosStr(char *instrName, char *op1, SToken *op2, SToken *op3){
      printInstruction("%s ", instrName);
      printInstruction("%s ", op1);
      printInstruction(" ");
      syntx_generateIdent(op2);
      if(op3 != NULL){ //if instruction has only two operands
        printInstruction(" ");
        syntx_generateIdent(op3);
      }
      printInstruction("\n");
}

/**
 * Generates instructions where second argument in instruction is not Token but pointer to char
 * op3 can be NULL for two-operands instructions
 */
void syntx_generateInstructionSecPosStr(char *instrName, SToken *op1, char *op2, SToken *op3){
      printInstruction("%s ", instrName);
      syntx_generateIdent(op1);
      printInstruction(" ");
      printInstruction("%s ", op2);
      if(op3 != NULL){ //if instruction has only two operands
        printInstruction(" ");
        syntx_generateIdent(op3);
      }
      printInstruction("\n");
}

/**
 * Generates code for basic operations
 */
void syntx_generateCodeForBasicOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){

  switch(operator->type){
    case opPlus:
    case opMns:
    case opMul:
    case opDivFlt:
    case opDiv:
      break;
    default:
      return;
  }

  if (partialResult == NULL)
    partialResult = leftOperand;
  else
    partialResult->symbol->dataType = leftOperand->symbol->dataType;

  switch(operator->type){
    case opPlus:
      if(leftOperand->symbol->dataType != dtString && rightOperand->symbol->dataType != dtString){ // if doesn't concatenates two strings
        syntx_generateInstruction("ADD", partialResult, leftOperand, rightOperand);
      }else{
        syntx_generateInstruction("CONCAT", partialResult, leftOperand, rightOperand);  // string + string
      }
      break;
    case opMns:
      syntx_generateInstruction("SUB", partialResult, leftOperand, rightOperand);
      break;
    case opMul:
      syntx_generateInstruction("MUL", partialResult, leftOperand, rightOperand);
      break;
    case opDivFlt:
      syntx_generateInstruction("DIV", partialResult, leftOperand, rightOperand);
      break;
    case opDiv:
      syntx_generateInstruction("DIV", partialResult, leftOperand, rightOperand);
      syntx_generateInstruction("FLOAT2INT", partialResult, partialResult, NULL);
      partialResult->symbol->dataType = dtInt;
      break;
    default:
      return;
  }
}

/**
 * Generates code for boolean operations
 */
void syntx_generateCodeForBoolOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){

  switch(operator->type){
    case opBoolAnd:
    case opBoolOr:
    case opBoolNot:
      break;
    default: return;
  }

  if (partialResult == NULL)
    partialResult = leftOperand;
  else
    partialResult->symbol->dataType = leftOperand->symbol->dataType;

  switch(operator->type){
    case opBoolAnd:
      syntx_generateInstruction("AND", partialResult, leftOperand, rightOperand);
      break;
    case opBoolOr:
      syntx_generateInstruction("OR", partialResult, leftOperand, rightOperand);
      break;
    case opBoolNot:
      syntx_generateInstruction("NOT", partialResult, leftOperand, NULL);
      break;
    default:
      return;
  }
}

/**
 * Generates code for assign operations
 */
void syntx_generateCodeForAsgnOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){

  switch(operator->type){
    case asgn:
    case opPlusEq:
    case opMnsEq:
    case opMulEq:
    case opDivFltEq:
    case opDivEq:
      break;
    default: return;
  }

  if (partialResult == NULL)
    partialResult = leftOperand;
  else
    partialResult->symbol->dataType = leftOperand->symbol->dataType;

  switch(operator->type){
    case asgn:
      syntx_generateInstruction("MOVE", leftOperand, rightOperand, NULL);
      break;
    case opPlusEq:
      if(leftOperand->symbol->dataType != dtString && rightOperand->symbol->dataType != dtString){ // if doesn't concatenates two strings
        syntx_generateInstruction("ADD", partialResult, leftOperand, rightOperand);
      }else{
        syntx_generateInstruction("CONCAT", partialResult, leftOperand, rightOperand);  // string + string
      }
      break;
    case opMnsEq:
      syntx_generateInstruction("SUB", partialResult, leftOperand, rightOperand);
      break;
    case opMulEq:
      syntx_generateInstruction("MUL", partialResult, leftOperand, rightOperand);
      break;
    case opDivFltEq:
      syntx_generateInstruction("DIV", partialResult, leftOperand, rightOperand);
      break;
    case opDivEq: // division integer by integer
      syntx_generateInstruction("DIV", partialResult, leftOperand, rightOperand);
      syntx_generateInstruction("FLOAT2INT", partialResult, partialResult, NULL);
      partialResult->symbol->dataType = dtInt;
      break;
    default:
      return;
  }
}

/**
 * Generates code for relation operations
 */
void syntx_generateCodeForRelOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
  switch(operator->type){
    case opLes: // <
      syntx_generateInstruction("LT", partialResult, leftOperand, rightOperand);
      break;
    case opGrt: // >
      syntx_generateInstruction("GT", partialResult, leftOperand, rightOperand);
      break;
    case opLessEq:  // <=
      syntx_generateInstruction("GT", partialResult, leftOperand, rightOperand);
      syntx_generateInstruction("NOT", partialResult, partialResult, NULL);
      break;
    case opGrtEq: // >=
      syntx_generateInstruction("LT", partialResult, leftOperand, rightOperand);
      syntx_generateInstruction("NOT", partialResult, partialResult, NULL);
      break;
    case opEq:  // =
      syntx_generateInstruction("EQ", partialResult, leftOperand, rightOperand);
      break;
    case opNotEq: // <>
      syntx_generateInstruction("EQ", partialResult, leftOperand, rightOperand);
      syntx_generateInstruction("NOT", partialResult, partialResult, NULL);
      break;
    default:
      return;
  }

  if(partialResult != NULL){
    //Sets partialResult (result from comparative operation) data type to boolean
    partialResult->symbol->dataType = dtBool;
  }
}

 /**
  * Generates code for variable definition
  *
  * funcToken represents function token
  * argIndex represents argument number from beginning
  * argValue represents constant or variable transmitted to the function
  */
 void syntx_generateCodeForVarDef(SToken *funcToken, int argIndex, SToken *argValue){

  TArgList args = funcToken->symbol->data.funcData.arguments;

   if(args->count <= argIndex){  // too many arguments
     scan_raiseCodeError(typeCompatibilityErr, "Too many arguments passed to function.", NULL);
   }
   // check argument data type
   if(args->get(args, argIndex)->dataType == dtFloat && argValue->symbol->dataType == dtInt){ // double - int -> double - double
     syntx_intToDoubleToken(argValue);
   }else if(args->get(args, argIndex)->dataType == dtInt && argValue->symbol->dataType == dtFloat){ // int - double -> int - int
     syntx_doubleToIntToken(argValue);
   }else if(args->get(args, argIndex)->dataType != argValue->symbol->dataType){
     scan_raiseCodeError(typeCompatibilityErr, "Argument passed to function has wrong data type.", argValue);  // prints error
   }

   printInstruction("PUSHS ");
   syntx_generateIdent(argValue);
   printInstruction("\n");
 }

 /**
  * Generates code for function call, moves return value, checks arguments count and sets data type of result
  *
  * funcToken represents function token
  * argIndex represents argument number from beginning
  * result updates result variable data type
  */
 void syntx_generateCodeForCallFunc(SToken *funcToken, int argIndex, SToken *result){
  TArgList args = funcToken->symbol->data.funcData.arguments;
   // checks arguments count
  if(args->count > argIndex){  // too few arguments
     scan_raiseCodeError(typeCompatibilityErr, "Too few arguments passed to function.", NULL);
  }

  printInstruction("PUSHFRAME\n");
  printInstruction("CREATEFRAME\n");

  for(int i = argIndex-1; i >= 0; i--)
  {
    char *argIdent = util_StrConcatenate("TF@", args->get(args, i)->ident);
    printInstruction("DEFVAR %s\n", argIdent);
    printInstruction("POPS %s\n", argIdent);
    mmng_safeFree(argIdent);
  }

  printInstruction("CALL %s\n", funcToken->symbol->data.funcData.label);
  printInstruction("PUSHS TF@%%retval\n");
  printInstruction("POPFRAME\n");
  printInstruction("POPS %s\n", result->symbol->ident);
  // sets correct token data type corresponding to function return value
  result->symbol->dataType = funcToken->symbol->data.funcData.returnType;
 }

/**
 * Main function for code generation
 * leftOperand expects constant or ident
 * operator expects one of available operators, otherwise throws apperr_runtimeError (in subfunction)
 * rightOperand expects constant, ident or NULL in case boolean NOT operator
 * partialResult reference variable to return result from part of expression back to the SyntaxAnalyzer - MUST BE VARIABLE!
 */
void syntx_generateCode(SToken *leftOperand, SToken *oper, SToken *rightOperand, SToken *partialResult){

  SToken *rightCopiedTokenAddr = NULL;
  SToken rightOperandCopy;

  if(rightOperand != NULL){
    rightOperandCopy = syntx_deepCopyToken(rightOperand);
    rightCopiedTokenAddr = &rightOperandCopy;
  }

  if (partialResult == NULL)
    partialResult = leftOperand;

  SToken leftOperandCopy = syntx_deepCopyToken(leftOperand);

  // checks data types, implicitly converts constants and generates code for implicit convertion of variables
  if(rightCopiedTokenAddr != NULL && oper->type != opBoolNot){  // operator is not bool NOT
    syntx_checkDataTypes(&leftOperandCopy, oper, rightCopiedTokenAddr);
  }else if(rightCopiedTokenAddr == NULL && oper->type == opBoolNot){  // operator is bool NOT
    syntx_checkDataTypeOfBool(&leftOperandCopy);
  }else{  //for example: NOT string, NOT float, etc.
    scan_raiseCodeError(typeCompatibilityErr, "An attempt to make a logical negation operation with non-bool data type.", NULL);  // prints error
  }

  // here are all data in right form

  // NOTICE: data type for partialResult is setted in generateCodexxx functions!

  // one of functions bellow prints instructions according to operator type
  syntx_generateCodeForBasicOps(&leftOperandCopy, oper, rightCopiedTokenAddr, partialResult);  // +, -, *, /, \, string +
  syntx_generateCodeForBoolOps(&leftOperandCopy, oper, rightCopiedTokenAddr, partialResult); // AND, OR, NOT
  syntx_generateCodeForAsgnOps(&leftOperandCopy, oper, rightCopiedTokenAddr, partialResult);  // +=, -=, *=, /=, \=, asgn
  syntx_generateCodeForRelOps(&leftOperandCopy, oper, rightCopiedTokenAddr, partialResult);  // <, >, <=, >=, =, <>
}