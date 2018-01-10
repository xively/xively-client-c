/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TLS_LAYER_H__
#define __XI_TLS_LAYER_H__

#include <xively_error.h>

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_tls_layer_push( void* context, void* data, xi_state_t state );

xi_state_t xi_tls_layer_pull( void* context, void* data, xi_state_t state );

xi_state_t xi_tls_layer_init( void* context, void* data, xi_state_t state );

xi_state_t xi_tls_layer_connect( void* context, void* data, xi_state_t state );

xi_state_t xi_tls_layer_close( void* context, void* data, xi_state_t state );

xi_state_t xi_tls_layer_close_externally( void* context, void* data, xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_TLS_LAYER_H__ */
