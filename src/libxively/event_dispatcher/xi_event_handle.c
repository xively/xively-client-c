/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_config.h"
#include "xi_event_handle.h"
#include "xi_macros.h"
#include "xi_allocator.h"
#include "xi_debug.h"
#include <string.h>

#if XI_DEBUG_EXTRA_INFO

xi_event_handle_t xi_make_empty_handle_impl( const uint8_t target_tid,
                                             const char* file_name,
                                             const int line_no )
{
    return ( xi_event_handle_t ){
        XI_EVENT_HANDLE_UNSET, {line_no, file_name}, .handlers.h0 = {0}, target_tid};
}

xi_event_handle_t xi_make_handle_argc0( const uint8_t target_tid,
                                        xi_event_handle_func_argc0_ptr fn_ptr,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){
        XI_EVENT_HANDLE_ARGC0, {line_no, file_name}, .handlers.h0 = {fn_ptr}, target_tid};
}

xi_event_handle_t xi_make_handle_argc1( const uint8_t target_tid,
                                        xi_event_handle_func_argc1_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC1,
                                 {line_no, file_name},
                                 .handlers.h1 = {fn_ptr, a1},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc2( const uint8_t target_tid,
                                        xi_event_handle_func_argc2_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC2,
                                 {line_no, file_name},
                                 .handlers.h2 = {fn_ptr, a1, a2},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc3( const uint8_t target_tid,
                                        xi_event_handle_func_argc3_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC3,
                                 {line_no, file_name},
                                 .handlers.h3 = {fn_ptr, a1, a2, a3},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc4( const uint8_t target_tid,
                                        xi_event_handle_func_argc4_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC4,
                                 {line_no, file_name},
                                 .handlers.h4 = {fn_ptr, a1, a2, a3, a4},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc5( const uint8_t target_tid,
                                        xi_event_handle_func_argc5_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4,
                                        xi_event_handle_arg5_t a5,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC5,
                                 {line_no, file_name},
                                 .handlers.h5 = {fn_ptr, a1, a2, a3, a4, a5},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc6( const uint8_t target_tid,
                                        xi_event_handle_func_argc6_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4,
                                        xi_event_handle_arg5_t a5,
                                        xi_event_handle_arg6_t a6,
                                        const char* file_name,
                                        const int line_no )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC6,
                                 {line_no, file_name},
                                 .handlers.h6 = {fn_ptr, a1, a2, a3, a4, a5, a6},
                                 target_tid};
}

#else /* #if XI_DEBUG_EXTRA_INFO */

xi_event_handle_t xi_make_empty_handle_impl( const uint8_t target_tid )
{
    return ( xi_event_handle_t )xi_make_empty_event_handle( target_tid );
}

xi_event_handle_t
xi_make_handle_argc0( const uint8_t target_tid, xi_event_handle_func_argc0_ptr fn_ptr )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC0, .handlers.h0 = {fn_ptr},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc1( const uint8_t target_tid,
                                        xi_event_handle_func_argc1_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC1, .handlers.h1 = {fn_ptr, a1},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc2( const uint8_t target_tid,
                                        xi_event_handle_func_argc2_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC2, .handlers.h2 = {fn_ptr, a1, a2},
                                 target_tid};
}

xi_event_handle_t xi_make_handle_argc3( const uint8_t target_tid,
                                        xi_event_handle_func_argc3_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC3,
                                 .handlers.h3 = {fn_ptr, a1, a2, a3}, target_tid};
}

xi_event_handle_t xi_make_handle_argc4( const uint8_t target_tid,
                                        xi_event_handle_func_argc4_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC4,
                                 .handlers.h4 = {fn_ptr, a1, a2, a3, a4}, target_tid};
}

xi_event_handle_t xi_make_handle_argc5( const uint8_t target_tid,
                                        xi_event_handle_func_argc5_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4,
                                        xi_event_handle_arg5_t a5 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC5,
                                 .handlers.h5 = {fn_ptr, a1, a2, a3, a4, a5}, target_tid};
}

xi_event_handle_t xi_make_handle_argc6( const uint8_t target_tid,
                                        xi_event_handle_func_argc6_ptr fn_ptr,
                                        xi_event_handle_arg1_t a1,
                                        xi_event_handle_arg2_t a2,
                                        xi_event_handle_arg3_t a3,
                                        xi_event_handle_arg4_t a4,
                                        xi_event_handle_arg5_t a5,
                                        xi_event_handle_arg5_t a6 )
{
    return ( xi_event_handle_t ){XI_EVENT_HANDLE_ARGC6,
                                 .handlers.h6 = {fn_ptr, a1, a2, a3, a4, a5, a6},
                                 target_tid};
}

#endif /* #if XI_DEBUG_EXTRA_INFO */

xi_state_t xi_pointerize_handle( xi_event_handle_t handle, xi_event_handle_t** pointer )
{
    /* PRECONDITIONS */
    if ( NULL == pointer )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL != *pointer )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_AT( xi_event_handle_t, *pointer, state );
    memcpy( *pointer, &handle, sizeof( xi_event_handle_t ) );

err_handling:
    return state;
}

void xi_dispose_handle( xi_event_handle_t* handle )
{
    memset( handle, 0, sizeof( xi_event_handle_t ) );
    handle->handle_type = XI_EVENT_HANDLE_UNSET;
}

uint8_t xi_handle_disposed( xi_event_handle_t* handle )
{
    return handle->handle_type == XI_EVENT_HANDLE_UNSET ? 1 : 0;
}
