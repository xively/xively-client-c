/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_CHAIN_H__
#define __XI_LAYER_CHAIN_H__

#include "xi_layer_api.h"
#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_layer_chain_s
{
    xi_layer_t* bottom;
    xi_layer_t* top;
} xi_layer_chain_t;

xi_layer_chain_t xi_layer_chain_connect( xi_layer_t* layers[], const size_t length );

xi_layer_chain_t xi_layer_chain_create( const xi_layer_type_id_t layers_ids[],
                                        const size_t length,
                                        struct xi_context_data_s* context_data,
                                        xi_layer_type_t layer_types[] );


xi_layer_chain_t
xi_layer_chain_create_with_user_data( const xi_layer_type_id_t layers_ids[],
                                      void* user_datas[],
                                      const size_t length,
                                      struct xi_context_data_s* context_data,
                                      xi_layer_type_t layer_types[] );

void xi_layer_chain_delete( xi_layer_chain_t* chain,
                            const size_t length,
                            xi_layer_type_t layer_types[] );


#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_CHAIN_H__ */
