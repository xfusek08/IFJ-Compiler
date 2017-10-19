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

#define STACK_INITIAL_SIZE 30


typedef struct integerStack *iStack;

/**
* Integer stack
*/
struct integerStack{
  /** insert value on top */
  void (*push)(iStack , int);
  /** remove item on top */
  void (*pop)(iStack);
  /** returns data of item on top */
  int (*top)(iStack);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void(*destruct)(iStack);

  int count;
  int size;
  int *stack;
};

typedef struct pointerStack pStack;
/**
* Pointer stack
*/
struct pointerStack{
  /** insert value on top */
  void (*push)(pStack *, void *);
  /** remove item on top */
  void (*pop)(pStack *);
  /** returns data of item on top */
  void *(*top)(pStack *);
  /** safe destruction of stack. If stack is not empty, throw error. */
  void (*destruct)(pStack *);
  
  int count;
  int size;
  int *stack;
};

/**
* Initialize empty integer stack and returns its pointer.
*/
iStack ist_init();

/**
* Initialize empty pointer stack and returns its pointer.
*/
pStack *pst_init();

#endif // !STACKSLIB