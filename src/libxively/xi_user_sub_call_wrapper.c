/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_user_sub_call_wrapper.h"

#include "xively_types.h"
#include "xi_handle.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_globals.h"

xi_state_t xi_user_sub_call_wrapper( void* context,
                                     void* data,
                                     xi_state_t in_state,
                                     void* client_callback,
                                     void* user_data,
                                     void* task_data )
{
    /* check for the existance of parameters - user data may be NULL */
    if ( ( NULL == client_callback ) || ( NULL == task_data ) )
    {
        return XI_INVALID_PARAMETER;
    }

    /* local variables */
    xi_state_t state                       = XI_STATE_OK;
    xi_context_handle_t context_handle     = XI_INVALID_CONTEXT_HANDLE;
    xi_sub_call_params_t params            = XI_EMPTY_SUB_CALL_PARAMS;
    xi_mqtt_message_t* msg                 = NULL;
    xi_mqtt_suback_status_t status         = XI_MQTT_SUBACK_FAILED;
    xi_mqtt_task_specific_data_t* sub_data = ( xi_mqtt_task_specific_data_t* )task_data;

    /* only if the library context is not null */
    if ( NULL != context )
    {
        state = xi_find_handle_for_object( xi_globals.context_handles_vector, context,
                                           &context_handle );
        XI_CHECK_STATE( state );
    }

    switch ( in_state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
        {
            status                      = ( xi_mqtt_suback_status_t )( intptr_t )data;
            params.suback.suback_status = status;
            params.suback.topic         = ( const char* )sub_data->subscribe.topic;

            ( ( xi_user_subscription_callback_t* )( client_callback ) )(
                context_handle, XI_SUB_CALL_SUBACK, &params, in_state, user_data );

            /* now it's ok to free the data as it's no longer needed */
            xi_mqtt_task_spec_data_free_subscribe_data( &sub_data );
        }
        break;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            status                      = ( xi_mqtt_suback_status_t )( intptr_t )data;
            params.suback.suback_status = status;
            params.suback.topic         = ( const char* )sub_data->subscribe.topic;

            ( ( xi_user_subscription_callback_t* )( client_callback ) )(
                context_handle, XI_SUB_CALL_SUBACK, &params, in_state, user_data );
        }
        break;
        case XI_STATE_OK:
        {
            msg = ( xi_mqtt_message_t* )data;

            params.message.temporary_payload_data =
                msg->publish.content ? msg->publish.content->data_ptr : NULL;
            params.message.temporary_payload_data_length =
                msg->publish.content ? msg->publish.content->length : 0;
            params.message.topic = ( const char* )sub_data->subscribe.topic;

            in_state = xi_mqtt_convert_to_qos( msg->common.common_u.common_bits.qos,
                                               &params.message.qos );
            XI_CHECK_STATE( in_state );

            in_state = xi_mqtt_convert_to_dup( msg->common.common_u.common_bits.dup,
                                               &params.message.dup_flag );
            XI_CHECK_STATE( in_state );

            in_state = xi_mqtt_convert_to_retain( msg->common.common_u.common_bits.retain,
                                                  &params.message.retain );
            XI_CHECK_STATE( in_state );

            ( ( xi_user_subscription_callback_t* )( client_callback ) )(
                context_handle, XI_SUB_CALL_MESSAGE, &params, in_state, user_data );
        }
        break;
        default:
        {
            ( ( xi_user_subscription_callback_t* )( client_callback ) )(
                context_handle, XI_SUB_CALL_UNKNOWN, NULL, in_state, user_data );
        }
        break;
    };

    xi_debug_format( "user callback returned with state: %s, (%d)",
                     xi_get_state_string( state ), state );

err_handling:
    /* call "destructor" on message */
    xi_mqtt_message_free( &msg );

    return state;
}
