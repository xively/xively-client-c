/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xively_gateway.h>
#include <xi_types.h>
#include <xi_gw_layer_stack.h>
#include <xi_handle.h>
#include <xi_globals.h>
#include <xi_context.h>

xi_context_handle_t xi_create_gateway_context()
{
    xi_context_t* gateway_context = NULL;
    xi_state_t state              = XI_STATE_OK;

    XI_CHECK_STATE( state = xi_create_context_with_custom_layers(
                        &gateway_context, xi_gw_layer_chain, XI_LAYER_CHAIN_GW,
                        XI_LAYER_CHAIN_GWSIZE_SUFFIX ) );

    xi_context_handle_t context_handle = 0;
    XI_CHECK_STATE( state =
                        xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                   gateway_context, &context_handle ) );

    return context_handle;

err_handling:

    return -state;
}

xi_state_t xi_delete_gateway_context( xi_context_handle_t gateway_context_handle )
{
    xi_context_t* gateway_context =
        xi_object_for_handle( xi_globals.context_handles_vector, gateway_context_handle );
    assert( gateway_context != NULL );

    return xi_delete_context_with_custom_layers(
        &gateway_context, xi_gw_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_GW ) );
}
