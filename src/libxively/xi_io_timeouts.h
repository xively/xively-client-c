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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    xi_heap_element_t* timeout;
    void* data1;
    void* data2;
} xi_io_timeout_t;

extern xi_heap_element_t* xi_io_timeouts_create( xi_evtd_instance_t* instance,
                                                 xi_event_handle_t handle,
                                                 xi_time_t time_diff,
                                                 xi_vector_t* io_timeouts );

extern xi_heap_element_t* xi_io_timeouts_cancel( xi_evtd_instance_t* instance,
                                                 xi_heap_element_t* heap_element,
                                                 xi_vector_t* io_timeouts );

extern xi_heap_element_t*
xi_io_timeouts_remove( xi_heap_element_t* heap_element, xi_vector_t* io_timeouts );

extern void xi_io_timeouts_restart( xi_evtd_instance_t* instance,
                                    xi_time_t new_time,
                                    xi_vector_t* io_timeouts );

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_TIMEOUTS_H__ */
