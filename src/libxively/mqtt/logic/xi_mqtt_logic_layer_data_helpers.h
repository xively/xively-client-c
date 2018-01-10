/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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

static inline int8_t
cmp_topics( const union xi_vector_selector_u* a, const union xi_vector_selector_u* b )
{
    const xi_mqtt_task_specific_data_t* ca =
        ( const xi_mqtt_task_specific_data_t* )
            a->ptr_value; /* this suppose to be the one from the vector */
    const xi_data_desc_t* cb = ( const xi_data_desc_t* )b->ptr_value; /* this suppose to
                                                                         be the one from
                                                                         the msg */

    if ( memcmp( ca->subscribe.topic, cb->data_ptr, cb->length ) == 0 )
    {
        return 0;
    }

    return 1;
}

static inline xi_state_t fill_with_pingreq_data( xi_mqtt_message_t* msg )
{
    memset( msg, 0, sizeof( xi_mqtt_message_t ) );
    msg->common.common_u.common_bits.type = XI_MQTT_TYPE_PINGREQ;

    return XI_STATE_OK;
}

static inline xi_state_t fill_with_puback_data( xi_mqtt_message_t* msg, uint16_t msg_id )
{
    memset( msg, 0, sizeof( xi_mqtt_message_t ) );
    msg->common.common_u.common_bits.type = XI_MQTT_TYPE_PUBACK;
    msg->puback.message_id                = msg_id;

    return XI_STATE_OK;
}

static inline xi_state_t
fill_with_connack_data( xi_mqtt_message_t* msg, uint8_t return_code )
{
    memset( msg, 0, sizeof( xi_mqtt_message_t ) );
    msg->common.common_u.common_bits.type = XI_MQTT_TYPE_CONNACK;
    msg->connack._unused                  = 0;
    msg->connack.return_code              = return_code;

    return XI_STATE_OK;
}

static inline xi_state_t fill_with_connect_data( xi_mqtt_message_t* msg,
                                                 const char* username,
                                                 const char* password,
                                                 uint16_t keepalive_timeout,
                                                 xi_session_type_t session_type,
                                                 const char* will_topic,
                                                 const char* will_message,
                                                 xi_mqtt_qos_t will_qos,
                                                 xi_mqtt_retain_t will_retain )
{
    xi_state_t local_state = XI_STATE_OK;

    memset( msg, 0, sizeof( xi_mqtt_message_t ) );

    msg->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos    = XI_MQTT_QOS_AT_MOST_ONCE;
    msg->common.common_u.common_bits.dup    = XI_MQTT_DUP_FALSE;
    msg->common.common_u.common_bits.type   = XI_MQTT_TYPE_CONNECT;
    /* These are filled in during serialization if used */
    msg->common.remaining_length                = 0;
    msg->connect.will_topic                     = NULL;
    msg->connect.will_message                   = NULL;
    msg->connect.flags_u.flags_bits.will        = 0;
    msg->connect.flags_u.flags_bits.will_retain = 0;
    msg->connect.flags_u.flags_bits.will_qos    = 0;
    const char* client_id                       = NULL;

    XI_CHECK_MEMORY( msg->connect.protocol_name = xi_make_desc_from_string_copy( "MQTT" ),
                     local_state );
    msg->connect.protocol_version = 4;

    /* our brokers require that we send client_id and username as the same
     * value */
    client_id = username;

    if ( NULL != client_id )
    {
        XI_CHECK_MEMORY( msg->connect.client_id =
                             xi_make_desc_from_string_copy( client_id ),
                         local_state );
    }

    if ( NULL != username )
    {
        msg->connect.flags_u.flags_bits.username_follows = 1;
        XI_CHECK_MEMORY( msg->connect.username =
                             xi_make_desc_from_string_copy( username ),
                         local_state );
    }
    else
    {
        msg->connect.flags_u.flags_bits.username_follows = 0;
    }

    if ( 1 == msg->connect.flags_u.flags_bits.username_follows && NULL != password )
    {
        msg->connect.flags_u.flags_bits.password_follows = 1;
        XI_CHECK_MEMORY( msg->connect.password =
                             xi_make_desc_from_string_copy( password ),
                         local_state );
    }
    else
    {
        msg->connect.flags_u.flags_bits.password_follows = 0;
    }

    /* If only ONE of the will_topic or will_message are set, return an error */
    if ( ( NULL == will_message ) && ( NULL != will_topic ) )
    {
        return XI_NULL_WILL_MESSAGE;
    }
    if ( ( NULL != will_message ) && ( NULL == will_topic ) )
    {
        return XI_NULL_WILL_TOPIC;
    }

    /* If both the will_topic and will_message are set, set the will variables */
    if ( ( NULL != will_message ) && ( NULL != will_topic ) )
    {
        XI_CHECK_MEMORY( msg->connect.will_topic =
                             xi_make_desc_from_string_copy( will_topic ),
                         local_state );
        XI_CHECK_MEMORY( msg->connect.will_message =
                             xi_make_desc_from_string_copy( will_message ),
                         local_state );
        msg->connect.flags_u.flags_bits.will        = 1;
        msg->connect.flags_u.flags_bits.will_retain = will_retain;
        msg->connect.flags_u.flags_bits.will_qos    = will_qos;
    }

    msg->connect.flags_u.flags_bits.clean_session =
        session_type == XI_SESSION_CLEAN ? 1 : 0;

    msg->connect.keepalive = keepalive_timeout;

err_handling:
    return local_state;
}

static inline xi_state_t fill_with_publish_data( xi_mqtt_message_t* msg,
                                                 const char* topic,
                                                 const xi_data_desc_t* cnt,
                                                 const xi_mqtt_qos_t qos,
                                                 const xi_mqtt_retain_t retain,
                                                 const xi_mqtt_dup_t dup,
                                                 const uint16_t id )
{
    xi_state_t local_state = XI_STATE_OK;

    if ( cnt->length > XI_MQTT_MAX_PAYLOAD_SIZE )
    {
        return XI_MQTT_PAYLOAD_SIZE_TOO_LARGE;
    }

    memset( msg, 0, sizeof( xi_mqtt_message_t ) );

    msg->common.common_u.common_bits.retain = retain;
    msg->common.common_u.common_bits.qos    = qos;
    msg->common.common_u.common_bits.dup    = dup;
    msg->common.common_u.common_bits.type   = XI_MQTT_TYPE_PUBLISH;
    msg->common.remaining_length = 0; // this is filled during the serialization

    XI_CHECK_MEMORY( msg->publish.topic_name = xi_make_desc_from_string_share( topic ),
                     local_state );

    XI_CHECK_MEMORY( msg->publish.content =
                         xi_make_desc_from_buffer_share( cnt->data_ptr, cnt->length ),
                     local_state );

    msg->publish.message_id = id;

err_handling:
    return local_state;
}

static inline xi_state_t fill_with_subscribe_data( xi_mqtt_message_t* msg,
                                                   const char* topic,
                                                   const uint16_t msg_id,
                                                   const xi_mqtt_qos_t qos,
                                                   const xi_mqtt_dup_t dup )
{
    xi_state_t local_state = XI_STATE_OK;

    memset( msg, 0, sizeof( xi_mqtt_message_t ) );

    msg->common.common_u.common_bits.retain = XI_MQTT_RETAIN_FALSE;
    msg->common.common_u.common_bits.qos =
        XI_MQTT_QOS_AT_LEAST_ONCE; /* forced by the protocol */
    msg->common.common_u.common_bits.dup  = dup;
    msg->common.common_u.common_bits.type = XI_MQTT_TYPE_SUBSCRIBE;
    msg->common.remaining_length = 0; /* this is filled during the serialization */

    XI_ALLOC_AT( xi_mqtt_topicpair_t, msg->subscribe.topics, local_state );

    XI_CHECK_MEMORY( msg->subscribe.topics->name = xi_make_desc_from_string_copy( topic ),
                     local_state );

    msg->subscribe.message_id                               = msg_id;
    msg->subscribe.topics->xi_mqtt_topic_pair_payload_u.qos = qos;

err_handling:
    return local_state;
}

static inline xi_state_t fill_with_disconnect_data( xi_mqtt_message_t* msg )
{
    memset( msg, 0, sizeof( xi_mqtt_message_t ) );

    msg->common.common_u.common_bits.type = XI_MQTT_TYPE_DISCONNECT;

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_LOGIC_LAYER_DATA_HELPERS_H__ */
