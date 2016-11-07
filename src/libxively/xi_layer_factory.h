/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_FACTORY_H__
#define __XI_LAYER_FACTORY_H__

#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* simpliest layer initialiser that does simply nothing, but returns the pointer
 * to the initialized layer */
xi_layer_t*
xi_layer_placement_create( xi_layer_t* layer,
                           void* user_data,
                           struct xi_context_data_s* context_data );

/* simpliest layer delete function that does simply nothing, but returns the
 * pointer to given layer which can free()'ed */
xi_layer_t* xi_layer_placement_delete( xi_layer_t* layer );

xi_layer_t*
xi_layer_alloc( xi_layer_type_id_t layer_type_id, xi_layer_type_t layer_types[] );

void xi_layer_free( xi_layer_t* layer, xi_layer_type_t layer_types[] );

xi_layer_t* xi_layer_place( xi_layer_t* layer,
                                          void* user_data,
                                          struct xi_context_data_s* context_data );

void xi_layer_destroy( xi_layer_t* layer );

xi_layer_t* xi_layer_create( const xi_layer_type_id_t layer_type_id,
                                           void* user_data,
                                           struct xi_context_data_s* context_data,
                                           xi_layer_type_t layer_types[] );

void xi_layer_delete( xi_layer_t* layer, xi_layer_type_t layer_types[] );

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_FACTORY_H__ */
