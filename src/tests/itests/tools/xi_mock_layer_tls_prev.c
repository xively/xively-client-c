/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_mock_layer_tls_prev.h"
#include "xi_layer_macros.h"
#include "xi_itest_helpers.h"
#include "xi_itest_mock_broker_layerchain.h"
#include "xi_globals.h"

#ifdef __cplusplus
extern "C" {
#endif

extern xi_context_t* xi_context_mockbroker; // test mock broker context

xi_mock_broker_control_t xi_mock_layer_tls_prev__check_expected__LAYER_LEVEL()
{
    return mock_type( xi_mock_broker_control_t );
}

#define XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( variable_to_check, level )           \
    if ( CONTROL_SKIP_CHECK_EXPECTED !=                                                  \
         xi_mock_layer_tls_prev__check_expected__##level() )                             \
    {                                                                                    \
        check_expected( variable_to_check );                                             \
    }

xi_state_t
xi_mock_layer_tls_prev_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( itest_mock_broker_codec_layer_chain );
    XI_UNUSED( XI_LAYER_CHAIN_MOCK_BROKER_CODECSIZE_SUFFIX );

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    const xi_mock_layer_tls_prev_control_t mock_control_directive =
        mock_type( xi_mock_layer_tls_prev_control_t );

    xi_data_desc_t* data_desc = ( xi_data_desc_t* )data;
    xi_free_desc( &data_desc );

    switch ( mock_control_directive )
    {
        case CONTROL_TLS_PREV_CONTINUE:
            break;
        case CONTROL_TLS_PREV_PUSH__RETURN_MESSAGE:
        {
            const char* string_to_send_back = mock_type( const char* );

            if ( string_to_send_back != 0 )
            {
                xi_data_desc_t* message_to_send_back =
                    xi_make_desc_from_string_copy( string_to_send_back );

                XI_PROCESS_PULL_ON_THIS_LAYER( context, message_to_send_back,
                                               XI_STATE_OK );
            }
        }
        break;
        case CONTROL_TLS_PREV_CLOSE:
            return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL,
                                                   mock_type( xi_state_t ) );
            break;
        default:
            break;
    }

    // pretend always successful send
    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, XI_STATE_WRITTEN );
}

xi_state_t
xi_mock_layer_tls_prev_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    return XI_PROCESS_PULL_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_tls_prev_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    /* Call close on the mockbroker chain */
    if ( NULL != xi_context_mockbroker )
    {
        xi_evtd_execute_in(
            xi_globals.evtd_instance,
            xi_make_handle(
                xi_itest_find_layer( xi_context_mockbroker, XI_LAYER_TYPE_MOCKBROKER_TOP )
                    ->layer_funcs->close,
                &xi_itest_find_layer( xi_context_mockbroker,
                                      XI_LAYER_TYPE_MOCKBROKER_TOP )
                     ->layer_connection,
                data, in_out_state ),
            1, NULL );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_layer_tls_prev_close_externally( void* context,
                                                    void* data,
                                                    xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_tls_prev_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_tls_prev_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_MOCK_BROKER_CONDITIONAL__CHECK_EXPECTED( in_out_state, LAYER_LEVEL );

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}

#ifdef __cplusplus
}
#endif
