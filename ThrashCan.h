#ifndef THRASHCAN_H
#define THRASHCAN_H

/*! \file ThrashCan.h
 * \brief file that defines the functions that wrap memory allocation.
 *
 * The purpose of this header is to wrap all the memory allocation done at
 * the query engine, since there are several files that do the work at the same
 * time. With this memory allocation is evaded in a simple way, since all memory
 * is freed calling to tc_emptyThrashcan().
 */

/**
 * Creates an instance to a thrashcan. Each instance handle a different set of pointers
 * to alloced memory.
 */
void tc_instanceThrashCan();

/**
 * wrapper to malloc function.
 * @param size size in bytes to ask for to the heap.
 * @return void pointer with the address to the memory allocated.
 */
void* tc_malloc(int size);

/**
 * wrapper to realloc function.
 * @param ptr pointer to already alloced memory.
 * @param size size in bytes to ask for to the heap.
 * @return void pointer with the address to the newly memory allocated.
 */
void* tc_realloc(void *ptr, int size);

/**
 * wrapper to strdup function.
 * @param str string to be duplicated.
 * @return duplicated string.
 */
char* tc_strdup(const char *str);

/**
 * Frees all memory allocated before the last time this function was called.
 */
void tc_emptyThrashCan();

#endif
