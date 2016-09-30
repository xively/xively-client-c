/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_TIMEOUTS_H__
#define __XI_IO_TIMEOUTS_H__

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "xi_time.h"
#include "xi_vector.h"
#include "xi_event_dispatcher_api.h"
#include "xi_time_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    xi_time_event_handle_t timeout;
    void* data1;
    void* data2;
} xi_io_timeout_t;

xi_state_t xi_io_timeouts_create( xi_evtd_instance_t* instance,
                                  xi_event_handle_t handle,
                                  xi_time_t time_diff,
                                  xi_vector_t* io_timeouts,
                                  xi_time_event_handle_t* ret_time_event_handle );

void xi_io_timeouts_cancel( xi_evtd_instance_t* instance,
                            xi_time_event_handle_t* time_event_handle,
                            xi_vector_t* io_timeouts );

void xi_io_timeouts_remove( xi_time_event_handle_t* time_event_handle,
                            xi_vector_t* io_timeouts );

void xi_io_timeouts_restart( xi_evtd_instance_t* instance,
                             xi_time_t new_time,
                             xi_vector_t* io_timeouts );

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_TIMEOUTS_H__ */
