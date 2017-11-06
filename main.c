/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    Main.c
 * \brief   Main program
 * \author  Petr Fusek (xfusek08)
 * \date    30.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "Libs/MMng.h"
#include "Libs/stacks.h"
#include "Libs/symtable.h"

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  mmng_init();
  symbt_init();
  char ident[3];
  ident[2] = '\0';
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      ident[0] = '0' + i;
      ident[1] = '0' + j;
      printf("inserting: %s\n", ident);
      symbt_findOrInsertSymb(ident);
    }
  }

  printf("\n");
  symbt_print();
  printf("\n");

  // lets delete all even umbers
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j+=2)
    {
      ident[0] = '0' + i;
      ident[1] = '0' + j;
      printf("deleting: %s\n", ident);
      symbt_deleteSymb(ident);
    }
  }

  printf("\n");
  symbt_print();
  printf("\n");

  printf("Push frame\n");
  symbt_pushFrame();

  printf("Before insert\n");
  symbt_print();
  printf("\n");
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      ident[0] = '0' + i;
      ident[1] = '0' + j;
      printf("inserting: %s\n", ident);
      symbt_findOrInsertSymb(ident);
    }
  }
  printf("Afterinsert\n");
  symbt_print();
  printf("\n");

  printf("Push frame\n");
  symbt_pushFrame();

  printf("\n");
  symbt_print();
  printf("\n");

  printf("Pop frame\n");
  symbt_popFrame();
  printf("\n");
  symbt_print();
  printf("\n");

  printf("Pop frame\n");
  symbt_popFrame();
  printf("\n");
  symbt_print();
  printf("\n");

  printf("Pop frame\n");
  symbt_popFrame();
  printf("\n");
  symbt_print();
  printf("\n");

  symbt_destroy();
  mmng_freeAll();
  return 0;
}
