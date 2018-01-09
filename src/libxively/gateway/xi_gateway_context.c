/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_gateway_context.h>

xi_state_t xi_create_gateway_context_with_custom_layers( xi_gateway_context_t** context,
                                                         xi_layer_type_t layer_config[],
                                                         xi_layer_type_id_t layer_chain[],
                                                         size_t layer_chain_size )
{
    ( void )context;
    ( void )layer_config;
    ( void )layer_chain;
    ( void )layer_chain_size;

    return XI_STATE_OK;
}
