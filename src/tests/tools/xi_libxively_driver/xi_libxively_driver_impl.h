// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#ifndef __XI_LIBXIVELY_DRIVER_IMPL_H__
#define __XI_LIBXIVELY_DRIVER_IMPL_H__

#include "xi_event_dispatcher_api.h"
#include "xi_types.h"

typedef struct xi_libxively_driver_s
{
    xi_evtd_instance_t* evtd_instance;
    xi_context_t* context;
} xi_libxively_driver_t;

extern xi_libxively_driver_t* libxively_driver;

xi_libxively_driver_t* xi_libxively_driver_create_instance();

xi_state_t xi_libxively_driver_destroy_instance( xi_libxively_driver_t** driver );

typedef void ( *xi_libxively_driver_callback_function_t )();

xi_state_t xi_libxively_driver_connect_with_callback(
    xi_libxively_driver_t* driver,
    const char* const host,
    uint16_t port,
    xi_libxively_driver_callback_function_t on_connected );

xi_state_t xi_libxively_driver_send_on_connect_finish( xi_libxively_driver_t* driver,
                                                       xi_state_t connect_result );

xi_state_t
xi_libxively_driver_send_on_message_received( xi_libxively_driver_t* driver,
                                              xi_sub_call_type_t call_type,
                                              const xi_sub_call_params_t* const params,
                                              xi_state_t receive_result );

xi_state_t xi_libxively_driver_send_on_publish_finish( xi_libxively_driver_t* driver,
                                                       void* data,
                                                       xi_state_t publish_result );

xi_state_t xi_libxively_driver_send_on_disconnect( xi_libxively_driver_t* driver,
                                                   xi_state_t error_code );

#endif /* __XI_LIBXIVELY_DRIVER_IMPL_H__ */
