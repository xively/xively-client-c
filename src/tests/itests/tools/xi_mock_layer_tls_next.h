/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_MOCK_LAYER_TLS_NEXT_H__
#define __XI_ITEST_MOCK_LAYER_TLS_NEXT_H__

#include "xi_layer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_mock_layer_tls_next_push( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_next_pull( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_next_close( void* context, void* data, xi_state_t state );

xi_state_t
xi_mock_layer_tls_next_close_externally( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_next_init( void* context, void* data, xi_state_t state );

xi_state_t xi_mock_layer_tls_next_connect( void* context, void* data, xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_ITEST_MOCK_LAYER_TLS_NEXT_H__ */
