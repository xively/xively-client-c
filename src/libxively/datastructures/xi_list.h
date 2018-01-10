/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LIST_H__
#define __XI_LIST_H__

#define XI_LIST_PUSH_BACK( type, list, elem )                                            \
    {                                                                                    \
        type* prev = list;                                                               \
        type* curr = prev;                                                               \
        while ( NULL != curr )                                                           \
        {                                                                                \
            prev = curr;                                                                 \
            curr = curr->__next;                                                         \
        }                                                                                \
        if ( prev == curr )                                                              \
        {                                                                                \
            list = elem;                                                                 \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            prev->__next = elem;                                                         \
        }                                                                                \
    }

#define XI_LIST_PUSH_FRONT( type, list, elem )                                           \
    {                                                                                    \
        if ( NULL != list )                                                              \
        {                                                                                \
            elem->__next = list;                                                         \
            list         = elem;                                                         \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            list = elem;                                                                 \
        }                                                                                \
    }

#define XI_LIST_FIND( type, list, cnd, val, out )                                        \
    {                                                                                    \
        out = list;                                                                      \
        while ( ( NULL != out ) && ( !cnd( out, val ) ) )                                \
        {                                                                                \
            out = out->__next;                                                           \
        }                                                                                \
    }

#define XI_LIST_DROP( type, list, elem )                                                 \
    {                                                                                    \
        type* prev = list;                                                               \
        type* curr = prev;                                                               \
        while ( ( NULL != curr ) && ( curr != elem ) )                                   \
        {                                                                                \
            prev = curr;                                                                 \
            curr = curr->__next;                                                         \
        }                                                                                \
        if ( curr == elem )                                                              \
        {                                                                                \
            if ( prev == curr )                                                          \
            {                                                                            \
                list         = curr->__next;                                             \
                elem->__next = 0;                                                        \
            }                                                                            \
            else                                                                         \
            {                                                                            \
                prev->__next = curr->__next;                                             \
                elem->__next = 0;                                                        \
            }                                                                            \
        }                                                                                \
    }

#define XI_LIST_POP( type, list, out )                                                   \
    {                                                                                    \
        out = list;                                                                      \
        if ( NULL == out->__next )                                                       \
        {                                                                                \
            list = NULL;                                                                 \
        }                                                                                \
        else                                                                             \
        {                                                                                \
            list        = out->__next;                                                   \
            out->__next = NULL;                                                          \
        }                                                                                \
    }

#define XI_LIST_EMPTY( type, list ) ( NULL == list )

#define XI_LIST_FOREACH( type, list, fun )                                               \
    {                                                                                    \
        type* curr = list;                                                               \
        type* tmp  = NULL;                                                               \
        while ( NULL != curr )                                                           \
        {                                                                                \
            tmp = curr->__next;                                                          \
            fun( curr );                                                                 \
            curr = tmp;                                                                  \
        }                                                                                \
    }

#define XI_LIST_FOREACH_WITH_ARG( type, list, fun, arg )                                 \
    {                                                                                    \
        type* curr = list;                                                               \
        type* tmp  = NULL;                                                               \
        while ( NULL != curr )                                                           \
        {                                                                                \
            tmp = curr->__next;                                                          \
            fun( curr, arg );                                                            \
            curr = tmp;                                                                  \
        }                                                                                \
    }

#define XI_LIST_NTH( type, list, n, out )                                                \
    {                                                                                    \
        type* curr = list;                                                               \
        size_t i   = 0;                                                                  \
        while ( NULL != curr )                                                           \
        {                                                                                \
            if ( i++ == n )                                                              \
            {                                                                            \
                out = curr;                                                              \
                break;                                                                   \
            }                                                                            \
            curr = curr->__next;                                                         \
        }                                                                                \
    }

#define XI_LIST_FIND_I( type, list, pred, pred_params, out )                             \
    {                                                                                    \
        type* curr = list;                                                               \
        size_t i   = 0;                                                                  \
        XI_UNUSED( i );                                                                  \
        while ( NULL != curr )                                                           \
        {                                                                                \
            if ( pred( ( pred_params ), curr, i++ ) == 1 )                               \
            {                                                                            \
                out = curr;                                                              \
                break;                                                                   \
            }                                                                            \
            curr = curr->__next;                                                         \
        }                                                                                \
    }

#define XI_LIST_SPLIT_I( type, list, pred, pred_params, out )                            \
    {                                                                                    \
        type* curr     = list;                                                           \
        type* out_curr = out;                                                            \
        type* prev     = NULL;                                                           \
        type* tmp      = NULL;                                                           \
        size_t i       = 0;                                                              \
        XI_UNUSED( i );                                                                  \
        while ( NULL != curr )                                                           \
        {                                                                                \
            if ( pred( ( pred_params ), curr, i++ ) == 1 )                               \
            {                                                                            \
                if ( NULL != prev )                                                      \
                {                                                                        \
                    prev->__next = curr->__next;                                         \
                }                                                                        \
                else                                                                     \
                {                                                                        \
                    list = curr->__next;                                                 \
                }                                                                        \
                                                                                         \
                if ( NULL != out_curr )                                                  \
                {                                                                        \
                    out_curr->__next = curr;                                             \
                    out_curr         = curr;                                             \
                }                                                                        \
                else                                                                     \
                {                                                                        \
                    out_curr = curr;                                                     \
                    out      = out_curr;                                                 \
                }                                                                        \
                                                                                         \
                tmp         = curr;                                                      \
                curr        = curr->__next;                                              \
                tmp->__next = NULL;                                                      \
            }                                                                            \
            else                                                                         \
            {                                                                            \
                prev = curr;                                                             \
                curr = curr->__next;                                                     \
            }                                                                            \
        }                                                                                \
    }

#endif /* __XI_LIST_H__ */
