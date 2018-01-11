/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_mock_layer_mqttlogic_next.h"
#include "xi_layer_macros.h"
#include "xi_itest_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t
xi_mock_layer_mqttlogic_next_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    if ( NULL != data )
    {
        XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
    }

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_mqttlogic_next_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_mqttlogic_next_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t xi_mock_layer_mqttlogic_next_close_externally( void* context,
                                                          void* data,
                                                          xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_mqttlogic_next_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    check_expected( in_out_state );

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_mqttlogic_next_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    if ( XI_STATE_OK == in_out_state )
    {
        return XI_PROCESS_POST_CONNECT_ON_PREV_LAYER( context, NULL, in_out_state );
    }
    else
    {
        return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, NULL, in_out_state );
    }

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif
