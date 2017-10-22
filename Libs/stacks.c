/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    stacks.c
* \brief   Stack library
* \author  Pavel Vosyka (xvosyk00)
* \date    18.10.2017 - Pavel Vosyka
* \todo implement pointer stack
* \todo make enum stack from integer stack
*/
/******************************************************************************/

#include "stacks.h"
#include "MMng.h"
#include <stdlib.h>
#include <stdio.h>

/****** INTEGER STACK ********/

void ist_push(TGrStack stack, int val)
{
  if (stack->count == stack->size)
  {
    //TODO: realloc
    fprintf(stderr, "stack realloc not implemented yet.\n");
    exit(1);
  }
  stack->stack[stack->count++] = val;
}

void ist_pop(TGrStack stack)
{
  if (stack->count == 0)
  {
    fprintf(stderr, "trying pop empty stack!\n");
    exit(1);
  }
  stack->count--;
}

int ist_top(TGrStack stack)
{
  if (stack->count == 0)
  {
    fprintf(stderr, "trying read from empty stack!\n");
    exit(1);
  }
  return stack->stack[stack->count-1];
}

void ist_destruct(TGrStack stack)
{
  if (stack->count != 0)
  {
    fprintf(stderr, "Trying to destruct non-empty stack!\n");
    exit(1);
  }
  mmng_safeFree(stack->stack);
  mmng_safeFree(stack);
}

TGrStack TGrStack_init()
{
  TGrStack stack = mmng_safeMalloc(sizeof(struct integerStack));
  stack->stack = mmng_safeMalloc(sizeof(int)*STACK_INITIAL_SIZE);
  stack->size = STACK_INITIAL_SIZE;
  stack->count = 0;
  stack->push = ist_push;
  stack->pop = ist_pop;
  stack->top = ist_top;
  stack->destruct = ist_destruct;
  return stack;
}


/****** POINTER STACK ********/