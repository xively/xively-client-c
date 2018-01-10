/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_EVENT_DISPATCHER_MACROS_H__
#define __XI_EVENT_DISPATCHER_MACROS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XI_EVTD_DECLARE_1_EV( e1 ) +1, e1##_ID = 1, e1 = 1 << 1
#define XI_EVTD_DECLARE_2_EV( e1, e2 )                                                   \
    +1 XI_EVTD_DECLARE_1_EV( e1 ), e2##_ID = 2, e2 = 1 << 2
#define XI_EVTD_DECLARE_3_EV( e1, e2, e3 )                                               \
    +1 XI_EVTD_DECLARE_2_EV( e1, e2 ), e3##_ID = 3, e3 = 1 << 3
#define XI_EVTD_DECLARE_4_EV( e1, e2, e3, e4 )                                           \
    +1 XI_EVTD_DECLARE_3_EV( e1, e2, e3 ), e4##_ID = 4, e4 = 1 << 4
#define XI_EVTD_DECLARE_5_EV( e1, e2, e3, e4, e5 )                                       \
    +1 XI_EVTD_DECLARE_4_EV( e1, e2, e3, e4 ), e5##_ID = 5, e5 = 1 << 5
#define XI_EVTD_DECLARE_6_EV( e1, e2, e3, e4, e5, e6 )                                   \
    +1 XI_EVTD_DECLARE_5_EV( e1, e2, e3, e4, e5 ), e5##_ID = 6, e6 = 1 << 6

#define XI_EVTD_GENERATE_EVENT_TYPE_ENUM( count, ... )                                   \
    typedef enum evts {                                                                  \
        XI_EVTD_NO_EVENT    = 1 << 0,                                                    \
        XI_EVTD_NO_EVENT_ID = 0,                                                         \
        XI_EVTD_COUNT       = 1 XI_EVTD_DECLARE_##count##_EV( __VA_ARGS__ )              \
    } xi_event_type_t;

#define XI_EVTD_GENERATE_EVENT_HANDLE_TYPEDEFS( ret, arg1, arg2, arg3, arg4, arg5,       \
                                                arg6 )                                   \
    typedef ret xi_event_handle_return_t;                                                \
    typedef arg1 xi_event_handle_arg1_t;                                                 \
    typedef arg2 xi_event_handle_arg2_t;                                                 \
    typedef arg3 xi_event_handle_arg3_t;                                                 \
    typedef arg4 xi_event_handle_arg4_t;                                                 \
    typedef arg5 xi_event_handle_arg5_t;                                                 \
    typedef arg6 xi_event_handle_arg6_t;                                                 \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc0 )( void );              \
    typedef xi_event_handle_func_argc0* xi_event_handle_func_argc0_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc1 )(                      \
        xi_event_handle_arg1_t );                                                        \
    typedef xi_event_handle_func_argc1* xi_event_handle_func_argc1_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc2 )(                      \
        xi_event_handle_arg1_t, xi_event_handle_arg2_t );                                \
    typedef xi_event_handle_func_argc2* xi_event_handle_func_argc2_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc3 )(                      \
        xi_event_handle_arg1_t, xi_event_handle_arg2_t, xi_event_handle_arg3_t );        \
    typedef xi_event_handle_func_argc3* xi_event_handle_func_argc3_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc4 )(                      \
        xi_event_handle_arg1_t, xi_event_handle_arg2_t, xi_event_handle_arg3_t,          \
        xi_event_handle_arg4_t );                                                        \
    typedef xi_event_handle_func_argc4* xi_event_handle_func_argc4_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc5 )(                      \
        xi_event_handle_arg1_t, xi_event_handle_arg2_t, xi_event_handle_arg3_t,          \
        xi_event_handle_arg4_t, xi_event_handle_arg5_t );                                \
    typedef xi_event_handle_func_argc5* xi_event_handle_func_argc5_ptr;                  \
                                                                                         \
    typedef xi_event_handle_return_t( xi_event_handle_func_argc6 )(                      \
        xi_event_handle_arg1_t, xi_event_handle_arg2_t, xi_event_handle_arg3_t,          \
        xi_event_handle_arg4_t, xi_event_handle_arg5_t, xi_event_handle_arg6_t );        \
    typedef xi_event_handle_func_argc6* xi_event_handle_func_argc6_ptr;
#ifdef __cplusplus
}
#endif

#endif /* __XI_EVENT_DISPATCHER_MACROS_H__ */
