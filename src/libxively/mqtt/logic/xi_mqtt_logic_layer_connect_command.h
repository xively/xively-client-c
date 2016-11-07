/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__
#define __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__

#include "xi_layer_api.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t get_error_from_connack( int return_code );

/**
 * @brief do_mqtt_connect_timeout
 *
 * Connection timeout, remove event from timeout event vector, exit
 */

xi_event_handle_return_t
do_mqtt_connect_timeout( xi_event_handle_arg1_t arg1, xi_event_handle_arg2_t arg2 );

xi_state_t do_mqtt_connect( void* ctx /* should be the context of the logic layer */
                            ,
                            void* data,
                            xi_state_t state,
                            void* msg_data );


#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_CONNECT_COMMAND_H__ */
