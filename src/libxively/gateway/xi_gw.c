/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
#include <xi_gateway_context.h>

#include <xi_vector.h>
#include <xi_helpers.h>

typedef struct xi_ed_id_ed_context_pair_s
{
    char* edge_device_id;
    xi_context_t* edge_device_context;
} xi_ed_id_ed_context_pair_t;

static int8_t _xi_ed_find_context_2_id( const union xi_vector_selector_u* e0,
                                        const union xi_vector_selector_u* e1 )
{
    if ( NULL == e0 || NULL == e1 )
    {
        return 1;
    }

    const xi_ed_id_ed_context_pair_t* pair0 =
        ( xi_ed_id_ed_context_pair_t* )e0->ptr_value;
    const xi_ed_id_ed_context_pair_t* pair1 =
        ( xi_ed_id_ed_context_pair_t* )e1->ptr_value;

    if ( NULL == pair0 || NULL == pair1 )
    {
        return 1;
    }

    return strcmp( pair0->edge_device_id, pair1->edge_device_id );
}

xi_state_t xi_connect_ed( xi_context_handle_t xih, const char* edge_device_id )
{
    ( void )xih;

    xi_state_t state                          = XI_STATE_OK;
    xi_ed_id_ed_context_pair_t* ed_id_context = NULL;

    xi_ed_id_ed_context_pair_t search_key = {.edge_device_id = ( char* )edge_device_id,
                                             .edge_device_context = NULL};

    const xi_vector_index_type_t found_index = xi_vector_find(
        xi_globals.context_handles_vector_edge_devices,
        XI_VEC_CONST_VALUE_PARAM( ( void* )&search_key ), _xi_ed_find_context_2_id );

    if ( -1 == found_index )
    {
        /* context not found to this application device id, create a new context */

        XI_ALLOC_AT( xi_ed_id_ed_context_pair_t, ed_id_context, state );

        ed_id_context->edge_device_id = xi_str_dup( edge_device_id );

        XI_CHECK_STATE(
            state = xi_create_context_with_custom_layers_and_evtd(
                &ed_id_context->edge_device_context, xi_gw_layer_chain, XI_LAYER_CHAIN_GW,
                XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_GW ), NULL, 0 ) );

        const xi_vector_elem_t* e = xi_vector_push(
            xi_globals.context_handles_vector_edge_devices,
            XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ed_id_context ) ) );

        if ( NULL == e )
        {
            goto err_handling;
        }

        /*
         * todo_atigyi:
         * - error handling
         * - get corresponding credentials to application device id
         */
    }
    else
    {
        /* there is already such a context */
    }

    return state;

err_handling:

    XI_SAFE_FREE( ed_id_context->edge_device_id );
    XI_SAFE_FREE( ed_id_context );

    return state;
}

xi_state_t xi_disconnect_ed( xi_context_handle_t xih, const char* edge_device_id )
{
    XI_UNUSED( xih );

    xi_ed_id_ed_context_pair_t search_key = {.edge_device_id = ( char* )edge_device_id,
                                             .edge_device_context = NULL};

    const xi_vector_index_type_t found_index = xi_vector_find(
        xi_globals.context_handles_vector_edge_devices,
        XI_VEC_CONST_VALUE_PARAM( ( void* )&search_key ), _xi_ed_find_context_2_id );

    if ( 0 <= found_index )
    {
        xi_ed_id_ed_context_pair_t* ed_id_context =
            ( xi_ed_id_ed_context_pair_t* )xi_vector_get(
                xi_globals.context_handles_vector_edge_devices, found_index );

        xi_delete_context_with_custom_layers(
            &ed_id_context->edge_device_context, xi_gw_layer_chain,
            XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_GW ) );

        xi_vector_del( xi_globals.context_handles_vector_edge_devices, found_index );

        XI_SAFE_FREE( ed_id_context->edge_device_id );
        XI_SAFE_FREE( ed_id_context );
    }

    return XI_STATE_OK;
}

xi_context_handle_t xi_create_gateway_context( xi_context_handle_t context_handle )
{
    XI_UNUSED( context_handle );

    xi_state_t state = XI_STATE_OK;

    xi_gateway_context_t* gateway_context = NULL;

    XI_CHECK_STATE( state = xi_create_gateway_context_with_custom_layers(
                        &gateway_context, xi_gw_layer_chain, XI_LAYER_CHAIN_GW,
                        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_GW ) ) );

    xi_context_handle_t gateway_context_handle = 0;
    XI_CHECK_STATE( state = xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                       gateway_context,
                                                       &gateway_context_handle ) );

    return gateway_context_handle;

err_handling:

    return -state;
}

xi_state_t xi_delete_gateway_context( xi_context_handle_t gateway_context_handle )
{
    xi_gateway_context_t* gateway_context =
        xi_object_for_handle( xi_globals.context_handles_vector, gateway_context_handle );

    assert( gateway_context != NULL );

    return xi_delete_gateway_context_with_custom_layers(
        &gateway_context, xi_gw_layer_chain,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_GW ) );
}

xi_state_t xi_gateway_publish( xi_context_handle_t gateway_context_handle,
                               const char* edge_device_id,
                               const uint8_t* data,
                               size_t data_len,
                               xi_user_callback_t* callback,
                               void* user_data )
{
    XI_UNUSED( gateway_context_handle );
    XI_UNUSED( edge_device_id );
    XI_UNUSED( data );
    XI_UNUSED( data_len );
    XI_UNUSED( callback );
    XI_UNUSED( user_data );

    /*
     *
     */

    return XI_STATE_OK;
}
