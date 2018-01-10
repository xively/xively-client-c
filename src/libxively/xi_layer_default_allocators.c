/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_layer.h"
#include "xi_layer_type.h"
#include "xi_allocator.h"
#include "xi_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_layer_t* default_layer_heap_alloc( xi_layer_type_t* type )
{
    xi_layer_t* ret  = ( xi_layer_t* )xi_alloc( sizeof( xi_layer_t ) );
    xi_state_t state = XI_STATE_OK;

    XI_CHECK_MEMORY( ret, state );

    memset( ret, 0, sizeof( xi_layer_t ) );

    ret->layer_funcs           = &type->layer_interface;
    ret->layer_type_id         = type->layer_type_id;
    ret->layer_connection.self = ret;

    return ret;

err_handling:
    return 0;
}

void default_layer_heap_free( xi_layer_type_t* type, xi_layer_t* layer )
{
    XI_UNUSED( type );
    xi_free( layer );
}

#ifdef __cplusplus
}
#endif
