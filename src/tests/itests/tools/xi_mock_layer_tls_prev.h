/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MOCK_LAYER_TLS_PREV_H__
#define __XI_ITEST_MOCK_LAYER_TLS_PREV_H__

#include "xi_layer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_mock_layer_tls_prev_control_e {
    CONTROL_TLS_PREV_CONTINUE,
    CONTROL_TLS_PREV_PUSH__RETURN_MESSAGE,
    CONTROL_TLS_PREV_CLOSE
} xi_mock_layer_tls_prev_control_t;


xi_state_t xi_mock_layer_tls_prev_push( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_prev_pull( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_prev_close( void* context, void* data, xi_state_t state );

xi_state_t
xi_mock_layer_tls_prev_close_externally( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_prev_init( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_prev_connect( void* context, void* data, xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_ITEST_MOCK_LAYER_TLS_PREV_H__ */
