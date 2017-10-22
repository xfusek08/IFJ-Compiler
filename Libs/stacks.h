/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    stacks.c
* \brief   Stack library
*
* Provide implementation of various stacks.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    18.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef STACKSLIB
#define STACKSLIB

#include "grammar.h"
#define STACK_INITIAL_SIZE 30


typedef struct grammarStack *TGrStack;

/**
* Grammar Stack
*
* Each item contains one grammar enum variable.
*/
struct grammarStack{
  /** insert value on top */
  void (*push)(TGrStack , Egrammar);
  /** remove item on top */
  void (*pop)(TGrStack);
  /** returns data of item on top */
  Egrammar (*top)(TGrStack);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void(*destruct)(TGrStack);

  int count;
  int size;
  int *stack;
};

typedef struct pointerStack *TPStack;
/**
* Pointer stack
*/
struct pointerStack{
  /** insert value on top */
  void (*push)(TPStack *, void *);
  /** remove item on top */
  void (*pop)(TPStack *);
  /** returns data of item on top */
  void *(*top)(TPStack *);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void (*destruct)(TPStack *);
  
  int count;
  int size;
  int *stack;
};

/**
* Initialize empty integer stack and returns its pointer.
*/
TGrStack TGrStack_init();

/**
* Initialize empty pointer stack and returns its pointer.
*/
TPStack TPStack_init();

#endif // !STACKSLIB