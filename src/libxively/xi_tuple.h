/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TUPLE_H__
#define __XI_TUPLE_H__

#include "xi_allocator.h"
#include "xi_macros.h"

#define XI_DEF_TUPLE_TYPE( type_name, ... )                                              \
    XI_DEF_TUPLE_TYPE_IMPL( __VA_ARGS__, XI_DEF_TUPLE_TYPE_4( type_name, __VA_ARGS__ ),  \
                            XI_DEF_TUPLE_TYPE_3( type_name, __VA_ARGS__ ),               \
                            XI_DEF_TUPLE_TYPE_2( type_name, __VA_ARGS__ ),               \
                            XI_DEF_TUPLE_TYPE_1( type_name, __VA_ARGS__ ), 0 )

#define XI_DEF_TUPLE_TYPE_IMPL( _1, _2, _3, _4, N, ... ) N

#ifdef XI_TUPLES_C
#define XI_DEF_TUPLE_TYPE_1( type_name, a1_t )                                           \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
    } type_name;                                                                         \
    type_name xi_make_tuple_##type_name( a1_t a1 )                                       \
    {                                                                                    \
        type_name ret;                                                                   \
        ret.a1 = a1;                                                                     \
        return ret;                                                                      \
    }                                                                                    \
    type_name* xi_alloc_make_tuple_##type_name( a1_t a1 )                                \
    {                                                                                    \
        type_name* ret   = ( type_name* )xi_alloc( sizeof( type_name ) );                \
        xi_state_t state = XI_STATE_OK;                                                  \
        XI_CHECK_MEMORY( ret, state );                                                   \
        memset( ret, 0, sizeof( type_name ) );                                           \
        ret->a1 = a1;                                                                    \
        return ret;                                                                      \
    err_handling:                                                                        \
        return 0;                                                                        \
    }
#else /* XI_TUPLES_C */
#define XI_DEF_TUPLE_TYPE_1( type_name, a1_t )                                           \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
    } type_name;                                                                         \
    extern type_name xi_make_tuple_##type_name( a1_t a1 );                               \
    extern type_name* xi_alloc_make_tuple_##type_name( a1_t a1 );
#endif /* XI_TUPLES_C */

#ifdef XI_TUPLES_C
#define XI_DEF_TUPLE_TYPE_2( type_name, a1_t, a2_t )                                     \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
    } type_name;                                                                         \
    type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2 )                              \
    {                                                                                    \
        type_name ret;                                                                   \
        ret.a1 = a1;                                                                     \
        ret.a2 = a2;                                                                     \
        return ret;                                                                      \
    }                                                                                    \
    type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2 )                       \
    {                                                                                    \
        type_name* ret   = ( type_name* )xi_alloc( sizeof( type_name ) );                \
        xi_state_t state = XI_STATE_OK;                                                  \
        XI_CHECK_MEMORY( ret, state );                                                   \
        memset( ret, 0, sizeof( type_name ) );                                           \
        ret->a1 = a1;                                                                    \
        ret->a2 = a2;                                                                    \
        return ret;                                                                      \
    err_handling:                                                                        \
        return 0;                                                                        \
    }
#else /* XI_TUPLES_C */
#define XI_DEF_TUPLE_TYPE_2( type_name, a1_t, a2_t )                                     \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
    } type_name;                                                                         \
    extern type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2 );                      \
    extern type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2 );
#endif /* XI_TUPLES_C */

#ifdef XI_TUPLES_C
#define XI_DEF_TUPLE_TYPE_3( type_name, a1_t, a2_t, a3_t )                               \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
        a3_t a3;                                                                         \
    } type_name;                                                                         \
    type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3 )                     \
    {                                                                                    \
        type_name ret;                                                                   \
        ret.a1 = a1;                                                                     \
        ret.a2 = a2;                                                                     \
        ret.a3 = a3;                                                                     \
        return ret;                                                                      \
    }                                                                                    \
    type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3 )              \
    {                                                                                    \
        type_name* ret   = ( type_name* )xi_alloc( sizeof( type_name ) );                \
        xi_state_t state = XI_STATE_OK;                                                  \
        XI_CHECK_MEMORY( ret, state );                                                   \
        memset( ret, 0, sizeof( type_name ) );                                           \
        ret->a1 = a1;                                                                    \
        ret->a2 = a2;                                                                    \
        ret->a3 = a3;                                                                    \
        return ret;                                                                      \
    err_handling:                                                                        \
        return 0;                                                                        \
    }
#else /* XI_TUPLES_C */
#define XI_DEF_TUPLE_TYPE_3( type_name, a1_t, a2_t, a3_t )                               \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
        a3_t a3;                                                                         \
    } type_name;                                                                         \
    extern type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3 );             \
    extern type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3 );
#endif /* XI_TUPLES_C */

#ifdef XI_TUPLES_C
#define XI_DEF_TUPLE_TYPE_4( type_name, a1_t, a2_t, a3_t, a4_t )                         \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
        a3_t a3;                                                                         \
        a4_t a4;                                                                         \
    } type_name;                                                                         \
    type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3, a4_t, a4 )           \
    {                                                                                    \
        type_name ret;                                                                   \
        ret.a1 = a1;                                                                     \
        ret.a2 = a2;                                                                     \
        ret.a3 = a3;                                                                     \
        ret.a4 = a4;                                                                     \
        return ret;                                                                      \
    }                                                                                    \
    type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3, a4_t, a4 )    \
    {                                                                                    \
        type_name* ret   = ( type_name* )xi_alloc( sizeof( type_name ) );                \
        xi_state_t state = XI_STATE_OK;                                                  \
        XI_CHECK_MEMORY( ret, state );                                                   \
        memset( ret, 0, sizeof( type_name ) );                                           \
        ret->a1 = a1;                                                                    \
        ret->a2 = a2;                                                                    \
        ret->a3 = a3;                                                                    \
        ret->a4 = a4;                                                                    \
        return ret;                                                                      \
    err_handling:                                                                        \
        return 0;                                                                        \
    }
#else /* XI_TUPLES_C */
#define XI_DEF_TUPLE_TYPE_4( type_name, a1_t, a2_t, a3_t, a4_t )                         \
    typedef struct                                                                       \
    {                                                                                    \
        a1_t a1;                                                                         \
        a2_t a2;                                                                         \
        a3_t a3;                                                                         \
        a4_t a4;                                                                         \
    } type_name;                                                                         \
    extern type_name xi_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3, a4_t, a4 );   \
    extern type_name* xi_alloc_make_tuple_##type_name( a1_t a1, a2_t a2, a3_t a3, a4_t,  \
                                                       a4 );
#endif /* XI_TUPLES_C */

#define xi_make_tuple_a1( type_name, a1 ) xi_make_tuple_##type_name( a1 );

#define xi_make_tuple_a2( type_name, a1, a2 ) xi_make_tuple_##type_name( a1, a2 );

#define xi_make_tuple_a3( type_name, a1, a2, a3 ) xi_make_tuple_##type_name( a1, a2, a3 );

#define xi_make_tuple_a4( type_name, a1, a2, a3, a4 )                                    \
    xi_make_tuple_##type_name( a1, a2, a3, a4 );

#define xi_make_tuple( type_name, ... )                                                  \
    xi_make_tuple_impl( __VA_ARGS__, xi_make_tuple_a4( type_name, __VA_ARGS__ ),         \
                        xi_make_tuple_a3( type_name, __VA_ARGS__ ),                      \
                        xi_make_tuple_a2( type_name, __VA_ARGS__ ),                      \
                        xi_make_tuple_a1( type_name, __VA_ARGS__ ), 0 )

#define xi_make_tuple_impl( _1, _2, _3, _4, N, ... ) N

#define xi_alloc_make_tuple_a1( type_name, a1 ) xi_alloc_make_tuple_##type_name( a1 );

#define xi_alloc_make_tuple_a2( type_name, a1, a2 )                                      \
    xi_alloc_make_tuple_##type_name( a1, a2 );

#define xi_alloc_make_tuple_a3( type_name, a1, a2, a3 )                                  \
    xi_alloc_make_tuple_##type_name( a1, a2, a3 );

#define xi_alloc_make_tuple_a4( type_name, a1, a2, a3, a4 )                              \
    xi_alloc_make_tuple_##type_name( a1, a2, a3, a4 );

#define xi_alloc_make_tuple( type_name, ... )                                            \
    xi_alloc_make_tuple_impl( __VA_ARGS__,                                               \
                              xi_alloc_make_tuple_a4( type_name, __VA_ARGS__ ),          \
                              xi_alloc_make_tuple_a3( type_name, __VA_ARGS__ ),          \
                              xi_alloc_make_tuple_a2( type_name, __VA_ARGS__ ),          \
                              xi_alloc_make_tuple_a1( type_name, __VA_ARGS__ ), 0 )

#define xi_alloc_make_tuple_impl( _1, _2, _3, _4, N, ... ) N

#define XI_SAFE_FREE_TUPLE( tuple ) XI_SAFE_FREE( tuple )

#endif /* __XI_TUPLE_H__ */
