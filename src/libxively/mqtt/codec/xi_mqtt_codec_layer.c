/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_coroutine.h"
#include "xi_list.h"
#include "xi_mqtt_codec_layer.h"
#include "xively.h"
#include "xi_tuples.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_serialiser.h"
#include "xi_mqtt_parser.h"
#include "xi_layer_macros.h"
#include "xi_layer_api.h"

#ifdef __cplusplus
extern "C" {
#endif

static void clear_task_queue( void* context )
{
    /* PRE-CONDITIONS */
    assert( context != 0 );

    xi_mqtt_codec_layer_data_t* layer_data =
        ( xi_mqtt_codec_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    /* clean the queue */
    while ( layer_data->task_queue )
    {
        xi_mqtt_codec_layer_task_t* tmp_task = 0;

        XI_LIST_POP( xi_mqtt_codec_layer_task_t, layer_data->task_queue, tmp_task );

        xi_mqtt_written_data_t* written_data =
            ( xi_mqtt_written_data_t* )xi_alloc_make_tuple(
                xi_mqtt_written_data_t, tmp_task->msg_id, tmp_task->msg_type );

        XI_PROCESS_PUSH_ON_NEXT_LAYER( context, written_data, XI_STATE_FAILED_WRITING );

        xi_mqtt_codec_layer_free_task( &tmp_task );
    }

    /* POST_CONDITIONS */
    assert( layer_data->task_queue == 0 );
}

xi_state_t xi_mqtt_codec_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_mqtt_codec_layer_data_t* layer_data =
        ( xi_mqtt_codec_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_message_t* msg       = ( xi_mqtt_message_t* )data;
    uint8_t* buffer              = NULL;
    xi_data_desc_t* data_desc    = NULL;
    size_t msg_contents_size     = 0;
    size_t remaining_len         = 0;
    size_t publish_payload_len   = 0;
    xi_mqtt_serialiser_rc_t rc   = XI_MQTT_SERIALISER_RC_ERROR;
    xi_data_desc_t* payload_desc = NULL;

    xi_mqtt_serialiser_t serializer;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        /* cleaning of unfinished requests */
        xi_mqtt_message_free( &msg );
        return XI_STATE_OK;
    }

    xi_mqtt_codec_layer_task_t* task =
        ( xi_mqtt_codec_layer_task_t* )layer_data->task_queue;

    /* there can be only one task that is being sent */
    /* if msg != 0 means that we have a notification from next layer */
    if ( XI_STATE_OK == in_out_state && NULL != msg )
    {
        xi_mqtt_codec_layer_task_t* new_task = xi_mqtt_codec_layer_make_task( msg );

        XI_CHECK_MEMORY( new_task, in_out_state );

        XI_LIST_PUSH_BACK( xi_mqtt_codec_layer_task_t, layer_data->task_queue, new_task );

        if ( XI_CR_IS_RUNNING( layer_data->push_cs ) )
        {
            return XI_STATE_OK;
        }
    }
    else if ( in_out_state == XI_STATE_WANT_WRITE )
    {
        /* additional check */
        assert( layer_data->push_cs <= 2 );

        /* make the task to continue it's processing with re-attached msg */
        xi_mqtt_codec_layer_continue_task( task, msg );
    }

    if ( NULL != msg )
    {
        xi_debug_mqtt_message_dump( msg );
    }

    /*------------------------------ BEGIN COROUTINE ----------------------- */
    XI_CR_START( layer_data->push_cs );

    xi_mqtt_serialiser_init( &serializer );

    XI_CHECK_MEMORY( msg, in_out_state );
    layer_data->msg_id   = xi_mqtt_get_message_id( msg );
    layer_data->msg_type = ( xi_mqtt_type_t )msg->common.common_u.common_bits.type;

    in_out_state = xi_mqtt_serialiser_size( &msg_contents_size, &remaining_len,
                                            &publish_payload_len, NULL, msg );

    XI_CHECK_STATE( in_out_state );

    xi_debug_format( "[m.id[%d] m.type[%d]] encoding", layer_data->msg_id,
                     msg->common.common_u.common_bits.type );

    msg_contents_size -= publish_payload_len;

    data_desc = xi_make_empty_desc_alloc( msg_contents_size );

    XI_CHECK_MEMORY( data_desc, in_out_state );

    /* if it's publish then the payload is sent separately
     * for more details check serialiser implementation and this function
     * at line 173 */
    rc = xi_mqtt_serialiser_write( &serializer, msg, data_desc, msg_contents_size,
                                   remaining_len );

    if ( rc == XI_MQTT_SERIALISER_RC_ERROR )
    {
        xi_debug_format( "[m.id[%d] m.type[%d]] mqtt_codec_layer serialization error",
                         layer_data->msg_id, layer_data->msg_type );

        in_out_state = XI_MQTT_SERIALIZER_ERROR;

        goto err_handling;
    }

    xi_debug_format( "[m.id[%d] m.type[%d]] mqtt_codec_layer sending message",
                     layer_data->msg_id, layer_data->msg_type );

    XI_CR_YIELD( layer_data->push_cs,
                 XI_PROCESS_PUSH_ON_PREV_LAYER( context, data_desc, in_out_state ) );

    /* here the state must be either WRITTEN or FAILED_WRITING
     * sanity check to valid the state */
    assert( XI_STATE_WRITTEN == in_out_state || XI_STATE_FAILED_WRITING == in_out_state );

    /* PRE-CONTINUE-CONDITIONS */
    assert( NULL != task );

    /* restore msg pointer since it is no longer available
     * from the function argument */
    msg = task->msg;

    /* PRE-CONTINUE-CONDITIONS */
    assert( NULL != msg );

    /* if header failed no need to send the PUBLISH payload */
    if ( XI_STATE_WRITTEN != in_out_state )
    {
        goto finalise;
    }

    /* if publish and not empty payload then send payload */
    if ( XI_MQTT_TYPE_PUBLISH == msg->common.common_u.common_bits.type &&
         msg->publish.content->length > 0 )
    {
        /* make a new desc but keep sharing memory */
        payload_desc = xi_make_desc_from_buffer_share( msg->publish.content->data_ptr,
                                                       msg->publish.content->length );

        XI_CHECK_MEMORY( payload_desc, in_out_state );

        XI_CR_YIELD( layer_data->push_cs, XI_PROCESS_PUSH_ON_PREV_LAYER(
                                              context, payload_desc, XI_STATE_OK ) );
    }

finalise: /* common part for all messages */
    if ( XI_STATE_WRITTEN == in_out_state )
    {
        xi_debug_format( "[m.id[%d] m.type[%d]] mqtt_codec_layer message sent",
                         layer_data->msg_id, layer_data->msg_type );
    }
    else
    {
        xi_debug_format( "[m.id[%d] m.type[%d]] mqtt_codec_layer message not sent",
                         layer_data->msg_id, layer_data->msg_type );
    }

    /* PRE-CONDITIONS */
    assert( NULL != task );
    assert( NULL != task->msg );

    /* releases the msg memory as it's no longer needed */
    xi_mqtt_message_free( &task->msg );

    /* here the state must be either WRITTEN or FAILED_WRITING
     * sanity check to valid the state */
    assert( in_out_state == XI_STATE_WRITTEN || in_out_state == XI_STATE_FAILED_WRITING );

    xi_mqtt_written_data_t* written_data = xi_alloc_make_tuple(
        xi_mqtt_written_data_t, layer_data->msg_id, layer_data->msg_type );

    XI_CHECK_MEMORY( written_data, in_out_state );

    XI_PROCESS_PUSH_ON_NEXT_LAYER( context, written_data, in_out_state );

    XI_LIST_POP( xi_mqtt_codec_layer_task_t, layer_data->task_queue, task );

    /* release the task as it's no longer required */
    xi_mqtt_codec_layer_free_task( &task );

    /* pop the next task and register it's execution */
    if ( NULL != layer_data->task_queue )
    {
        task = layer_data->task_queue;

        xi_mqtt_message_t* msg_to_send = xi_mqtt_codec_layer_activate_task( task );

        XI_CR_EXIT(
            layer_data->push_cs,
            xi_mqtt_codec_layer_push( context, msg_to_send, XI_STATE_WANT_WRITE ) );
    }

    XI_CR_EXIT( layer_data->push_cs, XI_STATE_OK );

    XI_CR_END();

err_handling:
    xi_debug_format( "something went wrong during mqtt message encoding: %s",
                     xi_get_state_string( in_out_state ) );

    XI_SAFE_FREE( buffer );
    XI_SAFE_FREE( data_desc );
    clear_task_queue( context );
    XI_CR_RESET( layer_data->push_cs );

    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, data, XI_STATE_FAILED_WRITING );
}

xi_state_t xi_mqtt_codec_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    xi_mqtt_codec_layer_data_t* layer_data =
        ( xi_mqtt_codec_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* data_desc = ( xi_data_desc_t* )data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == 0 )
    {
        xi_free_desc( &data_desc );
        return XI_STATE_OK;
    }

    if ( in_out_state != XI_STATE_OK )
    {
        goto err_handling;
    }

    XI_CR_START( layer_data->pull_cs );

    assert( NULL == layer_data->msg );

    XI_ALLOC_AT( xi_mqtt_message_t, layer_data->msg, in_out_state );

    xi_mqtt_parser_init( &layer_data->parser );

    do
    {
        layer_data->local_state =
            xi_mqtt_parser_execute( &layer_data->parser, layer_data->msg, data_desc );

        if ( layer_data->local_state == XI_STATE_WANT_READ )
        {
            xi_free_desc( &data_desc );
        }

        XI_CR_YIELD_UNTIL( layer_data->pull_cs,
                           ( layer_data->local_state == XI_STATE_WANT_READ ),
                           XI_STATE_OK );
    } while ( in_out_state == XI_STATE_OK &&
              layer_data->local_state == XI_STATE_WANT_READ );

    if ( in_out_state != XI_STATE_OK || layer_data->local_state != XI_STATE_OK )
    {
        xi_debug_logger( "error while reading msg!" );
        goto err_handling;
    }

    xi_debug_format( "[m.id[%d] m.type[%d]] msg decoded!",
                     xi_mqtt_get_message_id( layer_data->msg ),
                     layer_data->msg->common.common_u.common_bits.type );

    xi_debug_mqtt_message_dump( layer_data->msg );

    xi_mqtt_message_t* recvd = layer_data->msg;
    layer_data->msg          = NULL;

    /* register next stage of processing */
    XI_PROCESS_PULL_ON_NEXT_LAYER( context, recvd, layer_data->local_state );

    /* reset the coroutine since it has to read new msg */
    XI_CR_RESET( layer_data->pull_cs );

    /* delete the buffer if limit reached or previous call failed
     * reading the message */
    if ( data_desc->curr_pos == data_desc->length ||
         layer_data->local_state != XI_STATE_OK )
    {
        /* delete buffer no longer needed */
        xi_free_desc( &data_desc );
    }
    else
    {
        /* call this function recursively cause there might be next message */
        return xi_mqtt_codec_layer_pull( context, data, in_out_state );
    }

    XI_CR_EXIT( layer_data->pull_cs, XI_STATE_OK );
    XI_CR_END();

err_handling:
    xi_mqtt_message_free( &layer_data->msg );

    if ( data_desc != 0 )
    {
        xi_free_desc( &data_desc );
    }

    if ( layer_data )
    {
        /* reset the coroutine state */
        XI_CR_RESET( layer_data->pull_cs );
        clear_task_queue( context );
    }

    return XI_PROCESS_CLOSE_ON_PREV_LAYER(
        context, 0, XI_MAX( in_out_state, layer_data->local_state ) );
}

xi_state_t xi_mqtt_codec_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    assert( XI_THIS_LAYER( context )->user_data == 0 );

    XI_ALLOC_AT( xi_mqtt_codec_layer_data_t, XI_THIS_LAYER( context )->user_data,
                 in_out_state );

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );

err_handling:
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mqtt_codec_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();
    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_mqtt_codec_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    const xi_mqtt_codec_layer_data_t* layer_data =
        ( xi_mqtt_codec_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    return ( NULL == layer_data )
               ? XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state )
               : XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mqtt_codec_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_mqtt_codec_layer_data_t* layer_data =
        ( xi_mqtt_codec_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data )
    {
        clear_task_queue( context );
        XI_CR_RESET( layer_data->push_cs );

        xi_mqtt_message_free( &layer_data->msg );

        XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

#ifdef __cplusplus
}
#endif
