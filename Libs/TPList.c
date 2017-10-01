/**
 * @brief   General list of pointers sources
 * @file    TPList.c
 * @author  Petr Fusek (xfusek08)
 * @date    26.09.2017
 *
 * **Last modified:**  Petr Fusek
 */

#include <stdlib.h>
#include "MMng.h"
#include "TPList.h"

// TPListItem constructor
TPListItem TPListItem_create(void *pointer)
{
  TPListItem newItem = (TPListItem)mmng_safeMalloc(sizeof(struct PListItem));
  newItem->pointer = pointer;
  newItem->next = NULL;
  return newItem;
}

// TPListItem destructor
void TPListItem_destroy(TPListItem listItem)
{
  mmng_safeFree(listItem->pointer);
  mmng_safeFree(listItem);
}

// TPList constructor
TPList TPList_create()
{
  TPList newList = (TPList)mmng_safeMalloc(sizeof(struct PList));
  newList->count = 0;
  newList->head = NULL;
  newList->active = NULL;
  return newList;
}

// TPList destructor
void TPList_destroy(TPList list)
{
  
}

// *********************** Methods definitions *************************

// inserts pointer (reference on object) on to begining of the list
void TPList_insertFirst(TPList list, void *pointer)
{
  TPListItem newItem = TPListItem_create(pointer);
  newItem->next = list->head;
  list->head = newItem;
  list->count++;
}
 
// get fisrt pointer in the list 
void *TPList_getFirst(TPList list)
{
  return (list->head != NULL) ? list->head->pointer : NULL;
}
 
// Deletes and frees first pointer in list 
void TPList_deleteFirst(TPList list)
{
  if (list->head != NULL)
  {
    TPListItem itemToDel = list->head;
    list->head = itemToDel->next;
    TPListItem_destroy(itemToDel);
    list->count--;
  }
}
 
// sets fisrt pointer as active 
void TPList_first(TPList list)
{ 
  list->active = list->head;
}

// pointer activity is passed to next neighbor of active pointer   
void TPList_next(TPList list)
{
  if (list->active != NULL)
    list->active = list->active->next;
}
 