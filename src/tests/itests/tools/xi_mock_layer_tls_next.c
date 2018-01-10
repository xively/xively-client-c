/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_mock_layer_tls_next.h"
#include "xi_layer_macros.h"
#include "xi_itest_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t
xi_mock_layer_tls_next_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_tls_next_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_mock_layer_tls_next_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_mock_layer_tls_next_close_externally( void* context,
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
xi_mock_layer_tls_next_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t
xi_mock_layer_tls_next_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    check_expected( in_out_state );

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif
