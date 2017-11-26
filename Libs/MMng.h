/******************************************************************************/
/**
 * \project IFJ-Compiler
 * \file    MMng.h
 * \brief   Memory manager
 *
 * This package provides functions for basic safe memory operations.
 * There are functions for memory allocation and deallocation including deallocation of all memory at once.
 * Operations are treated and all errors are handled by freeing memory and terminating program with corresponding error description.
 * Therefore there is no need for another external checks of allocation results. This makes better surence of proper work with memmory.
 *
 * \author  Petr Fusek (xfusek08)
 * \date    19.10.2017 - Petr Fusek
 * \todo all errors redo for Application Error library
 * \todo add some debugging logging macra ...
 */
/******************************************************************************/

#ifndef _MMng
#define _MMng

#include <stdlib.h>

/**
 * Initialization
 *
 * Function prepares internal data structures and allows using another functions of memory manager.
 * This function has to be called before first call of **safe allocation fucntion** (\ref mmng_safeMalloc) owterwise error is occured.
 */
void mmng_init();

/**
 * Safe allocation
 *
 * Function provides allocation of memory. In case of error frees all so far allocated memory and terminate program with coresponding error.
 * Pointer of new allocated block is stored in internal data structure for safe deallocation.
 *
 * \param   size_t    size of memory to be allocated, corespond to result of sizeof() function
 * \retval  void *    pointer to allocated memory
 */
void *mmng_safeMalloc(size_t size);

/**
 * Reallocate allocated memory of given pointer to new size.
 *
 * \param   void *    pointer to allocated memory, if NULL new memory is allocated
 * \param   size_t    size of memory to be allocated, corespond to result of sizeof() function
 * \retval  void *    new pointer to reallocated memmory
 */
void *mmng_safeRealloc(void *pointer, size_t size);

/**
 * Free all allocated memory
 *
 * Function safely frees all allocated memory including internal data structures.
 * After calling this function program should end or memory manager has to be initialized again.
 */
void mmng_freeAll();

/**
 * Safe free
 *
 * Function safely frees pointer and unregister it from internal data structures
 */
void mmng_safeFree(void *pointer);

#endif // _MMng
