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
  if(stackSymb > 25 || inputSymb > 25){
    return 0;
  }

  EGrSymb precTable[25][25] = {
             /* + */   /* - */  /* * */   /* / */  /* \ */  /* ( */  /* ) */  /* i */  /* , */  /* = */  /* <> */         /* < */  /* <= */ /* > */  /* >= */ /* += */ /* -= */ /* *= */ /* /= */ /* \= */ /* := */ /* AND*/ /* OR */ /* NOT*/ /* $ */
    /* +  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* +  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* -  */ {precGrt, precGrt, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* -  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* *  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* *  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* /  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* /  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* \  */ {precGrt, precGrt, precLes, precLes, precGrt, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* \  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* (  */ {precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precEqu, precLes, precLes, /* (  */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precUnd},
    /* )  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* )  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* i  */ {precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precGrt, precUnd, precGrt, precGrt, precGrt, /* i  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* ,  */ {precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precEqu, precLes, precLes, /* ,  */ precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precLes},
    /* =  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* =  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* <> */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <> */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* <  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* <= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* <= */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* >  */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >  */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* >= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precGrt, precGrt, /* >= */ precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* += */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* += */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* -= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* -= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* *= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* *= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* /= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* /= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* \= */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* \= */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* := */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* := */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* AND*/ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* AND*/ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* OR */ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* OR */ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* NOT*/ {precLes, precLes, precLes, precLes, precLes, precLes, precGrt, precLes, precGrt, precLes, precLes, /* NOT*/ precLes, precLes, precLes, precLes, precGrt, precGrt, precGrt, precGrt, precGrt, precGrt, precUnd, precUnd, precUnd, precGrt},
    /* $  */ {precLes, precLes, precLes, precLes, precLes, precLes, precUnd, precLes, precLes, precLes, precLes, /* $  */ precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precLes, precUnd, precUnd, precUnd, precUnd}
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
    printf("INT2FLOAT %s %s\n", token->symbol->ident, token->symbol->ident);
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
    printf("FLOAT2R2EINT %s %s\n", token->symbol->ident, token->symbol->ident); // half to even
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
int syntx_checkDataTypesOfDiv(SToken *leftOperand, SToken *rightOperand){
  // TODO: really cast here - doubleToInt?
  if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtInt){  // int - int
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> int - int

    syntx_doubleToIntToken(leftOperand);
    return 1;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> int - int

    syntx_doubleToIntToken(rightOperand);
    return 1;
  }

  return 0; //other combinations are not allowed

}

/**
 * Checks data types of two operands of assignment operators
 * Returns 1 if everything is OK, otherwise 0
 */
int syntx_checkDataTypesOfAgnOps(SToken *leftOperand, SToken *rightOperand){
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
    return 1;
  }else if(leftOperand->symbol->dataType == dtBool && rightOperand->symbol->dataType == dtBool){  // bool - bool
    return 1;
  }else if(leftOperand->symbol->dataType == dtFloat && rightOperand->symbol->dataType == dtInt){  // double - int -> double - double

    syntx_intToDoubleToken(rightOperand);
    return 1;

  }else if(leftOperand->symbol->dataType == dtInt && rightOperand->symbol->dataType == dtFloat){  // int - double -> int - int

    syntx_doubleToIntToken(rightOperand);
    return 1;
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
 * Generates int@constant, float@constant, bool@constant, string@constant or variable
 */
void syntx_generateIdent(SToken *token){

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
 * rightOperand could be NULL for two-operands instructions
 */
void syntx_generateInstruction(char *instrName, SToken *partialResult, SToken *leftOperand, SToken *rightOperand){
      printf("%s ", instrName);
      syntx_generateIdent(partialResult);
      printf(" ");
      syntx_generateIdent(leftOperand);
      if(rightOperand != NULL){ //if instruction has only two operands
        printf(" ");
        syntx_generateIdent(rightOperand);
      }
      printf("\n");
}

/**
 * Generates code for basic operations
 */
void syntx_generateCodeForBasicOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
  
  switch(operator->type){
    case opPlus:
      if(leftOperand->symbol->dataType != dtString && leftOperand->symbol->dataType != dtString){ // if doesn't concatenates two strings
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
    default:
      break;
  }
}

/**
 * Generates code for boolean operations
 */
void syntx_generateCodeForBoolOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
  
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
      break;
  }
}

/**
 * Generate code for boolean operations
 */
// void syntx_generateCodeForBoolOps(SToken *leftOperand, SToken *operator, SToken *rightOperand, SToken *partialResult){
//   printf("CREATEFRAME\n");

// }


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

  // one of functions bellow prints instructions by operator type
  syntx_generateCodeForBasicOps(leftOperand, operator, rightOperand, partialResult);
  syntx_generateCodeForBoolOps(leftOperand, operator, rightOperand, partialResult);

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
  sprintf(ident, "LF@%%T%d", nextTokenIdent);
  //if not defined, define ident
  if (symbt_findSymb(ident) == NULL)
  {
    symbt_insertSymbOnTop(ident);
    printf("DEFVAR %s\n", ident);
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
    if (list->active->next == NULL)
      return 0;
    if (list->active->next->next == NULL)
      return 0;
    SToken *arg1 = &list->active->token;
    SToken *arg2 = &list->active->next->token;
    SToken *arg3 = &list->active->next->next->token;
    if (arg1->symbol->type == symtConstant && arg3->symbol->type == symtConstant)
    {
      SToken t = syntx_doArithmeticOp(arg1, arg2, arg3);
      list->postInsert(list, &t);
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
      SToken ref_var;
      ref_var = sytx_getFreeVar();
      syntx_generateCode(arg1, arg2, arg3, &ref_var);
      list->postDelete(list);
      list->postDelete(list);
      list->postInsert(list, &ref_var);
      list->next(list);
      list->preDelete(list);
    }
    break;
  case kwNot:
    if (list->active->next == NULL)
      return 0;
    SToken ref_var;
    ref_var = sytx_getFreeVar();
    syntx_generateCode(&list->active->token, &list->active->next->token, NULL, &ref_var);
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

/**
* Precedent statement analyze
*/
TSymbol syntx_processExpression(SToken *actToken, TSymbol symbol)
{
  nextTokenIdent = 0; //reset identificator generator
  EGrSymb terminal;
  do {
     terminal = syntx_getFirstTerminal(tlist);

    //debug print
    DDPRINT("Analyzing token: %d", actToken->type);
    DDPRINT("First terminal on stack: %d", terminal);

    if (actToken->symbol != NULL)
    {
      switch (actToken->symbol->type)
      {
      case symtFuction:
        //TODO: handle function identifier somehow
        break;
      case symtVariable:
      case symtConstant:
        //ok
        break;
      case symtUnknown:
        fprintf(stderr, "Error: Symbol was not defined.");
        scan_raiseCodeError(syntaxErr);
        break;
      }
    }
    EGrSymb tablesymb;
    if (!syntx_getPrecedence(terminal, actToken->type, &tablesymb))
    {
      DPRINT("Table: undefined precedence!.");
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
      {
        SToken auxToken;
        auxToken.type = precLes;
        tlist->postInsert(tlist, &auxToken);
        tlist->insertLast(tlist, actToken);
        *actToken = nextToken();
      }
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
    getchar(); //debug
  } while (!(terminal == eol && actToken->type == eol) && actToken->type <= 24);
  DPRINT("konec vyrazu.");

  if(!(tlist->first != NULL && tlist->first->next != NULL && tlist->first->next->next == NULL))
  {
    scan_raiseCodeError(syntaxErr);
  }

  //free ident stack
  syntx_emptyVarStack();
  SToken resultToken = tlist->last->token;
  
  if (symbol == NULL)
  {
    //return temporary variable with result
    tlist->deleteLast(tlist);
    return resultToken.symbol;
  }
  else {
    //mov result from temporary variable to requested variable
    SToken retT;
    retT.dataType = NT_EXPR;
    retT.symbol = symbol;
    syntx_generateInstruction("MOV", &retT, &resultToken, NULL);
    tlist->deleteLast(tlist);
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


// test function
void syntx_testFunction(){
  //char * var1Name = "LF@var1";
  //char * var2Name= "LF@var2";
  //char * var3Name = "LF@temp";
  char *testingString = "promenna";


  SToken token1;
  token1.type = NT_EXPR;
  token1.symbol = mmng_safeMalloc(sizeof(struct Symbol));
  token1.symbol->type = symtConstant;
  token1.symbol->data.stringVal = testingString;
  //token1.symbol->type = symtVariable;
  //token1.symbol->ident = var1Name;
  token1.symbol->dataType = dtString;

  SToken oper;
  oper.type = NT_EXPR;
  oper.type = opPlus;

  SToken token2;
  token2.type = NT_EXPR;
  token2.symbol = mmng_safeMalloc(sizeof(struct Symbol));
  token2.symbol->type = symtConstant;
  token2.symbol->data.stringVal = "ijuuhrg";
  //token2.symbol->type = symtVariable;
  //token2.symbol->ident = var2Name;
  token2.symbol->dataType = dtString;

  SToken token3;
  token3.type = NT_EXPR;
  token3.symbol = mmng_safeMalloc(sizeof(struct Symbol));
  token3.symbol->type = symtConstant;
  token3.symbol->data.intVal = 1;
  //token3.symbol->type = symtVariable;
  //token3.symbol->ident = var3Name;
  token3.symbol->dataType = dtInt;

  syntx_generateCode(&token1, &oper, &token2, &token3);
}

