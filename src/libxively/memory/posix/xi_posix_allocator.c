/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void* __xi_alloc( size_t b )
{
    return ( void* )malloc( b );
}

void* __xi_realloc( void* ptr, size_t b )
{
    return ( void* )realloc( ptr, b );
}

void __xi_free( void* p )
{
    free( p );
}

#ifdef __cplusplus
}
#endif
