/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_PLATFORM_ALLOCATOR_H__
#define __XI_PLATFORM_ALLOCATOR_H__

#include <stdlib.h>

#include "xi_config.h"

#ifdef XI_MEMORY_LIMITER_ENABLED
#include "xi_memory_limiter.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief allocates the number of bytes given by the b parameter
 * @param b number of bytes to allocate
 * @return ptr to allocated memory or NULL if there was not enough memory
 * @note newly allocated memory is unspecified ( use memset in order to set it to some
 * value )
 */
extern void* __xi_alloc( size_t b );

/**
 * @brief allocates the number of elements given by the num parameter of size described by
 * size argument
 *
 * It reduces to alloc( num * size );
 *
 * @param num
 * @param size
 */
extern void* __xi_calloc( size_t num, size_t size );

/**
 * @brief changes the size of already allocated memory
 *
 * if there is no more memory left it will return NULL
 * it may return different ptr than ptr
 * min( b, old_size ) is the space that suppose to be remain and the newly allocated
 * memory is unspecified
 *
 * @param ptr
 * @param b
 */
extern void* __xi_realloc( void* ptr, size_t b );

/**
 * @brief frees previously allocated memory
 * @param ptr to allocated memory buffer may be equall to NULL
 */
extern void __xi_free( void* ptr );

/**
 * @brief macro to make thin facade for debug and memory limiting
 */
#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_alloc( b ) xi_memory_limiter_alloc_application( b, __FILE__, __LINE__ )
#else
#define xi_alloc __xi_alloc
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_calloc( num, byte_count )                                                     \
    xi_memory_limiter_calloc_application( num, byte_count, __FILE__, __LINE__ )
#else
#define xi_calloc __xi_calloc
#endif

/**
 * @brief macro to make thin facade for debug and memory limiting
 */
#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_realloc( p, b )                                                               \
    xi_memory_limiter_realloc_application( p, b, __FILE__, __LINE__ )
#else
#define xi_realloc __xi_realloc
#endif

/**
 * @brief macro to make thin facade for debug and memory limiting
 */
#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_free( p ) xi_memory_limiter_free( p )
#else
#define xi_free __xi_free
#endif

/* for system allocations */
#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_alloc_system( b ) xi_memory_limiter_alloc_system( b, __FILE__, __LINE__ )
#else
#define xi_alloc_system( b ) __xi_alloc( b )
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_calloc_system( num, byte_count )                                              \
    xi_memory_limiter_calloc_system( num, byte_count, __FILE__, __LINE__ )
#else
#define xi_calloc_system( num, byte_count ) __xi_calloc( num, byte_count )
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_realloc_system( b ) xi_memory_limiter_realloc_system( b, __FILE__, __LINE__ )
#else
#define xi_realloc_system( b ) __xi_realloc( b )
#endif

/* for exposing ptr's */
#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_alloc_ptr &xi_memory_limiter_alloc_application_export
#else
#define xi_alloc_ptr &__xi_alloc
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_calloc_ptr &xi_memory_limiter_calloc_application_export
#else
#define xi_calloc_ptr &__xi_calloc
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_realloc_ptr &xi_memory_limiter_realloc_application_export
#else
#define xi_realloc_ptr &__xi_realloc
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#define xi_free_ptr &xi_memory_limiter_free
#else
#define xi_free_ptr &__xi_free
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_PLATFORM_ALLOCATOR_H__ */
