/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_gateway_context.h>
#include <xi_handle.h>
#include <xi_globals.h>

xi_state_t xi_create_gateway_context_with_custom_layers( xi_gateway_context_t** context,
                                                         xi_layer_type_t layer_config[],
                                                         xi_layer_type_id_t layer_chain[],
                                                         size_t layer_chain_size )
{
    xi_state_t state = XI_STATE_OK;

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    *context = NULL;

    XI_ALLOC_AT( xi_gateway_context_t, *context, state );

    ( *context )->layer_chain = xi_layer_chain_create(
        layer_chain, layer_chain_size, &( *context )->context_data, layer_config );

    XI_CHECK_STATE( state = xi_register_handle_for_object(
                        xi_globals.context_handles_vector, 111, *context ) );

    return state;

err_handling:

    XI_SAFE_FREE( *context );

    return state;
}

xi_state_t xi_delete_gateway_context_with_custom_layers( xi_gateway_context_t** context,
                                                         xi_layer_type_t layer_config[],
                                                         size_t layer_chain_size )
{
    if ( NULL == *context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_delete_handle_for_object( xi_globals.context_handles_vector, *context );

    xi_layer_chain_delete( &( ( *context )->layer_chain ), layer_chain_size,
                           layer_config );

    XI_SAFE_FREE( *context );

    return XI_STATE_OK;
}
