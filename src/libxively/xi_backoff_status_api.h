/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef XI_BACKOFF_STATUS_API_H
#define XI_BACKOFF_STATUS_API_H

#include <stdint.h>

#include "xi_backoff_status.h"
#include "xi_err.h"
#include "xi_event_dispatcher_api.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void xi_inc_backoff_penalty();

extern void xi_dec_backoff_penalty();

extern uint32_t xi_get_backoff_penalty();

extern void xi_cancel_backoff_event();

#ifdef XI_BACKOFF_GODMODE
extern void xi_reset_backoff_penalty();
#endif

extern xi_state_t xi_backoff_configure_using_data( xi_vector_elem_t* backoff_lut,
                                                   xi_vector_elem_t* decay_lut,
                                                   size_t len,
                                                   xi_memory_type_t memory_type );

extern void xi_backoff_release();

extern xi_backoff_class_t xi_backoff_classify_state( const xi_state_t state );

extern xi_backoff_class_t xi_update_backoff_penalty( const xi_state_t state );

extern xi_state_t xi_restart_update_time();

#ifdef __cplusplus
}
#endif

#endif /* XI_BACKOFF_STATUS_API_H */
