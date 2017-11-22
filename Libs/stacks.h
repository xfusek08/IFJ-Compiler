/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    stacks.c
* \brief   Stack library
*
* Provide implementation of various stacks.
*
* \author  Pavel Vosyka (xvosyk00)
* \date    24.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#ifndef _STACKSLIB
#define _STACKSLIB

#include "grammar.h"
/*Capacity of newly initialized stack*/
#define STACK_INITIAL_SIZE 30
/*Indicates how much space stack realloc when its full.
new_size = old_size + old_size * STACK_REALLOC_MULTIPLIER)
*/
#define STACK_REALLOC_MULTIPLIER 2


typedef struct grammarStack *TGrStack;

/**
* Grammar Stack
*
* Each item contains one grammar enum variable.
*/
struct grammarStack{
  /** insert value on top */
  void (*push)(TGrStack , EGrSymb);
  /** remove item on top */
  void (*pop)(TGrStack);
  /** returns data of item on top */
  EGrSymb (*top)(TGrStack);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void(*destroy)(TGrStack);
  /** Number of items in stack. */
  int count;
  /** Capacity of stack. */
  int size;
  EGrSymb *grArray;
};

typedef struct pointerStack *TPStack;
/**
* Pointer stack
*/
struct pointerStack{
  /** insert value on top */
  void (*push)(TPStack, void *);
  /** remove item on top */
  void (*pop)(TPStack);
  /** returns data of item on top */
  void *(*top)(TPStack);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void (*destroy)(TPStack);
  /** Number of items in stack. */
  int count;
  /** Capacity of stack. */
  int size;
  void **ptArray;
};

/**
* Initialize empty integer stack and returns its pointer.
*/
TGrStack TGrStack_create();

/**
* Initialize empty pointer stack and returns its pointer.
*/
TPStack TPStack_create();

#endif // !_STACKSLIB