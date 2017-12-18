/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xively_gateway.h>

xi_context_handle_t xi_create_gateway_context()
{
    return 0;
}

xi_state_t xi_delete_gateway_context( xi_context_handle_t gateway_context_handle )
{
    ( void )gateway_context_handle;

    return XI_STATE_OK;
}
