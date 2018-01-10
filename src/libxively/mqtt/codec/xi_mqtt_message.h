/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_MESSAGE_H__
#define __XI_MQTT_MESSAGE_H__

#include <stdio.h>
#include <stdint.h>

#include "xi_data_desc.h"
#include <xively_mqtt.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_mqtt_type_e {
    XI_MQTT_TYPE_NONE        = 0,
    XI_MQTT_TYPE_CONNECT     = 1,
    XI_MQTT_TYPE_CONNACK     = 2,
    XI_MQTT_TYPE_PUBLISH     = 3,
    XI_MQTT_TYPE_PUBACK      = 4,
    XI_MQTT_TYPE_PUBREC      = 5,
    XI_MQTT_TYPE_PUBREL      = 6,
    XI_MQTT_TYPE_PUBCOMP     = 7,
    XI_MQTT_TYPE_SUBSCRIBE   = 8,
    XI_MQTT_TYPE_SUBACK      = 9,
    XI_MQTT_TYPE_UNSUBSCRIBE = 10,
    XI_MQTT_TYPE_UNSUBACK    = 11,
    XI_MQTT_TYPE_PINGREQ     = 12,
    XI_MQTT_TYPE_PINGRESP    = 13,
    XI_MQTT_TYPE_DISCONNECT  = 14,
} xi_mqtt_type_t;

typedef enum {
    XI_MQTT_MESSAGE_CLASS_UNKNOWN = 0,
    XI_MQTT_MESSAGE_CLASS_TO_SERVER,
    XI_MQTT_MESSAGE_CLASS_FROM_SERVER
} xi_mqtt_message_class_t;

typedef struct xi_mqtt_topic_s
{
    struct mqtt_topic_s* next;
    xi_data_desc_t* name;
} xi_mqtt_topic_t;

typedef struct xi_mqtt_topicpair_s
{
    struct xi_mqtt_topicpair_s* next;
    xi_data_desc_t* name;
    union {
        xi_mqtt_suback_status_t status;
        xi_mqtt_qos_t qos;
    } xi_mqtt_topic_pair_payload_u;
} xi_mqtt_topicpair_t;

typedef union xi_mqtt_message_u {
    struct common_s
    {
        union {
            struct
            {
                unsigned int retain : 1;
                unsigned int qos : 2;
                unsigned int dup : 1;
                unsigned int type : 4;
            } common_bits;
            uint8_t common_value;
        } common_u;
        uint32_t remaining_length;
    } common;

    struct
    {
        struct common_s common;

        xi_data_desc_t* protocol_name;
        uint8_t protocol_version;

        union {
            struct
            {
                unsigned int reserverd : 1;
                unsigned int clean_session : 1;
                unsigned int will : 1;
                unsigned int will_qos : 2;
                unsigned int will_retain : 1;
                unsigned int password_follows : 1;
                unsigned int username_follows : 1;
            } flags_bits;
            uint8_t flags_value;
        } flags_u;

        uint16_t keepalive;

        xi_data_desc_t* client_id;

        xi_data_desc_t* will_topic;
        xi_data_desc_t* will_message;

        xi_data_desc_t* username;
        xi_data_desc_t* password;
    } connect;

    struct
    {
        struct common_s common;

        uint8_t _unused;
        uint8_t return_code;
    } connack;

    struct
    {
        struct common_s common;

        xi_data_desc_t* topic_name;
        uint16_t message_id;

        xi_data_desc_t* content;
    } publish;

    struct
    {
        struct common_s common;

        uint16_t message_id;
    } puback;

    struct
    {
        struct common_s common;

        uint16_t message_id;
    } pubrec;

    struct
    {
        struct common_s common;

        uint16_t message_id;
    } pubrel;

    struct
    {
        struct common_s common;

        uint16_t message_id;
    } pubcomp;

    struct
    {
        struct common_s common;

        uint16_t message_id;
        xi_mqtt_topicpair_t* topics;
    } subscribe;

    struct
    {
        struct common_s common;

        uint16_t message_id;
        xi_mqtt_topicpair_t* topics;
    } suback;

    struct
    {
        struct common_s common;

        uint16_t message_id;
        xi_mqtt_topic_t* topics;
    } unsubscribe;

    struct
    {
        struct common_s common;

        uint16_t message_id;
    } unsuback;

    struct
    {
        struct common_s common;
    } pingreq;

    struct
    {
        struct common_s common;
    } pingresp;

    struct
    {
        struct common_s common;
    } disconnect;
} xi_mqtt_message_t;

#if XI_DEBUG_OUTPUT
extern void xi_debug_mqtt_message_dump( const xi_mqtt_message_t* msg );
#else
#define xi_debug_mqtt_message_dump( ... )
#endif

extern void xi_mqtt_message_free( xi_mqtt_message_t** msg );

/**
 * @name    xi_mqtt_class_msg_type_receiving
 * @brief   Classifies the message while executing the receiving code
 *
 * Message classification itself is required to pick up the proper qeueue of logic tasks
 * to avoid message id collision for incoming chains ( like receiving PUBLISH ) with
 * outgoing chains( like sendingmSUBSCRIBE or PUBLISH )
 *
 * There are two situation that we might want to classify the message:
 *      - sending the message   ( push )
 *      - receiving the message ( pull )
 *
 * There are two possible scenarios that can happen during classification:
 *      - message is classified as a part of outgoing message chain ( like sending
 * PUBLISH, or KEEPALIVE )
 *      - message is classified as a part of incoming message chain ( like receiving
 * PUBLISH on 1st and 2nd QoS levels )
 *
 */
extern xi_mqtt_message_class_t
xi_mqtt_class_msg_type_receiving( const xi_mqtt_type_t msg_type );

/**
 * @name    xi_mqtt_class_msg_type_sending
 * @brief   Classifies the message while executing the sending code
 *
 * @see xi_mqtt_class_msg_type_receiving
 */
extern xi_mqtt_message_class_t
xi_mqtt_class_msg_type_sending( const xi_mqtt_type_t msg_type );

extern uint16_t xi_mqtt_get_message_id( const xi_mqtt_message_t* msg );


extern xi_state_t xi_mqtt_convert_to_qos( unsigned int qos, xi_mqtt_qos_t* qos_out );

extern xi_state_t xi_mqtt_convert_to_dup( unsigned int dup, xi_mqtt_dup_t* dup_out );

extern xi_state_t
xi_mqtt_convert_to_retain( unsigned int retain, xi_mqtt_retain_t* retain_out );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_MESSAGE_H__ */
