/******************************************************************************/
/**
* \project IFJ-Compiler
* \file    stacks.c
* \brief   Stack library
* \author  Pavel Vosyka (xvosyk00)
* \date    24.10.2017 - Pavel Vosyka
*/
/******************************************************************************/

#include "stacks.h"
#include "MMng.h"
#include "grammar.h"
#include "appErr.h"
#include <stdlib.h>
#include <stdio.h>

// =============================================================================
// ====================== support functions ====================================
// =============================================================================
/****** GRAMMAR STACK ********/

void ist_push(TGrStack stack, EGrSymb val)
{
  if (stack->count == stack->size)
  {
    stack->grArray = mmng_safeRealloc(stack->grArray, stack->size * sizeof(EGrSymb) + stack->size * sizeof(EGrSymb) * STACK_REALLOC_MULTIPLIER);
  }
  stack->grArray[stack->count++] = val;
}

void ist_pop(TGrStack stack)
{
  if (stack->count == 0)
  {
    apperr_runtimeError("TGrStack: Trying to pop empty stack!");
  }
  stack->count--;
}

EGrSymb ist_top(TGrStack stack)
{
  if (stack->count == 0)
  {
    apperr_runtimeError("TGrStack: Trying read from empty stack!");
  }
  return stack->grArray[stack->count-1];
}

void ist_destroy(TGrStack stack)
{
  if (stack->count != 0)
  {
    apperr_runtimeError("TGrStack: Trying to destroy non-empty stack!");
  }
  mmng_safeFree(stack->grArray);
  mmng_safeFree(stack);
}

/****** POINTER STACK ********/
void pst_push(TPStack stack, void *val)
{
  if (stack->count == stack->size)
  {
    stack->ptArray = mmng_safeRealloc(stack->ptArray, stack->size * sizeof(void *) + stack->size * sizeof(void *) * STACK_REALLOC_MULTIPLIER);
  }
  stack->ptArray[stack->count++] = val;
}

void pst_pop(TPStack stack)
{
  if (stack->count == 0)
  {
    apperr_runtimeError("TPStack: Trying to pop empty stack!");
  }
  stack->count--;
}

void *pst_top(TPStack stack)
{
  if (stack->count == 0)
  {
    apperr_runtimeError("TPStack: Trying read from empty stack!");
  }
  return stack->ptArray[stack->count - 1];
}

void pst_destroy(TPStack stack)
{
  if (stack->count != 0)
  {
    apperr_runtimeError("TPStack: Trying to destroy non-empty stack!");
  }
  mmng_safeFree(stack->ptArray);
  mmng_safeFree(stack);
}


// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

TGrStack TGrStack_create()
{
  TGrStack stack = mmng_safeMalloc(sizeof(struct grammarStack));
  stack->grArray = mmng_safeMalloc(sizeof(EGrSymb)*STACK_INITIAL_SIZE);
  stack->size = STACK_INITIAL_SIZE;
  stack->count = 0;
  stack->push = ist_push;
  stack->pop = ist_pop;
  stack->top = ist_top;
  stack->destroy = ist_destroy;
  return stack;
}

void TGrStack_postInsert(TGrStack stack, int position, EGrSymb symbol)
{
  if (stack->count == stack->size)
  {
    stack->push(stack, eol); //becouse full stack is handled in push function
    stack->pop(stack);
  }
  for (int i = position+1; i < stack->count; i++)
  {
    stack->grArray[i + 1] = stack->grArray[i];
  }
  stack->count++;
  stack->grArray[position + 1] = symbol;
}

TPStack TPStack_create()
{
  TPStack stack = mmng_safeMalloc(sizeof(struct pointerStack));
  stack->ptArray = mmng_safeMalloc(sizeof(void *)*STACK_INITIAL_SIZE);
  stack->size = STACK_INITIAL_SIZE;
  stack->count = 0;
  stack->push = pst_push;
  stack->pop = pst_pop;
  stack->top = pst_top;
  stack->destroy = pst_destroy;
  return stack;
}