/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    MMng.c
 * \brief   Memory manager sources
 * \author  Petr Fusek (xfusek08)
 * \date    24.10.2017 - Petr Fusek
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mmng.h"
#include "apperr.h"

// =============================================================================
// ================= Iternal data structures definition ========================
// =============================================================================

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


// global internal instance of list of pointers
TMMPList GLBPointerList;

// =============================================================================
// ======================= TMMPItem implementation ==============================
// =============================================================================

// constructor of TMMPItem
TMMPItem TMMPItem_create()
{
  TMMPItem newItem = (TMMPItem)malloc(sizeof(struct PItem));
  if (newItem == NULL)
    apperr_runtimeError("Allocation error in memory manager");

  newItem->pointer = NULL;
  newItem->next = NULL;

  return newItem;
}

// =============================================================================
// ======================= TMMPList implementation ==============================
// =============================================================================

// constructor of TMMPList
TMMPList TMMPList_create()
{
  TMMPList newList = (TMMPList)malloc(sizeof(struct PList));
  if (newList == NULL)
    apperr_runtimeError("Allocation error in memory manager");

  newList->head = NULL;
  newList->tail = NULL;

  return newList;
}

// adds item with pointer on the end of list
void TMMPList_addPointer(TMMPList list, void *pointer)
{
  if (pointer == NULL)
    return;

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
  if (pointer == NULL)
    return false;

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

// =============================================================================
// ====================== support function ====================================
// =============================================================================

// error if GLBPointerList is not initialized
void assertIfNotInit()
{
  if (GLBPointerList == NULL)
  {
    fprintf( stderr, "\033[31;1mError:\033[0m Memory manager is not initialized.\n");
    exit(internalErr);
  }
}

// =============================================================================
// ====================== Interface implementation =============================
// =============================================================================

// memory manager initialization
void mmng_init()
{
  if (GLBPointerList != NULL)
    apperr_runtimeError("mmng_init(): Memory manager is already initialized.");
  GLBPointerList = TMMPList_create();
}

// Safe allocation
void *mmng_safeMalloc(size_t size)
{
  assertIfNotInit();

  void *pointer = malloc(size);
  if (pointer == NULL)
    apperr_runtimeError("mmng_safeMalloc(): Allocation error.");

  TMMPList_addPointer(GLBPointerList, pointer);
  return pointer;
}

// Reallocate allocated memory of given pointer to new size.
void *mmng_safeRealloc(void *pointer, size_t size)
{
  assertIfNotInit();

  void *newPointer = realloc(pointer, size);
  if (newPointer == NULL)
    apperr_runtimeError("mmng_safeRealloc(): Reallocation error.");

  // delete old pointer
  TMMPList_deletePointer(GLBPointerList, pointer);
  // store new pointer
  TMMPList_addPointer(GLBPointerList, newPointer);
  return newPointer;
}

// Free all allocated memory
void mmng_freeAll()
{
  assertIfNotInit();

  while (GLBPointerList->head != NULL)
    mmng_safeFree(GLBPointerList->head->pointer);
  free(GLBPointerList);
  GLBPointerList = NULL;
}

// Safe free
void mmng_safeFree(void *pointer)
{
  assertIfNotInit();

  if (TMMPList_deletePointer(GLBPointerList, pointer))
    free(pointer);
  else
    apperr_runtimeError("mmng_safeFree(): Pointer can't be freed. Pointer is not part of internal pointer evidence.");
}
