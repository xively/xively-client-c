/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "xi_coroutine.h"
#include "xi_layer_interface.h"
#include "xi_macros.h"
#include "xi_mqtt_errors.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_parser.h"
#include "xi_macros.h"
#include "xi_allocator.h"

static xi_state_t
read_string( xi_mqtt_parser_t* parser, xi_data_desc_t** dst, xi_data_desc_t* src )
{
    assert( NULL != parser );
    assert( NULL != dst );
    assert( NULL != src );

    /* local state */
    xi_state_t local_state = XI_STATE_OK;
    size_t to_read         = 0;
    size_t src_left        = 0;
    size_t len_to_read     = 0;

    /* check for the existance of the data descriptor */
    if ( NULL == *dst )
    {
        XI_CHECK_MEMORY( *dst = xi_make_empty_desc_alloc( 4 ), local_state );
    }

    /* local variables */
    to_read     = parser->str_length - ( *dst )->length;
    src_left    = src->length - src->curr_pos;
    len_to_read = XI_MIN( to_read, src_left );

    XI_CR_START( parser->read_cs );

    XI_CR_YIELD_ON( parser->read_cs, ( ( src->curr_pos - src->length ) == 0 ),
                    XI_STATE_WANT_READ );
    parser->str_length = ( src->data_ptr[src->curr_pos] << 8 );
    src->curr_pos += 1;
    parser->data_length += 1;

    XI_CR_YIELD_ON( parser->read_cs, ( ( src->curr_pos - src->length ) == 0 ),
                    XI_STATE_WANT_READ );
    parser->str_length += src->data_ptr[src->curr_pos];
    src->curr_pos += 1;
    parser->data_length += 1;

    to_read     = parser->str_length - ( *dst )->length;
    src_left    = src->length - src->curr_pos;
    len_to_read = XI_MIN( to_read, src_left );

    XI_CR_YIELD_ON( parser->read_cs, ( ( src->curr_pos - src->length ) == 0 ),
                    XI_STATE_WANT_READ );

    /* @TODO code duplication with read_data -> solve this via
     * code extraction to separate function */
    while ( to_read > 0 )
    {
        XI_CHECK_STATE(
            local_state = xi_data_desc_append_data_resize(
                ( *dst ), ( const char* )src->data_ptr + src->curr_pos, len_to_read ) );

        src->curr_pos += len_to_read;
        to_read -= len_to_read;
        parser->data_length += len_to_read;

        XI_CR_YIELD_UNTIL( parser->read_cs, ( to_read > 0 ), XI_STATE_WANT_READ );
    }

    XI_CR_RESTART( parser->read_cs,
                   ( to_read == 0 ? XI_STATE_OK : XI_MQTT_PARSER_ERROR ) );

err_handling:
    XI_CR_EXIT( parser->read_cs, local_state );

    XI_CR_END();
}

xi_state_t
xi_mqtt_parse_suback_response( xi_mqtt_suback_status_t* dst, const uint8_t resp )
{
    assert( NULL != dst );

    switch ( resp )
    {
        case XI_MQTT_QOS_0_GRANTED:
        case XI_MQTT_QOS_1_GRANTED:
        case XI_MQTT_QOS_2_GRANTED:
        case XI_MQTT_SUBACK_FAILED:
            *dst = ( xi_mqtt_suback_status_t )resp;
            break;
        default:
            return XI_MQTT_PARSER_ERROR;
    }

    return XI_STATE_OK;
}

static xi_state_t
read_data( xi_mqtt_parser_t* parser, xi_data_desc_t** dst, xi_data_desc_t* src )
{
    assert( NULL != parser );
    assert( NULL != dst );
    assert( NULL != src );

    /* local state */
    xi_state_t local_state = XI_STATE_OK;
    size_t to_read         = 0;
    size_t src_left        = 0;
    size_t len_to_read     = 0;

    if ( NULL == *dst )
    {
        XI_CHECK_MEMORY( *dst = xi_make_empty_desc_alloc( 4 ), local_state );
    }

    /* local variables */
    to_read     = parser->str_length - ( *dst )->length;
    src_left    = src->length - src->curr_pos;
    len_to_read = XI_MIN( to_read, src_left );

    XI_CR_START( parser->read_cs );

    XI_CR_YIELD_ON( parser->read_cs, ( ( src->curr_pos - src->length ) == 0 ),
                    XI_STATE_WANT_READ );

    while ( to_read > 0 )
    {
        XI_CHECK_STATE(
            local_state = xi_data_desc_append_data_resize(
                ( *dst ), ( const char* )src->data_ptr + src->curr_pos, len_to_read ) );

        src->curr_pos += len_to_read;
        to_read -= len_to_read;

        XI_CR_YIELD_UNTIL( parser->read_cs, ( to_read > 0 ), XI_STATE_WANT_READ );
    }

    XI_CR_RESTART( parser->read_cs,
                   ( to_read == 0 ? XI_STATE_OK : XI_MQTT_PARSER_ERROR ) );

err_handling:
    XI_CR_EXIT( parser->read_cs, local_state );

    XI_CR_END();
}

#define READ_STRING( into )                                                              \
    do                                                                                   \
    {                                                                                    \
        local_state = read_string( parser, into, src );                                  \
        XI_CR_YIELD_UNTIL( parser->cs, ( local_state == XI_STATE_WANT_READ ),            \
                           XI_STATE_WANT_READ );                                         \
        if ( local_state != XI_STATE_OK )                                                \
        {                                                                                \
            XI_CR_EXIT( parser->cs, local_state );                                       \
        }                                                                                \
    } while ( local_state != XI_STATE_OK )

#define READ_DATA( into )                                                                \
    do                                                                                   \
    {                                                                                    \
        local_state = read_data( parser, into, src );                                    \
        XI_CR_YIELD_UNTIL( parser->cs, ( local_state == XI_STATE_WANT_READ ),            \
                           XI_STATE_WANT_READ );                                         \
        if ( local_state != XI_STATE_OK )                                                \
        {                                                                                \
            XI_CR_EXIT( parser->cs, local_state );                                       \
        }                                                                                \
    } while ( local_state != XI_STATE_OK )

void xi_mqtt_parser_init( xi_mqtt_parser_t* parser )
{
    memset( parser, 0, sizeof( xi_mqtt_parser_t ) );
}

void xi_mqtt_parser_buffer( xi_mqtt_parser_t* parser,
                            uint8_t* buffer,
                            size_t buffer_length )
{
    parser->buffer_pending = 1;
    parser->buffer         = buffer;
    parser->buffer_length  = buffer_length;
}

xi_state_t xi_mqtt_parser_execute( xi_mqtt_parser_t* parser,
                                   xi_mqtt_message_t* message,
                                   xi_data_desc_t* data_buffer_desc )
{
    xi_data_desc_t* src           = data_buffer_desc;
    static xi_state_t local_state = XI_STATE_OK;

    XI_CR_START( parser->cs );

    local_state = XI_STATE_OK;

    XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                    XI_STATE_WANT_READ );

    message->common.common_u.common_value = src->data_ptr[src->curr_pos];

    /* remaining length */
    parser->digit_bytes      = 0;
    parser->multiplier       = 1;
    parser->remaining_length = 0;
    parser->data_length      = 1;

    do
    {
        src->curr_pos += 1;
        parser->digit_bytes += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        parser->remaining_length +=
            ( src->data_ptr[src->curr_pos] & 0x7f ) * parser->multiplier;
        parser->multiplier *= 128;
    } while ( ( ( uint8_t )src->data_ptr[src->curr_pos] & 0x80 ) != 0 &&
              parser->digit_bytes < 4 );

    if ( ( uint8_t )src->data_ptr[src->curr_pos] >= 0x80 )
    {
        parser->error = XI_MQTT_ERROR_PARSER_INVALID_REMAINING_LENGTH;

        XI_CR_EXIT( parser->cs, XI_MQTT_PARSER_ERROR );
    }

    message->common.remaining_length = parser->remaining_length;

    src->curr_pos += 1;
    parser->data_length += 1;

    if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_CONNECT )
    {
        READ_STRING( &message->connect.protocol_name );

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connect.protocol_version = src->data_ptr[src->curr_pos];

        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connect.flags_u.flags_value = src->data_ptr[src->curr_pos];

        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connect.keepalive = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connect.keepalive += src->data_ptr[src->curr_pos];
        src->curr_pos += 1;
        parser->data_length += 1;

        READ_STRING( &message->connect.client_id );

        if ( message->connect.flags_u.flags_bits.will )
        {
            READ_STRING( &message->connect.will_topic );
        }

        if ( message->connect.flags_u.flags_bits.will )
        {
            READ_STRING( &message->connect.will_message );
        }

        if ( message->connect.flags_u.flags_bits.username_follows )
        {
            READ_STRING( &message->connect.username );
        }

        if ( message->connect.flags_u.flags_bits.password_follows )
        {
            READ_STRING( &message->connect.password );
        }

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_CONNACK )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connack._unused = src->data_ptr[src->curr_pos];
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->connack.return_code = src->data_ptr[src->curr_pos];
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBLISH )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        READ_STRING( &message->publish.topic_name );

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        if ( message->common.common_u.common_bits.qos > 0 )
        {
            message->publish.message_id = ( src->data_ptr[src->curr_pos] << 8 );
            src->curr_pos += 1;
            parser->data_length += 1;

            XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                            XI_STATE_WANT_READ );

            message->publish.message_id += src->data_ptr[src->curr_pos] & 0xFF;
            src->curr_pos += 1;
            parser->data_length += 1;
        }

        parser->str_length = ( parser->remaining_length + 2 ) - parser->data_length;

        if ( parser->str_length > 0 )
        {
            READ_DATA( &message->publish.content );
        }

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBACK )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ )

        message->puback.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ )

        message->puback.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBREC )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ )

        message->pubrec.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ )

        message->pubrec.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBREL )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->pubrel.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->pubrel.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBCOMP )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->pubcomp.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->pubcomp.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_SUBSCRIBE )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ )

        message->subscribe.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->subscribe.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        XI_ALLOC_AT( xi_mqtt_topicpair_t, message->subscribe.topics, local_state );

        READ_STRING( &message->subscribe.topics->name );

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->subscribe.topics->xi_mqtt_topic_pair_payload_u.qos =
            ( xi_mqtt_qos_t )src->data_ptr[src->curr_pos];
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_SUBACK )
    {
        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->suback.message_id = ( src->data_ptr[src->curr_pos] << 8 );
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        message->suback.message_id += src->data_ptr[src->curr_pos] & 0xFF;
        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_YIELD_ON( parser->cs, ( ( src->curr_pos - src->length ) == 0 ),
                        XI_STATE_WANT_READ );

        XI_ALLOC_AT( xi_mqtt_topicpair_t, message->suback.topics, local_state );

        XI_CHECK_STATE( local_state = xi_mqtt_parse_suback_response(
                            &message->suback.topics->xi_mqtt_topic_pair_payload_u.status,
                            src->data_ptr[src->curr_pos] ) );

        src->curr_pos += 1;
        parser->data_length += 1;

        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_PINGRESP )
    {
        /* nothing to parse */
        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else if ( message->common.common_u.common_bits.type == XI_MQTT_TYPE_DISCONNECT )
    {
        /* nothing to parse */
        XI_CR_EXIT( parser->cs, XI_STATE_OK );
    }
    else
    {
        xi_debug_logger( "XI_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID" );
        parser->error = XI_MQTT_ERROR_PARSER_INVALID_MESSAGE_ID;
        XI_CR_RESET( parser->read_cs );
        XI_CR_EXIT( parser->cs, XI_MQTT_PARSER_ERROR );
    }

err_handling:
    XI_CR_EXIT( parser->cs, local_state );

    XI_CR_END();
}
