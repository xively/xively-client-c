/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_EVENT_DISPATCHER_API_H__
#define __XI_EVENT_DISPATCHER_API_H__

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "xi_config.h"
#include "xi_vector.h"
#include "xively_time.h"
#include "xi_allocator.h"
#include "xi_macros.h"
#include "xi_event_handle.h"
#include "xi_time_event.h"
#include "xi_event_handle_queue.h"

#include "xi_critical_section.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef intptr_t xi_fd_t;

typedef enum xi_evtd_fd_type_e {
    XI_EVTD_FD_TYPE_SOCKET = 0,
    XI_EVTD_FD_TYPE_FILE
} xi_evtd_fd_type_t;

typedef struct xi_evtd_tuple_s
{
    xi_fd_t fd;
    xi_event_handle_t handle;
    xi_event_handle_t read_handle;
    xi_event_type_t event_type;
    xi_evtd_fd_type_t fd_type;
} xi_evtd_fd_tuple_t;

typedef struct xi_evtd_instance_s
{
    xi_time_t current_step;
    xi_vector_t* time_events_container;
    xi_event_handle_queue_t* call_queue;
    struct xi_critical_section_s* cs;
    xi_vector_t* handles_and_socket_fd;
    xi_vector_t* handles_and_file_fd;
    xi_event_handle_t on_empty;
    uint8_t stop;
} xi_evtd_instance_t;

extern int8_t xi_evtd_register_file_fd( xi_evtd_instance_t* instance,
                                        xi_event_type_t event_type,
                                        xi_fd_t fd,
                                        xi_event_handle_t handle );

extern int8_t xi_evtd_register_socket_fd( xi_evtd_instance_t* instance,
                                          xi_fd_t fd,
                                          xi_event_handle_t read_handle );

extern int8_t xi_evtd_unregister_file_fd( xi_evtd_instance_t* instance, xi_fd_t fd );

extern int8_t xi_evtd_unregister_socket_fd( xi_evtd_instance_t* instance, xi_fd_t fd );

extern int8_t xi_evtd_continue_when_evt_on_socket( xi_evtd_instance_t* instance,
                                                   xi_event_type_t event_type,
                                                   xi_event_handle_t handle,
                                                   xi_fd_t fd );

extern void
xi_evtd_continue_when_empty( xi_evtd_instance_t* instance, xi_event_handle_t handle );

extern xi_event_handle_queue_t*
xi_evtd_execute( xi_evtd_instance_t* instance, xi_event_handle_t handle );

extern xi_state_t xi_evtd_execute_in( xi_evtd_instance_t* instance,
                                      xi_event_handle_t handle,
                                      xi_time_t time_diff,
                                      xi_time_event_handle_t* ret_time_event_handle );

extern xi_state_t
xi_evtd_cancel( xi_evtd_instance_t* instance, xi_time_event_handle_t* time_event_handle );

extern xi_state_t xi_evtd_restart( xi_evtd_instance_t* instance,
                                   xi_time_event_handle_t* time_event_handle,
                                   xi_time_t new_time );

extern xi_evtd_instance_t* xi_evtd_create_instance( void );

extern void xi_evtd_destroy_instance( xi_evtd_instance_t* instance );

extern xi_event_handle_return_t xi_evtd_execute_handle( xi_event_handle_t* handle );

extern uint8_t xi_evtd_single_step( xi_evtd_instance_t* instance, xi_time_t new_step );

extern void xi_evtd_step( xi_evtd_instance_t* instance, xi_time_t new_step );

extern uint8_t xi_evtd_dispatcher_continue( xi_evtd_instance_t* instance );

extern uint8_t
xi_evtd_all_continue( xi_evtd_instance_t** event_dispatchers, uint8_t num_evtds );

extern xi_state_t
xi_evtd_update_event_on_socket( xi_evtd_instance_t* instance, xi_fd_t fds );

extern xi_state_t
xi_evtd_update_event_on_file( xi_evtd_instance_t* instance, xi_fd_t fds );

extern void xi_evtd_stop( xi_evtd_instance_t* instance );

extern xi_event_handle_t
xi_make_event_handle( void* func, xi_event_handle_argc_t argc, ... );

/**
 * @brief xi_evtd_update_file_events
 *
 * Because this function is now platform independent it doesn't need to be implemented
 * explicitly in platform loop
 *
 * @param instance
 * @return 1 if any file has been updated 0 otherwise
 */
extern uint8_t xi_evtd_update_file_fd_events( xi_evtd_instance_t* const instance );

/**
 * @brief xi_evtd_get_time_of_earliest_event
 *
 * Calculates the time of execution of the earliest event registered in event dispatcher.
 *
 * @param instance of an event dispatcher which will be queried for the time
 * @param pointer to the xi_time_t where the value of time of execution of the earliest
 * event will be stored, if there is no events value under the pointer want be modified
 * @return XI_STATE_OK if the earliest event exists, XI_ELEMENT_NOT_FOUND if there is no
 * events
 */
extern xi_state_t xi_evtd_get_time_of_earliest_event( xi_evtd_instance_t* instance,
                                                      xi_time_t* out_timeout );

#ifdef __cplusplus
}
#endif

#endif /* __XI_EVENT_DISPATCHER_API_H__ */
