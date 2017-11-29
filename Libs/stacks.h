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
#include "scanner.h"

/*Capacity of newly initialized stack*/
#define STACK_INITIAL_SIZE 30
/*Indicates how much space stack realloc when its full.
new_size = old_size + old_size * STACK_REALLOC_MULTIPLIER)
*/
#define STACK_REALLOC_MULTIPLIER 2


typedef struct tokenListItem {
  struct tokenListItem *prev;
  struct tokenListItem *next;
  SToken token;
}TTkListItem;

typedef struct tokenList *TTkList;

/**
* Token list
*
* Each item contains one token.
*/
struct tokenList{
  /** insert token at the end */
  void(*insertLast)(TTkList, SToken *);
  /** remove token at the end */
  void(*deleteLast)(TTkList);
  /** returns pointer to last token */
  SToken *(*getLast)(TTkList);
  /** returns non zero value, if list is empty. */
  int(*isEmpty)(TTkList);
  /** set last item as active */
  void(*activate)(TTkList);
  /** set previous item as active*/
  void(*prev)(TTkList);
  /** set next item as active*/
  void(*next)(TTkList);
  /** returns pointer to active token */
  SToken *(*getActive)(TTkList);
  /** insert token after active item */
  void(*postInsert)(TTkList, SToken *);
  /** delete token after active item */
  void(*postDelete)(TTkList);
  /** delete token before active item */
  void(*preDelete)(TTkList);
  /** safe destruction of list. If list is not empty, throw error. */
  void(*destroy)(TTkList);
  TTkListItem *first;
  TTkListItem *last;
  TTkListItem *active;
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
* Initialize empty token list returns its pointer.
*/
TTkList TTkList_create();

/**
 * Debug function for printing TTkList.
 */
void TTkList_print(TTkList);

/**
 * Debug function for printing EGrSymb enum.
 */
void printEgr(EGrSymb symb);
/**
* Initialize empty pointer stack and returns its pointer.
*/
TPStack TPStack_create();

#endif // !_STACKSLIB