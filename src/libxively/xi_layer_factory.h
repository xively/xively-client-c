/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_FACTORY_H__
#define __XI_LAYER_FACTORY_H__

#include "xi_layer.h"
#include "xi_layer_interface.h"
#include "xi_layer_default_allocators.h"

#ifdef __cplusplus
extern "C" {
#endif

/* simpliest layer initialiser that does simply nothing, but returns the pointer
 * to the initialized layer */
inline static xi_layer_t*
xi_layer_placement_create( xi_layer_t* layer,
                           void* user_data,
                           struct xi_context_data_s* context_data )
{
    layer->user_data    = user_data;
    layer->context_data = context_data;

    return layer;
}

/* simpliest layer delete function that does simply nothing, but returns the
 * pointer to given layer which can free()'ed */
inline static xi_layer_t* xi_layer_placement_delete( xi_layer_t* layer )
{
    return layer;
}

static inline xi_layer_t*
xi_layer_alloc( xi_layer_type_id_t layer_type_id, xi_layer_type_t layer_types[] )
{
    xi_layer_type_t* layer_type = &layer_types[layer_type_id];
    return default_layer_heap_alloc( layer_type );
}

static inline void xi_layer_free( xi_layer_t* layer, xi_layer_type_t layer_types[] )
{
    xi_layer_type_t* layer_type = &layer_types[layer->layer_type_id];
    default_layer_heap_free( layer_type, layer );
}

static inline xi_layer_t* xi_layer_place( xi_layer_t* layer,
                                          void* user_data,
                                          struct xi_context_data_s* context_data )
{
    /* PRECONDITION */
    assert( layer != 0 );

    return xi_layer_placement_create( layer, user_data, context_data );
}

static inline void xi_layer_destroy( xi_layer_t* layer )
{
    /* PRECONDITION */
    assert( layer != 0 );

    xi_layer_placement_delete( layer );
}

static inline xi_layer_t* xi_layer_create( const xi_layer_type_id_t layer_type_id,
                                           void* user_data,
                                           struct xi_context_data_s* context_data,
                                           xi_layer_type_t layer_types[] )
{
    xi_state_t state = XI_STATE_OK;
    xi_layer_t* ret  = xi_layer_alloc( layer_type_id, layer_types );

    XI_CHECK_MEMORY( ret, state );

    return xi_layer_place( ret, user_data, context_data );

err_handling:
    return 0;
}

static inline void xi_layer_delete( xi_layer_t* layer, xi_layer_type_t layer_types[] )
{
    xi_layer_destroy( layer );
    xi_layer_free( layer, layer_types );
}

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_FACTORY_H__ */
