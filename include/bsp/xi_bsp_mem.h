/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_MEM_H__
#define __XI_BSP_MEM_H__

/**
 * @file xi_bsp_mem.h
 * @brief Xively Client's Board Support Platform (BSP) for Memory Management
 *
 * This file defines the Memory Management API used by the Xively C Client.
 *
 * The Xively C Client manages memory through these functions. By implementing these
 * functions the application can customize memory management of the Xively C Client. For
 * instance a custom implementation can use static memory on devices where
 * heap isn't desired. Another use case might be memory tracking by injecting metrics
 * into the implementation.
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @function
 * @brief Allocates a block of byte_count bytes of memory, returns a pointer to
 *        the beginning of the block.
 *
 * There are no requirements against the content of the memory block returned.
 *
 * @param [in] byte_count the number of bytes to allocate
 * @return the beginning of the allocated block
 */
void* xi_bsp_mem_alloc( size_t byte_count );

/**
 * @function
 * @brief Changes the size of the memory block pointed to by ptr.
 *
 * The content of the memory block is preserved up to the lesser of the new and old
 * sizes, even if the block is moved to a new location. If the new size is larger,
 * the value of the newly allocated portion is indeterminate.
 *
 * @param [in] ptr pointer to a memory block, this memory block will be reallocated
 * @return a pointer to the reallocated memory block
 */
void* xi_bsp_mem_realloc( void* ptr, size_t byte_count );

/**
 * @function
 * @brief A block of memory previously allocated by a call to xi_bsp_mem_malloc,
 * xi_bsp_mem_realloc is deallocated, making it available again for further
 * allocations.
 *
 * @param [in] ptr pointer to a memory block
 */
void xi_bsp_mem_free( void* ptr );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_MEM_H__ */
