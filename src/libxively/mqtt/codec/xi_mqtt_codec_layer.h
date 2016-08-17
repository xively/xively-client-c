/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_MQTT_CODEC_LAYER_H__
#define __XI_MQTT_CODEC_LAYER_H__

#include "xi_layer.h"
#include "xi_common.h"
#include "xi_mqtt_codec_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_mqtt_codec_layer_push( void* context, void* data, xi_state_t );

xi_state_t xi_mqtt_codec_layer_pull( void* context, void* data, xi_state_t );

xi_state_t xi_mqtt_codec_layer_init( void* context, void* data, xi_state_t );

xi_state_t xi_mqtt_codec_layer_connect( void* context, void* data, xi_state_t );

xi_state_t xi_mqtt_codec_layer_close( void* context, void* data, xi_state_t state );

xi_state_t
xi_mqtt_codec_layer_close_externally( void* context, void* data, xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_CODEC_LAYER_H__ */
