/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_mock_layer_mqttlogic_prev.h"
#include "xi_layer_macros.h"
#include "xi_itest_helpers.h"
#include "xi_tuples.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t
xi_mock_layer_mqttlogic_prev_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )data;

    /* xi_debug_mqtt_message_dump( msg ); */

    check_expected( in_out_state );

    xi_state_t response_state = mock_type( xi_state_t );

    if ( XI_STATE_OK == response_state )
    {
        /* mimic successful message send */
        const uint16_t msg_id = xi_mqtt_get_message_id( msg );
        const xi_mqtt_type_t msg_type =
            ( xi_mqtt_type_t )msg->common.common_u.common_bits.type;

        check_expected( data );

        xi_mqtt_written_data_t* written_data =
            xi_alloc_make_tuple( xi_mqtt_written_data_t, msg_id, msg_type );

        xi_mqtt_message_free( &msg );

        return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, written_data, XI_STATE_WRITTEN );
    }
    else if ( XI_CONNECTION_RESET_BY_PEER_ERROR ==
              response_state ) /* this is just one error but we could create more
                                  different scenarios using that system */
    {
        xi_mqtt_message_free( &msg );

        return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER(
            context, NULL, XI_CONNECTION_RESET_BY_PEER_ERROR );
    }

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_mqttlogic_prev_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_mqttlogic_prev_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_layer_mqttlogic_prev_close_externally( void* context,
                                                          void* data,
                                                          xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();


    check_expected( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_mqttlogic_prev_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_mqttlogic_prev_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}

#ifdef __cplusplus
}
#endif
