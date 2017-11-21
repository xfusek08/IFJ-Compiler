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

#define LISTP(x, y) fprintf(stderr, x); fprintf(stderr, "\n"); TTkList_print(y)

// =============================================================================
// ====================== support functions ====================================
// =============================================================================
/****** TOKEN STACK ********/

void tknl_insertLast(TTkList list, SToken *token)
{
  TTkListItem *item = mmng_safeMalloc(sizeof(TTkListItem));
  item->next = NULL;
  item->token = *token;
  if (list->last != NULL)
  {
    item->prev = list->last;
    list->last->next = item;
  }
  else {
    list->first = item;
    list->last = NULL;
  }
  list->last = item;
  LISTP("TTkList: called insertLast()", list);
}

void tknl_deleteLast(TTkList list)
{
  if (list->last == NULL)
  {
    apperr_runtimeError("TTkList: Trying to delete item from empty list!");
  }
  TTkListItem *item = list->last;
  if (list->last->prev != NULL)
  {
    list->last->prev->next = NULL;
  }
  else if(list->last == list->first){
    list->first = NULL;
  }
  list->last = list->last->prev;
  free(item);
  LISTP("TTkList: called deleteLast()", list);
}

int tknl_isEmpty(TTkList list)
{
  if (list->first != NULL)
    return 1;
  return 0;
}

SToken *tknl_getLast(TTkList list)
{
  if (list->last == NULL)
  {
    apperr_runtimeError("TTkList: Trying read from empty list!");
  }
  return &list->last->token;
}

void tknl_destroy(TTkList list)
{
  if (list->first != NULL)
  {
    apperr_runtimeError("TTkList: Trying to destroy non-empty list!");
  }
  mmng_safeFree(list);
}

void tknl_activate(TTkList list)
{
  list->active = list->last;
  LISTP("TTkList: called activate()", list);
}

void tknl_prev(TTkList list)
{
  list->active = list->active->prev;
  LISTP("TTkList: called prev()", list);
}

void tknl_next(TTkList list)
{
  list->active = list->active->next;
  LISTP("TTkList: called next()", list);
}

SToken *tknl_getActive(TTkList list)
{
  if(list->active != NULL)
    return &list->active->token;
  return NULL;
}

void tknl_postInsert(TTkList list, SToken *token)
{
  if (list->active != NULL)
  {
    TTkListItem *newitem = mmng_safeMalloc(sizeof(TTkListItem));
    newitem->token = *token;
    newitem->prev = list->active;
    if (list->active == list->last)
    {
      list->last = newitem;
    }
    else {
      list->active->next->prev = newitem;
    }
    newitem->next = list->active->next;
    list->active->next = newitem;
  }
  else {
    apperr_runtimeError("TTkList: Trying to postInsert on inactive list!");
  }
  LISTP("TTkList: called postInsert()", list);
}

void tknl_postDelete(TTkList list)
{
  if (list->active != NULL && list->active->next != NULL)
  {
    TTkListItem *item = list->active->next;
    if (list->active->next->next == NULL)
    {
      list->last = list->active;
    }
    else {
      list->active->next->next->prev = list->active;
    }
    list->active->next = list->active->next->next;
    free(item);
  }
  else {
    apperr_runtimeError("TTkList: error in postDelete() function, list is not active or active is last item.");
  }
  LISTP("TTkList: called postDelete()", list);
}

void tknl_preDelete(TTkList list)
{
  if (list->active != NULL && list->active->prev != NULL)
  {
    TTkListItem *item = list->active->prev;
    if (list->active->prev->prev == NULL)
    {
      list->first = list->active;
    }
    else {
      list->active->prev->prev->next = list->active;
    }
    list->active->prev = list->active->prev->prev;
    free(item);
  }
  else {
    apperr_runtimeError("TTkList: error in preDelete() function, list is not active or active is first item.");
  }
  LISTP("TTkList: called preDelete()", list);
}

void TTkList_print(TTkList list)
{
  TTkListItem *item = list->first;
  if (item == NULL) {
    fprintf(stderr, "List is empty.\n");
    return;
  }
  fprintf(stderr, "-first--");
  while (item->next != NULL) {
    if (item == list->active)
    {
      fprintf(stderr, ">>");
    }
    fprintf(stderr, "|%d|", item->token.type);
    if (item == list->active)
    {
      fprintf(stderr, "<<--");
    }
    else {
      fprintf(stderr, "----");
    }
    item = item->next;
  }
  if (list->last == item)
  {
    if (item != list->active) {
      fprintf(stderr, "|%d|--last-\n", item->token.type);
    }
    else
    {
      fprintf(stderr, ">>|%d|<<-last-\n", item->token.type);
    }
  }
  else {
    fprintf(stderr, "!!List is corrupted!!\n");
    apperr_runtimeError("TTkList is corrupted!");
  }
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

TTkList TTkList_create()
{
  TTkList List = mmng_safeMalloc(sizeof(struct tokenList));
  List->first = NULL;
  List->last = NULL;

  List->insertLast = tknl_insertLast;
  List->deleteLast = tknl_deleteLast;
  List->getLast = tknl_getLast;
  List->isEmpty = tknl_isEmpty;
  List->activate = tknl_activate;
  List->prev = tknl_prev;
  List->next = tknl_next;
  List->getActive = tknl_getActive;
  List->postInsert = tknl_postInsert;
  List->postDelete = tknl_postDelete;
  List->preDelete = tknl_preDelete;
  List->destroy = tknl_destroy;

  return List;
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