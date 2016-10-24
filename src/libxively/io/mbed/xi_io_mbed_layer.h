/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_MBED_LAYER_H__
#define __XI_IO_MBED_LAYER_H__

#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_io_mbed_layer_push( struct layer_connectivity_s* context,
                                  const void* data,
                                  const layer_hint_t hint );

xi_state_t xi_io_mbed_layer_pull( struct layer_connectivity_s* context,
                                  const void* data,
                                  const layer_hint_t hint );

xi_state_t xi_io_mbed_layer_close( struct layer_connectivity_s* context );

xi_state_t xi_io_mbed_layer_close_externally( struct layer_connectivity_s* context );

xi_state_t xi_io_mbed_layer_init( struct layer_connectivity_s* context,
                                  const void* data,
                                  const layer_hint_t hint );

xi_state_t xi_io_mbed_layer_connect( struct layer_connectivity_s* context,
                                     const void* data,
                                     const layer_hint_t hint );

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_MBED_LAYER_H__ */
