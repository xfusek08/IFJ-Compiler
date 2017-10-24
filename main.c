/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    3.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Libs/MMng.h"
#include "Libs/stacks.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  // ... spousta mallocu a frees ...
  
  TGrStack stack = TGrStack_create();
  for (int i = 1; i < 10; i++)
  {
    stack->push(stack, placeholder);
  }

  for (int i = 1; i < 10; i++)
  {
    printf("top: ");
    printf("%d\n", stack->top(stack));
    stack->pop(stack);
  }

  stack->destroy(stack);

  printf("-------Pointer tack\n");
  int a = 1;
  int b = 2;
  int c = 3;
  TPStack pstack = TPStack_create();
  pstack->push(pstack, &a);
  pstack->push(pstack, &b);
  pstack->push(pstack, &c);

  for (int i = 1; i < 4; i++)
  {
    printf("top: ");
    printf("%d\n", *(int *)(pstack->top(pstack)));
    pstack->pop(pstack);
  }

  pstack->destroy(pstack);

  mmng_freeAll();
                         
  
  return 0;
}
