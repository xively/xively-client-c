/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_HANDLE_TYPEDEF_H__
#define __XI_HANDLE_TYPEDEF_H__

#include "xi_config.h"
#include "xi_event_dispatcher_macros.h"
#include "xi_layer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

XI_EVTD_GENERATE_EVENT_TYPE_ENUM(
    4, XI_EVENT_WANT_READ, XI_EVENT_WANT_WRITE, XI_EVENT_ERROR, XI_EVENT_WANT_CONNECT );

XI_EVTD_GENERATE_EVENT_HANDLE_TYPEDEFS(
    xi_state_t, void*, void*, xi_state_t, void*, void*, void* );

typedef enum {
    XI_EVENT_HANDLE_UNSET = 0,
    XI_EVENT_HANDLE_ARGC0,
    XI_EVENT_HANDLE_ARGC1,
    XI_EVENT_HANDLE_ARGC2,
    XI_EVENT_HANDLE_ARGC3,
    XI_EVENT_HANDLE_ARGC4,
    XI_EVENT_HANDLE_ARGC5,
    XI_EVENT_HANDLE_ARGC6,
} xi_event_handle_argc_t;

typedef struct xi_event_handle_s
{
    /* type of function pointer based on argument count */
    xi_event_handle_argc_t handle_type;
#if XI_DEBUG_EXTRA_INFO
    struct
    {
        int debug_line_init;
        const char* debug_file_init;
        /* TODO:
         * int                     debug_line_last_call;
         * const char*             debug_file_last_call;
         * int                     debug_time_last_call; */
    } debug_info;
#endif
    union {
        struct
        {
            /* function pointer with 0 arguments */
            xi_event_handle_func_argc0_ptr fn_argc0;
        } h0;

        struct
        {
            /* function pointer with 1 argument */
            xi_event_handle_func_argc1_ptr fn_argc1;
            xi_event_handle_arg1_t a1;
        } h1;

        struct
        {
            /* function pointer with 2 arguments */
            xi_event_handle_func_argc2_ptr fn_argc2;
            xi_event_handle_arg1_t a1;
            xi_event_handle_arg2_t a2;
        } h2;

        struct
        {
            /* function pointer with 3 arguments */
            xi_event_handle_func_argc3_ptr fn_argc3;
            xi_event_handle_arg1_t a1;
            xi_event_handle_arg2_t a2;
            xi_event_handle_arg3_t a3;
        } h3;

        struct
        {
            /* function pointer with 4 arguments */
            xi_event_handle_func_argc4_ptr fn_argc4;
            xi_event_handle_arg1_t a1;
            xi_event_handle_arg2_t a2;
            xi_event_handle_arg3_t a3;
            xi_event_handle_arg4_t a4;
        } h4;

        struct
        {
            /* function pointer with 5 arguments */
            xi_event_handle_func_argc5_ptr fn_argc5;
            xi_event_handle_arg1_t a1;
            xi_event_handle_arg2_t a2;
            xi_event_handle_arg3_t a3;
            xi_event_handle_arg4_t a4;
            xi_event_handle_arg5_t a5;
        } h5;

        struct
        {
            /* function pointer with 6 arguments */
            xi_event_handle_func_argc6_ptr fn_argc6;
            xi_event_handle_arg1_t a1;
            xi_event_handle_arg2_t a2;
            xi_event_handle_arg3_t a3;
            xi_event_handle_arg4_t a4;
            xi_event_handle_arg5_t a5;
            xi_event_handle_arg6_t a6;
        } h6;
    } handlers;

    uint8_t target_tid;
} xi_event_handle_t;

#define xi_make_empty_event_handle( target_tid )                                         \
    {                                                                                    \
        XI_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, target_tid                            \
    }

#ifdef __cplusplus
}
#endif

#endif /* __XI_HANDLE_TYPEDEF_H__ */
