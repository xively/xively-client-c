/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <string.h>
#include <stdio.h>

#include "xi_mqtt_message.h"
#include "xi_debug.h"
#include "xi_macros.h"
#include "xi_allocator.h"

#if XI_DEBUG_OUTPUT
void xi_debug_mqtt_message_dump( const xi_mqtt_message_t* message )
{
    xi_debug_printf( "message\n" );
    xi_debug_printf( "  type:              %d\n",
                     message->common.common_u.common_bits.type );
    xi_debug_printf( "  qos:               %d\n",
                     message->common.common_u.common_bits.qos );
    xi_debug_printf( "  dup:               %s\n",
                     message->common.common_u.common_bits.dup ? "true" : "false" );
    xi_debug_printf( "  retain:            %s\n",
                     message->common.common_u.common_bits.retain ? "true" : "false" );

    if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_CONNECT )
    {
        xi_debug_printf( "  protocol name:     " );
        xi_debug_data_desc_dump( ( message->connect.protocol_name ) );
        xi_debug_printf( "\n" );

        xi_debug_printf( "  protocol version:  %d\n", message->connect.protocol_version );

        xi_debug_printf( "  has username:      %s\n",
                         message->connect.flags_u.flags_bits.username_follows ? "true"
                                                                              : "false" );
        xi_debug_printf( "  has password:      %s\n",
                         message->connect.flags_u.flags_bits.password_follows ? "true"
                                                                              : "false" );
        xi_debug_printf( "  has will:          %s\n",
                         message->connect.flags_u.flags_bits.will ? "true" : "false" );
        xi_debug_printf( "  will qos:          %d\n",
                         message->connect.flags_u.flags_bits.will_qos );
        xi_debug_printf( "  retains will:      %s\n",
                         message->connect.flags_u.flags_bits.will_retain ? "true"
                                                                         : "false" );
        xi_debug_printf( "  clean session:     %s\n",
                         message->connect.flags_u.flags_bits.clean_session ? "true"
                                                                           : "false" );

        xi_debug_printf( "  keep alive:        %d\n", message->connect.keepalive );

        xi_debug_printf( "  client id:         " );
        xi_debug_data_desc_dump( ( message->connect.client_id ) );
        xi_debug_printf( "\n" );

        xi_debug_printf( "  will topic:        " );
        xi_debug_data_desc_dump( ( message->connect.will_topic ) );
        xi_debug_printf( "\n" );
        xi_debug_printf( "  will message:      " );
        xi_debug_data_desc_dump( ( message->connect.will_message ) );
        xi_debug_printf( "\n" );

        xi_debug_printf( "  username:          " );
        xi_debug_data_desc_dump( ( message->connect.username ) );
        xi_debug_printf( "\n" );
        xi_debug_printf( "  password:          " );
        xi_debug_data_desc_dump( ( message->connect.password ) );
        xi_debug_printf( "\n" );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBLISH )
    {
        xi_debug_printf( "  message_id:        %d\n", message->publish.message_id );
        xi_debug_printf( "topic_name: \n" );
        if ( message->publish.topic_name )
            xi_debug_data_desc_dump( message->publish.topic_name );
        xi_debug_printf( "\n" );
        xi_debug_printf( "content: \n" );
        if ( message->publish.content )
            xi_debug_data_desc_dump( message->publish.content );
        xi_debug_printf( "\n" );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBACK )
    {
        xi_debug_printf( "  message_id:        %d\n", message->puback.message_id );
    }
}
#endif

void xi_mqtt_message_free( xi_mqtt_message_t** msg )
{
    if ( msg == NULL || *msg == NULL )
    {
        return;
    }

    xi_mqtt_message_t* m = *msg;

    switch ( m->common.common_u.common_bits.type )
    {
        case XI_MQTT_TYPE_CONNECT:
            if ( m->connect.client_id )
                xi_free_desc( &m->connect.client_id );
            if ( m->connect.password )
                xi_free_desc( &m->connect.password );
            if ( m->connect.username )
                xi_free_desc( &m->connect.username );
            if ( m->connect.protocol_name )
                xi_free_desc( &m->connect.protocol_name );
            if ( m->connect.will_message )
                xi_free_desc( &m->connect.will_message );
            if ( m->connect.will_topic )
                xi_free_desc( &m->connect.will_topic );
            break;
        case XI_MQTT_TYPE_PUBLISH:
            if ( m->publish.content )
                xi_free_desc( &m->publish.content );
            if ( m->publish.topic_name )
                xi_free_desc( &m->publish.topic_name );
            break;
        case XI_MQTT_TYPE_SUBSCRIBE:
            if ( m->subscribe.topics )
            {
                if ( m->subscribe.topics )
                    xi_free_desc( &m->subscribe.topics->name );
                XI_SAFE_FREE( m->subscribe.topics );
            }
            break;
        case XI_MQTT_TYPE_SUBACK:
            if ( m->suback.topics )
            {
                if ( m->suback.topics->name )
                    xi_free_desc( &m->suback.topics->name );
                XI_SAFE_FREE( m->suback.topics );
            }
            break;
    }

    XI_SAFE_FREE( *msg );
}

uint16_t xi_mqtt_get_message_id( const xi_mqtt_message_t* msg )
{
    switch ( msg->common.common_u.common_bits.type )
    {
        case XI_MQTT_TYPE_CONNECT:
            return 0;
        case XI_MQTT_TYPE_CONNACK:
            return 0;
        case XI_MQTT_TYPE_PUBLISH:
            return msg->publish.message_id;
        case XI_MQTT_TYPE_PUBACK:
            return msg->puback.message_id;
        case XI_MQTT_TYPE_PUBREC:
            return msg->pubrec.message_id;
        case XI_MQTT_TYPE_PUBREL:
            return msg->pubrel.message_id;
        case XI_MQTT_TYPE_PUBCOMP:
            return msg->pubcomp.message_id;
        case XI_MQTT_TYPE_SUBSCRIBE:
            return msg->subscribe.message_id;
        case XI_MQTT_TYPE_SUBACK:
            return msg->suback.message_id;
        case XI_MQTT_TYPE_UNSUBACK:
            return msg->unsuback.message_id;
        case XI_MQTT_TYPE_PINGREQ:
            return 0;
        case XI_MQTT_TYPE_PINGRESP:
            return 0;
        case XI_MQTT_TYPE_DISCONNECT:
            return 0;
        default:
            xi_debug_logger( "unhandled msg type... " );
    }

    return 0;
}

xi_mqtt_message_class_t xi_mqtt_class_msg_type_receiving( const xi_mqtt_type_t msg_type )
{
    switch ( msg_type )
    {
        case XI_MQTT_TYPE_PUBLISH:
        case XI_MQTT_TYPE_PUBREL:
            return XI_MQTT_MESSAGE_CLASS_FROM_SERVER;
        case XI_MQTT_TYPE_PUBACK:
        case XI_MQTT_TYPE_PUBREC:
        case XI_MQTT_TYPE_PUBCOMP:
        case XI_MQTT_TYPE_SUBACK:
        case XI_MQTT_TYPE_UNSUBACK:
        case XI_MQTT_TYPE_PINGRESP:
            return XI_MQTT_MESSAGE_CLASS_TO_SERVER;
        default:
            xi_debug_logger( "unhandled msg type..." );
            return XI_MQTT_MESSAGE_CLASS_UNKNOWN;
    }
}

xi_mqtt_message_class_t xi_mqtt_class_msg_type_sending( const xi_mqtt_type_t msg_type )
{
    switch ( msg_type )
    {
        case XI_MQTT_TYPE_PUBACK:
        case XI_MQTT_TYPE_PUBREC:
        case XI_MQTT_TYPE_PUBCOMP:
            return XI_MQTT_MESSAGE_CLASS_FROM_SERVER;
        case XI_MQTT_TYPE_PUBLISH:
        case XI_MQTT_TYPE_PUBREL:
        case XI_MQTT_TYPE_SUBSCRIBE:
        case XI_MQTT_TYPE_UNSUBSCRIBE:
        case XI_MQTT_TYPE_PINGREQ:
        case XI_MQTT_TYPE_DISCONNECT:
            return XI_MQTT_MESSAGE_CLASS_TO_SERVER;
        default:
            xi_debug_logger( "unhandled msg type..." );
            return XI_MQTT_MESSAGE_CLASS_UNKNOWN;
    }
}

xi_state_t xi_mqtt_convert_to_qos( unsigned int qos, xi_mqtt_qos_t* qos_out )
{
    if ( NULL == qos_out )
    {
        return XI_INVALID_PARAMETER;
    }

    switch ( qos )
    {
        case 0:
            *qos_out = XI_MQTT_QOS_AT_MOST_ONCE;
            break;
        case 1:
            *qos_out = XI_MQTT_QOS_AT_LEAST_ONCE;
            break;
        case 2:
            *qos_out = XI_MQTT_QOS_EXACTLY_ONCE;
            break;
        default:
            return XI_INVALID_PARAMETER;
    }

    return XI_STATE_OK;
}

xi_state_t xi_mqtt_convert_to_dup( unsigned int dup, xi_mqtt_dup_t* dup_out )
{
    if ( NULL == dup_out )
    {
        return XI_INVALID_PARAMETER;
    }

    switch ( dup )
    {
        case 0:
            *dup_out = XI_MQTT_DUP_FALSE;
            break;
        case 1:
            *dup_out = XI_MQTT_DUP_TRUE;
            break;
        default:
            return XI_INVALID_PARAMETER;
    }

    return XI_STATE_OK;
}

xi_state_t xi_mqtt_convert_to_retain( unsigned int retain, xi_mqtt_retain_t* retain_out )
{
    if ( NULL == retain_out )
    {
        return XI_INVALID_PARAMETER;
    }

    switch ( retain )
    {
        case 0:
            *retain_out = XI_MQTT_RETAIN_FALSE;
            break;
        case 1:
            *retain_out = XI_MQTT_RETAIN_TRUE;
            break;
        default:
            return XI_INVALID_PARAMETER;
    }

    return XI_STATE_OK;
}
