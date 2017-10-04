/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    MMng.c
 * \brief   Memory manager sources
 * \author  Petr Fusek (xfusek08)
 * \date    26.09.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "MMng.h"

// ************************** internal data structures *****************************

// one item of general pointer table
typedef struct PItem *TMMPItem;
struct PItem {
  void *pointer;
  TMMPItem next;
};

// table of all allocated pointers
typedef struct PList *TMMPList;
struct PList {
  TMMPItem head;
  TMMPItem tail;
};

// constructor of TMMPItem
TMMPItem TMMPItem_create()
{
  TMMPItem newItem = (TMMPItem)malloc(sizeof(struct PItem));
  if (newItem == NULL)
  {
    // allocation error in mmng ... write out and exit;
    perror("TMMPItem_create(): Allocation error in memory manager");
    mmng_freeAll();
    exit(1);
  }

  newItem->pointer = NULL;
  newItem->next = NULL;
  
  return newItem;
}

// constructor of TMMPList
TMMPList TMMPList_create()
{

  TMMPList newList = (TMMPList)malloc(sizeof(struct PList));
  if (newList == NULL)
  {
    // allocation error in mmng ... write out and exit;
    perror("TMMPList_create(): Allocation error in memory manager");
    exit(1);
  }

  newList->head = NULL;
  newList->tail = NULL;

  return newList;
}

// operation ovef pointer list

// adds item with pointer on the end of list
void TMMPList_addPointer(TMMPList list, void *pointer)
{
  TMMPItem newItem = TMMPItem_create();
  
  newItem->pointer = pointer;
  
  if (list->tail != NULL)
    list->tail->next = newItem;
  
  list->tail = newItem;

  if (list->head == NULL)
    list->head = newItem;
}

/**
 * removes pointer from list but frees only list item not pointer itself 
 * returns true if some item was deleted, otherwise false
 */
bool TMMPList_deletePointer(TMMPList list, void *pointer)
{
  TMMPItem actItem = list->head;
  TMMPItem prevItem = NULL;
  while(actItem != NULL)
  {
    if (actItem->pointer == pointer)
    {
      if (prevItem != NULL)
        prevItem->next = actItem->next;
      
      if (list->head == actItem) 
        list->head = actItem->next;

      if (list->tail == actItem) 
        list->tail = prevItem;

      free(actItem);
      return true;
    }
    prevItem = actItem; 
    actItem = actItem->next;
  }
  return false;
}

// definition of internal static instance of list pf pointers
TMMPList GLBPointerList;

// ******************************* support functions  ******************************

// error if GLBPointerList is not initialized
void assertIfNotInit(char *funcname)
{
  if (GLBPointerList == NULL)
  {
    if (funcname == NULL)
      funcname = "undefined function";
    fprintf(stderr, "%s: Memory manager is not initialized.", funcname);
    exit(1);
  }
}

// ************************** Interface function definition ************************

void mmng_init()
{
  if (GLBPointerList != NULL)
  {
    fprintf(stderr, "mmng_init(): Memory manager is already initialized.");
    exit(1);
  } 
  GLBPointerList = TMMPList_create();
}

// Safe allocation
void *mmng_safeMalloc(size_t size)
{
  assertIfNotInit("mmng_safeMalloc()");

  void *pointer = malloc(size);
  if (pointer == NULL)
  {
    perror("mmng_safeMalloc(): Allocation error");
    mmng_freeAll();
    exit(1);
  }
  TMMPList_addPointer(GLBPointerList, pointer);
  return pointer;
}

// Free all allocated memory
void mmng_freeAll()
{
  assertIfNotInit("mmng_freeAll()");
  
  while (GLBPointerList->head != NULL)
    mmng_safeFree(GLBPointerList->head);
  free(GLBPointerList);
  GLBPointerList = NULL;
}

// Safe free
void mmng_safeFree(void *pointer)
{
  assertIfNotInit("mmng_safeFree()");
  
  if (TMMPList_deletePointer(GLBPointerList, pointer))
    free(pointer);
  else
  {
    fprintf(stderr, "mmng_safeFree(): pointer canntot be freed. Pointer is not part of internal pointer evidence");
    exit(1);
  }
}