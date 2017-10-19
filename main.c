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
  
  iStack stack = ist_init();
  for (int i = 1; i < 10; i++)
  {
    stack->push(stack, i);
  }

  for (int i = 1; i < 10; i++)
  {
    printf("top: ");
    printf("%d\n", stack->top(stack));
    stack->pop(stack);
  }

  stack->destruct(stack);
  mmng_freeAll();
                         
  
  return 0;
}
