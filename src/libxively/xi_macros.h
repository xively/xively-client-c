/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MACROS_H__
#define __XI_MACROS_H__

#include <string.h>
#include "xi_allocator.h"
#include "xi_err.h"

#define XI_STR_EXPAND( tok ) #tok
#define XI_STR( tok ) XI_STR_EXPAND( tok )
#define XI_MIN( a, b ) ( ( a ) > ( b ) ? ( b ) : ( a ) )
#define XI_MAX( a, b ) ( ( a ) > ( b ) ? ( a ) : ( b ) )
#define XI_UNUSED( x ) ( void )( x )

#define XI_GUARD_EOS( s, size )                                                          \
    {                                                                                    \
        ( s )[( size )-1] = '\0';                                                        \
    }

/* a = input value, mi = minimum, ma = maximum */
#define XI_CLAMP( a, mi, ma ) XI_MIN( XI_MAX( ( a ), ( mi ) ), ( ma ) )

#define XI_CHECK_CND( cnd, e, s )                                                        \
    XI_UNUSED( ( s ) );                                                                  \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        ( s ) = ( e );                                                                   \
        goto err_handling;                                                               \
    }

#define XI_CHECK_STATE( s )                                                              \
    if ( ( s ) != XI_STATE_OK )                                                          \
    {                                                                                    \
        goto err_handling;                                                               \
    }

#define XI_CHECK_ZERO( a, e, s ) XI_CHECK_CND( ( a ) == 0, ( e ), ( s ) )

#define XI_CHECK_CND_DBGMESSAGE( cnd, e, s, dbgmessage )                                 \
    XI_UNUSED( ( s ) );                                                                  \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        ( s ) = ( e );                                                                   \
        xi_debug_logger( dbgmessage );                                                   \
        goto err_handling;                                                               \
    }

#define XI_CHECK_NEG( a )                                                                \
    if ( ( a ) < 0 ) )

#define XI_CHECK_PTR( a, b ) if ( ( a ) == ( b ) )

#define XI_SAFE_FREE( a )                                                                \
    if ( ( a ) )                                                                         \
    {                                                                                    \
        xi_free( a );                                                                    \
        ( a ) = NULL;                                                                    \
    }

#define XI_CHECK_MEMORY( a, s ) XI_CHECK_CND( ( a ) == 0, XI_OUT_OF_MEMORY, s )

#define XI_CHECK_SIZE( a, b, e, s ) XI_CHECK_CND( ( ( a ) >= ( b ) || ( a ) < 0 ), e, s )

#define XI_CLEAR_STATIC_BUFFER( a ) memset( ( a ), 0, sizeof( a ) )

#define XI_CHECK_S( s, size, o, e, st )                                                  \
    {                                                                                    \
        XI_CHECK_SIZE( s, size - o, e, st )                                              \
        else                                                                             \
        {                                                                                \
            ( o ) += ( s );                                                              \
        }                                                                                \
    }

/* generic allocation macro group */
#define XI_ALLOC_BUFFER_AT( type_t, out, size, state )                                   \
    out = ( type_t* )xi_alloc( size );                                                   \
    XI_CHECK_MEMORY( out, state );                                                       \
    memset( out, 0, size );

#define XI_ALLOC_BUFFER( type_t, out, size, state )                                      \
    type_t* out = NULL;                                                                  \
    XI_ALLOC_BUFFER_AT( type_t, out, size, state );

#define XI_ALLOC_AT( type_t, out, state )                                                \
    XI_ALLOC_BUFFER_AT( type_t, out, sizeof( type_t ), state );

#define XI_ALLOC( type_t, out, state )                                                   \
    type_t* out = NULL;                                                                  \
    XI_ALLOC_AT( type_t, out, state )

/* system allocation macro group */
#define XI_ALLOC_SYSTEM_BUFFER_AT( type_t, out, size, state )                            \
    out = ( type_t* )xi_alloc_system( size );                                            \
    XI_CHECK_MEMORY( out, state );                                                       \
    memset( out, 0, size );

#define XI_ALLOC_SYSTEM_BUFFER( type_t, out, size, state )                               \
    type_t* out = NULL;                                                                  \
    XI_ALLOC_SYSTEM_BUFFER_AT( type_t, out, size, state );

#define XI_ALLOC_SYSTEM_AT( type_t, out, state )                                         \
    XI_ALLOC_SYSTEM_BUFFER_AT( type_t, out, sizeof( type_t ), state );

#define XI_ALLOC_SYSTEM( type_t, out, state )                                            \
    type_t* out = NULL;                                                                  \
    XI_ALLOC_SYSTEM_AT( type_t, out, state )

#define XI_ARRAYSIZE( a ) ( sizeof( a ) / sizeof( ( a )[0] ) )

#define XI_TIME_MILLISLEEP( milliseconds, timespec_localvariablename )                   \
    struct timespec timespec_localvariablename;                                          \
    timespec_localvariablename.tv_sec  = 0;                                              \
    timespec_localvariablename.tv_nsec = milliseconds * 1000 * 1000;                     \
    nanosleep( &timespec_localvariablename, NULL );

#endif /*__XI_MACROS_H__ */
