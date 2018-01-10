/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_EVENT_HANDLE_H__
#define __XI_EVENT_HANDLE_H__

#include "xi_config.h"
#include "xi_event_handle_typedef.h"
#include "xi_thread_ids.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Idea for that macro trick has been taken from the other macro
 * that counts the number of arguments of variadic macro. The rule
 * and generic idea is the same. The xi_make_handle_impl takes always
 * Nth argument and uses it for macro substitution. The N has to be const
 * and it's 6 in our case since macro 'xi_make_handle' can be called
 * at most with 6 arguments: threadid, fnptr + 4args.
 */

#define xi_make_handle( ... )                                                            \
    xi_make_threaded_handle( XI_THREADID_MAINTHREAD, __VA_ARGS__ )
#define xi_make_handle_impl( _0, _1, _2, _3, _4, _5, _6, _7, N, ... ) N

#if XI_DEBUG_EXTRA_INFO

#define xi_make_threaded_handle( ... )                                                   \
    xi_make_handle_impl(                                                                 \
        __VA_ARGS__, xi_make_handle_argc6( __VA_ARGS__, __FILE__, __LINE__ ),            \
        xi_make_handle_argc5( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        xi_make_handle_argc4( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        xi_make_handle_argc3( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        xi_make_handle_argc2( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        xi_make_handle_argc1( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        xi_make_handle_argc0( __VA_ARGS__, __FILE__, __LINE__ ),                         \
        0 ) /* the last 0 to not alow the impl macro to not to have last                 \
            parameter in case of 1 arg passed */

#define xi_make_empty_handle( ... )                                                      \
    xi_make_empty_handle_impl( XI_THREADID_MAINTHREAD, __FILE__, __LINE__ )

extern xi_event_handle_t xi_make_empty_handle_impl( const uint8_t target_tid,
                                                    const char* file_name,
                                                    const int line_no );

extern xi_event_handle_t xi_make_handle_argc0( const uint8_t target_tid,
                                               xi_event_handle_func_argc0_ptr fn_ptr,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc1( const uint8_t target_tid,
                                               xi_event_handle_func_argc1_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc2( const uint8_t target_tid,
                                               xi_event_handle_func_argc2_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc3( const uint8_t target_tid,
                                               xi_event_handle_func_argc3_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc4( const uint8_t target_tid,
                                               xi_event_handle_func_argc4_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc5( const uint8_t target_tid,
                                               xi_event_handle_func_argc5_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4,
                                               xi_event_handle_arg5_t a5,
                                               const char* file_name,
                                               const int line_no );

extern xi_event_handle_t xi_make_handle_argc6( const uint8_t target_tid,
                                               xi_event_handle_func_argc6_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4,
                                               xi_event_handle_arg5_t a5,
                                               xi_event_handle_arg6_t a6,
                                               const char* file_name,
                                               const int line_no );

#else /* XI_DEBUG_EXTRA_INFO */

#define xi_make_threaded_handle( ... )                                                   \
    xi_make_handle_impl(                                                                 \
        __VA_ARGS__, xi_make_handle_argc6( __VA_ARGS__ ),                                \
        xi_make_handle_argc5( __VA_ARGS__ ), xi_make_handle_argc4( __VA_ARGS__ ),        \
        xi_make_handle_argc3( __VA_ARGS__ ), xi_make_handle_argc2( __VA_ARGS__ ),        \
        xi_make_handle_argc1( __VA_ARGS__ ), xi_make_handle_argc0( __VA_ARGS__ ),        \
        0 ) /* the last 0 to not alow tha impl macro to not to have last parameter       \
             * in case of 1 arg passed */

#define xi_make_empty_handle( ... ) xi_make_empty_handle_impl( XI_THREADID_MAINTHREAD )

extern xi_event_handle_t xi_make_empty_handle_impl( const uint8_t target_tid );

extern xi_event_handle_t
xi_make_handle_argc0( const uint8_t target_tid, xi_event_handle_func_argc0_ptr fn_ptr );

extern xi_event_handle_t xi_make_handle_argc1( const uint8_t target_tid,
                                               xi_event_handle_func_argc1_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1 );

extern xi_event_handle_t xi_make_handle_argc2( const uint8_t target_tid,
                                               xi_event_handle_func_argc2_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2 );

extern xi_event_handle_t xi_make_handle_argc3( const uint8_t target_tid,
                                               xi_event_handle_func_argc3_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3 );

extern xi_event_handle_t xi_make_handle_argc4( const uint8_t target_tid,
                                               xi_event_handle_func_argc4_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4 );

extern xi_event_handle_t xi_make_handle_argc5( const uint8_t target_tid,
                                               xi_event_handle_func_argc5_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4,
                                               xi_event_handle_arg5_t a5 );

extern xi_event_handle_t xi_make_handle_argc6( const uint8_t target_tid,
                                               xi_event_handle_func_argc6_ptr fn_ptr,
                                               xi_event_handle_arg1_t a1,
                                               xi_event_handle_arg2_t a2,
                                               xi_event_handle_arg3_t a3,
                                               xi_event_handle_arg4_t a4,
                                               xi_event_handle_arg5_t a5,
                                               xi_event_handle_arg5_t a6 );
#endif /* XI_DEBUG_EXTRA_INFO */

#ifdef __cplusplus
}
#endif

xi_state_t xi_pointerize_handle( xi_event_handle_t handle, xi_event_handle_t** pointer );

extern void xi_dispose_handle( xi_event_handle_t* handle );

extern uint8_t xi_handle_disposed( xi_event_handle_t* handle );

#endif /* __XI_EVENT_HANDLE_H__ */
