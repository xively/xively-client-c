/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "xi_io_dummy_layer.h"
#include "xi_io_dummy_layer_state.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_common.h"

xi_state_t xi_io_dummy_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

xi_state_t xi_io_dummy_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

xi_state_t xi_io_dummy_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

xi_state_t
xi_io_dummy_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_io_dummy_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}

xi_state_t xi_io_dummy_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    return XI_STATE_OK;
}
