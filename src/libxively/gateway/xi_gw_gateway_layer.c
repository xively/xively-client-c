/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_gw_gateway_layer.h>
#include <xi_layer_api.h>

xi_state_t xi_gw_gateway_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_gw_gateway_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return in_out_state;
}

xi_state_t xi_gw_gateway_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_gw_gateway_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    /* call the application callback, parameters should include target edge device id and
     * the message body
    */

    return in_out_state;
}

xi_state_t xi_gw_gateway_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_gw_gateway_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;
    ( void )in_out_state;

    return in_out_state;
}
