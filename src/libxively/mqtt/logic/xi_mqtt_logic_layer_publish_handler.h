/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__
#define __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__

#include "xi_layer_api.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

void call_topic_handler( void* context /* should be the context of the logic layer */
                         ,
                         void* msg_data );

xi_state_t on_publish_q0_recieved(
    xi_layer_connectivity_t* context /* should be the context of the logic layer */
    ,
    void* data,
    xi_state_t state,
    void* msg_data );

xi_state_t
on_publish_q1_recieved( void* context /* should be the context of the logic layer */
                        ,
                        void* data,
                        xi_state_t state,
                        void* msg_data );

xi_state_t on_publish_recieved(
    xi_layer_connectivity_t* context /* should be the context of the logic layer */
    ,
    xi_mqtt_message_t* msg_memory,
    xi_state_t state );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_PUBLISH_HANDLER_H__ */
