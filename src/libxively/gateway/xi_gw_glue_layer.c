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

xi_state_t xi_gw_glue_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    /*
     - subscribe main client to incoming channel with a callback which feeds back incoming
     message to function xi_gw_glue_layer_pull
     - call API xi_subscribe to do this
     */


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

    xi_data_desc_t* mqtt_message_to_tunnel = ( xi_data_desc_t* )data;

    xi_mqtt_qos_t shell_message_qos       = 0;
    xi_mqtt_retain_t shell_message_retain = 0;

    in_out_state = xi_publish_data(
        XI_CONTEXT_DATA( context )->main_context_handle, "$Tunnel/tunnel-id-guid",
        mqtt_message_to_tunnel->data_ptr, mqtt_message_to_tunnel->length,
        shell_message_qos, shell_message_retain, NULL, NULL );

    return in_out_state;
}

xi_state_t xi_gw_glue_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_data_desc_t* buffer_desc = NULL;

    ( void )data;
    ( void )in_out_state;

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, ( void* )buffer_desc, XI_STATE_OK );
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
