/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef XI_BACKOFF_STATUS_H
#define XI_BACKOFF_STATUS_H

#include <stdint.h>

#include "xi_vector.h"
#include "xi_time_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    XI_BACKOFF_CLASS_NONE = 0,
    XI_BACKOFF_CLASS_RECOVERABLE,
    XI_BACKOFF_CLASS_TERMINAL
} xi_backoff_class_t;

typedef xi_vector_index_type_t xi_backoff_lut_index_t;

typedef struct xi_backoff_status_s
{
    xi_time_event_handle_t next_update;
    xi_vector_t* backoff_lut;
    xi_vector_t* decay_lut;
    xi_backoff_class_t backoff_class;
    xi_backoff_lut_index_t backoff_lut_i;
} xi_backoff_status_t;

#ifdef __cplusplus
}
#endif

#endif /* XI_BACKOFF_STATISTICS_H */
