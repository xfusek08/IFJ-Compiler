/**
 * @brief   General list of pointers
 * @file    TPList.h
 * @author  Petr Fusek (xfusek08)
 * @date    26.09.2017
 *
 * **Last modified:**  Petr Fusek
 */

#ifndef _TPList
#define _TPList

typedef struct PListItem *TPListItem;
struct PListItem {
  void *pointer;
  TPListItem next;
};

/**
 * @brief TPListItem constructor - Creates istance of TPListItem object.
 * 
 * Function allocates memory and inits object do its default value
 * @param   void *      pointer to be stored
 * @retval  TPListItem  instance of TPListItem
 */
TPListItem TPListItem_create(void *pointer);

/**
 * @brief TPListItem destructor - destroy istance of TPListItem objet.
 * 
 * Frees stored pointer and then item itself
 * @param   TPListItem  pointer to TPListItem object instance
 */
void TPListItem_destroy(TPListItem listItem);
 
typedef struct PList *TPList;
struct PList {
  int count;
  TPListItem head;
  TPListItem active;
};

/**
 * @brief TPList constructor - Creates istance of object TPList.
 * 
 * Function allocates memory and inits object do its default value
 * @retval TPList  instance of TPList
 */
 TPList TPList_create();

 /**
 * @brief TPList destructor - destroy istance of object TPList.
 * 
 * Frees all stored pointers and itself
 */
void TPList_destroy(TPList list);
 
// ******************** Methods ***********************

void TPList_clear(TPList list);

void TPList_remove(TPList list, void *pointer);

void TPList_removeByIndex(TPList list, unsigned int index);

/**
 * @brief Iterate through whole list and calls function on each pointer
 * @param TPList   list reference 
 * @param function* pointer to function with prototype: 
 *    \code{.c} bool doFunc(void *); \endcode 
 *    Function is called for each pointer in list and actual pointer is passed in parameter.\n
 *    Iteration is stopped when function returns false.
 */
void TPList_foreach(TPList list, bool (*doFunc)(void *actPointer));

/**
 * @brief inserts pointer (reference on object) on to begining of the list
 * @param TPList   list reference 
 * @param void *  Pointer to be insert
 */
void TPList_insertFirst(TPList list, void *pointer);

/**
 * @brief get fisrt pointer in the list 
 * @param   TPList   list reference
 * @retval  void *  pointer on void. Needs to be retyped
 * @error   throws when list is empty  
 */
void *TPList_getFirst(TPList list);

/**
 * @brief Deletes and frees first pointer in list 
 * @param   TPList   list reference
 */
void TPList_deleteFirst(TPList list);

/**
 * @brief sets fisrt pointer as active 
 * @param   TPList   list reference
 */
void TPList_first(TPList list);

/**
 * @brief pointer activity is passed to next neighbor of active pointer   
 */
void TPList_next(TPList list);
 
#endif // _TPList
