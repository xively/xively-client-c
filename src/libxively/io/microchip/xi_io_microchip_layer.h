/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_MICROCHIP_LAYER_H__
#define __XI_IO_MIROCHIP_LAYER_H__

#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_io_microchip_layer_push( void* context, void* data, xi_state_t state );

xi_state_t xi_io_microchip_layer_pull( void* context, void* data, xi_state_t state );

xi_state_t xi_io_microchip_layer_close( void* context, void* data, xi_state_t state );

xi_state_t
xi_io_microchip_layer_close_externally( void* context, void* data, xi_state_t state );

xi_state_t xi_io_microchip_layer_init( void* context, void* data, xi_state_t state );

xi_state_t xi_io_microchip_layer_connect( void* context, void* data, xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_POSIX_LAYER_H__ */
