/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_LOGIC_LAYER_DATA_HELPERS_H__
#define __XI_MQTT_LOGIC_LAYER_DATA_HELPERS_H__

#include "string.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

int8_t
cmp_topics( const union xi_vector_selector_u* a, const union xi_vector_selector_u* b );

xi_state_t fill_with_pingreq_data( xi_mqtt_message_t* msg );

xi_state_t fill_with_puback_data( xi_mqtt_message_t* msg, uint16_t msg_id );

xi_state_t fill_with_connack_data( xi_mqtt_message_t* msg, uint8_t return_code );

xi_state_t fill_with_connect_data( xi_mqtt_message_t* msg,
                                   const char* username,
                                   const char* password,
                                   uint16_t keepalive_timeout,
                                   xi_session_type_t session_type,
                                   const char* will_topic,
                                   const char* will_message,
                                   xi_mqtt_qos_t will_qos,
                                   xi_mqtt_retain_t will_retain );


xi_state_t fill_with_publish_data( xi_mqtt_message_t* msg,
                                   const char* topic,
                                   const xi_data_desc_t* cnt,
                                   const xi_mqtt_qos_t qos,
                                   const xi_mqtt_retain_t retain,
                                   const xi_mqtt_dup_t dup,
                                   const uint16_t id );

xi_state_t fill_with_subscribe_data( xi_mqtt_message_t* msg,
                                     const char* topic,
                                     const uint16_t msg_id,
                                     const xi_mqtt_qos_t qos,
                                     const xi_mqtt_dup_t dup );

xi_state_t fill_with_disconnect_data( xi_mqtt_message_t* msg );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_DATA_HELPERS_H__ */
