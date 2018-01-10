/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_IO_NET_LAYER_H__
#define __XI_IO_NET_LAYER_H__

#include "xively_error.h"

xi_state_t xi_io_net_layer_init( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_io_net_layer_connect( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_io_net_layer_push( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_io_net_layer_pull( void* context, void* data, xi_state_t in_out_state );

xi_state_t xi_io_net_layer_close( void* context, void* data, xi_state_t in_out_state );

xi_state_t
xi_io_net_layer_close_externally( void* context, void* data, xi_state_t in_out_state );

#endif /* __XI_IO_NET_LAYER_H__ */
