// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#ifndef __XI_DRIVER_CODEC_PROTOBUF_LAYER_H__
#define __XI_DRIVER_CODEC_PROTOBUF_LAYER_H__

#include "xi_layer_interface.h"

/**
 * driver codec layer:
 * Main purpose is two folded: first is to encode C struct data
 * to binary format to make an IO layer able to forward message
 * to the other side. Second is to decode incoming binary data
 * to C struct to notify driver logic layer about incoming messages.
 *
 * Google's protobuf solution is applied in this version as
 * an encoder / decoder.
 */

xi_state_t
xi_driver_codec_protobuf_layer_push( void* context, void* data, xi_state_t state );

xi_state_t
xi_driver_codec_protobuf_layer_pull( void* context, void* data, xi_state_t state );

xi_state_t
xi_driver_codec_protobuf_layer_close( void* context, void* data, xi_state_t state );

xi_state_t xi_driver_codec_protobuf_layer_close_externally( void* context,
                                                            void* data,
                                                            xi_state_t state );

xi_state_t
xi_driver_codec_protobuf_layer_init( void* context, void* data, xi_state_t state );

xi_state_t
xi_driver_codec_protobuf_layer_connect( void* context, void* data, xi_state_t state );

#endif /* __XI_DRIVER_CODEC_PROTOBUF_LAYER_H__ */
