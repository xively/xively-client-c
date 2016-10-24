/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <wm_os.h>

#ifdef __cplusplus
extern "C" {
#endif

void* __xi_alloc( size_t b )
{
    return ( void* )os_mem_alloc( b );
}

void* __xi_realloc( void* p, size_t b )
{
    return ( void* )os_mem_realloc( p, b );
}

void __xi_free( void* p )
{
    os_mem_free( p );
}

#ifdef __cplusplus
}
#endif
