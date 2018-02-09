/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_CHAIN_H__
#define __XI_LAYER_CHAIN_H__

#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_layer_api.h"
#include "xi_layer.h"
#include "xi_layer_type.h"
#include "xi_layer_factory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_layer_chain_s
{
    xi_layer_t* bottom;
    xi_layer_t* top;
} xi_layer_chain_t;

static inline xi_layer_chain_t
xi_layer_chain_connect( xi_layer_t* layers[], const size_t length )
{
    XI_UNUSED( layers );
    assert( length >= 2 && "you have to connect at least two layers to each other" );

    size_t i = 1;
    for ( ; i < length; ++i )
    {
        XI_LAYERS_CONNECT( layers[i - 1], layers[i] );
    }

    xi_layer_chain_t ret;

    ret.bottom = layers[0];
    ret.top    = layers[length - 1];

    return ret;
}

static inline xi_layer_chain_t
xi_layer_chain_create( const xi_layer_type_id_t layers_ids[],
                       const size_t length,
                       struct xi_context_data_s* context_data,
                       xi_layer_type_t layer_types[] )
{
    xi_layer_t* layers[length];
    memset( layers, 0, sizeof( layers ) );

    size_t i = 0;
    for ( ; i < length; ++i )
    {
        void* init_to_null = NULL;
        layers[i] =
            xi_layer_create( layers_ids[i], init_to_null, context_data, layer_types );
    }

    return xi_layer_chain_connect( layers, length );
}

static inline xi_layer_chain_t
xi_layer_chain_create_with_user_data( const xi_layer_type_id_t layers_ids[],
                                      void* user_datas[],
                                      const size_t length,
                                      struct xi_context_data_s* context_data,
                                      xi_layer_type_t layer_types[] )
{
    xi_layer_t* layers[length];
    memset( layers, 0, sizeof( layers ) );

    size_t i = 0;
    for ( ; i < length; ++i )
    {
        layers[i] =
            xi_layer_create( layers_ids[i], user_datas[i], context_data, layer_types );
    }

    return xi_layer_chain_connect( layers, length );
}

static inline void xi_layer_chain_delete( xi_layer_chain_t* chain,
                                          const size_t length,
                                          xi_layer_type_t layer_types[] )
{
    xi_layer_t* layers[length];
    memset( layers, 0, sizeof( layers ) );

    assert( chain != 0 && "layer chain must not be 0!" );
    assert( chain->bottom->layer_connection.next != 0 &&
            "layer chain must have at least 2 elements!" );

    xi_layer_t* prev = chain->bottom;
    xi_layer_t* tmp  = prev->layer_connection.next;

    unsigned char idx = 0;
    layers[idx]       = prev;

    while ( tmp )
    {
        XI_LAYERS_DISCONNECT( prev, tmp );

        prev = tmp;
        tmp  = tmp->layer_connection.next;

        layers[++idx] = prev;
    }

    size_t i = 0;
    for ( ; i < length; ++i )
    {
        xi_layer_delete( layers[i], layer_types );
    }
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_CHAIN_H__ */
