/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_gw_glue_layer.h>
#include <xi_layer_api.h>
#include <xively_mqtt.h>
#include <xively.h>
#include <xi_types.h>
#include <xively_api_impls.h>

static void
_xi_message_arrived_on_tunnel_callback( xi_context_handle_t in_context_handle,
                                        xi_sub_call_type_t call_type,
                                        const xi_sub_call_params_t* const params,
                                        xi_state_t state,
                                        void* user_data )
{
    printf( "--- %s ---, state: %d, user_data: %p\n", __FUNCTION__, state, user_data );

    XI_UNUSED( in_context_handle );
    XI_UNUSED( call_type );

    if ( XI_MQTT_SUBSCRIPTION_SUCCESSFULL == state )
    {
    }
    else if ( XI_MQTT_SUBSCRIPTION_FAILED == state )
    {
        /* todo_atigyi: probably we should return connection error here since, the
           main client wasn't able to subscribe to the edge device's tunnel topic. */
    }
    else if ( XI_STATE_OK == state )
    {
        /* PUBLISH arrived on tunnel */
        xi_data_desc_t* data = xi_make_desc_from_buffer_copy(
            params->message.temporary_payload_data,
            params->message.temporary_payload_data_length );

        XI_PROCESS_PULL_ON_THIS_LAYER( user_data, data, state );
    }
}

void _xi_publish_result_callback( xi_context_handle_t in_context_handle,
                                  void* user_data,
                                  xi_state_t state )
{
    printf( "--- --- --- %s ---, state: %d, user_data: %p\n", __FUNCTION__, state,
            user_data );
    XI_UNUSED( in_context_handle );

    XI_PROCESS_PUSH_ON_THIS_LAYER( user_data, NULL,
                                   ( XI_STATE_OK == state ) ? XI_STATE_WRITTEN : state );
}

xi_state_t xi_gw_glue_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    /* subscribing to edge device's tunnel topic */
    xi_subscribe_impl( XI_CONTEXT_DATA( context )->main_context_handle,
                       "$Tunnel/tunnel-id-guid", XI_MQTT_QOS_AT_MOST_ONCE,
                       _xi_message_arrived_on_tunnel_callback, context,
                       XI_THREADID_MAINTHREAD );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_gw_glue_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, NULL, in_out_state );
}

xi_state_t xi_gw_glue_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    if ( XI_STATE_OK == in_out_state && NULL != data )
    {
        xi_data_desc_t* mqtt_message_to_tunnel = ( xi_data_desc_t* )data;

        xi_mqtt_qos_t shell_message_qos       = 0;
        xi_mqtt_retain_t shell_message_retain = 0;

        /* MQTT over MQTT, tunneling happens here. Publishing an MQTT encoded message. */
        /* todo_atigyi: use appropriate topic name here for every different edge device */
        in_out_state = xi_publish_data_impl(
            XI_CONTEXT_DATA( context )->main_context_handle, "$Tunnel/tunnel-id-guid",
            mqtt_message_to_tunnel, shell_message_qos, shell_message_retain,
            _xi_publish_result_callback, context, XI_THREADID_MAINTHREAD );
    }
    else if ( XI_STATE_WRITTEN == in_out_state )
    {
        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, in_out_state );
    }

    return in_out_state;
}

xi_state_t xi_gw_glue_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_gw_glue_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_gw_glue_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}
