/**
 * @brief   Memory manager
 * @file    MMng.h
 * @author  Petr Fusek (xfusek08)
 * @date    26.09.2017
 *
 * This package provides functions for basic safe memory operations.
 * There are functions for memory allocation and deallocation including deallocation of all memory at once.
 * Operations are treated and all errors are handled by freeing memory and terminating program with corresponding error description.
 * Therefore there is no need of another external check of allocation result. This is make better surence of right work with memmory. 
 *
 * **Last modified:**  Petr Fusek
 */

#ifndef _MMng
#define _MMng

/**
 * @brief Initialization 
 *
 * Function prepares internal data structures and allows using another functions of memory manager.
 * This function has to be called before first call of **safe allocation fucntion** (@ref mmng_safeMalloc) owterwise error is occured.
 */
void mmng_init();

/**
 * @brief Safe allocation
 * 
 * Function provides allocation of memory. In case of error frees all so far allocated memory and terminate program with coresponding error.
 * Pointer of new allocated block is stored in internal data structure for safe deallocation.
 *
 * @param   size_t    size of memory to be allocated, corespond to result of sizeof() function
 * @retval  void *    pointer to allocated memory
 */
 void *mmng_safeMalloc(size_t size);

 /**
 * @brief Free all allocated memory
 * 
 * Function safely frees all allocated memory including internal data structures.
 * After calling this function program should end or memory manager has to be initialized again. 
 */
 void mmng_freeAll();

 /**
 * @brief Safe free
 * 
 * Function safely frees pointer and unregister it from internal data structures
 */
 void mmng_safeFree(void *pointer);

#endif // _MMng
