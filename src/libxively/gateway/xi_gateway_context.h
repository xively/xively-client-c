/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_GATEWAY_CONTEXT_H__
#define __XI_GATEWAY_CONTEXT_H__

#include <xi_layer_type.h>
#include <xi_layer_chain.h>

typedef struct xi_gateway_context_s
{
    xi_layer_chain_t layer_chain;

} xi_gateway_context_t;

xi_state_t xi_create_gateway_context_with_custom_layers( xi_gateway_context_t** context,
                                                         xi_layer_type_t layer_config[],
                                                         xi_layer_type_id_t layer_chain[],
                                                         size_t layer_chain_size );


#endif /* __XI_GATEWAY_CONTEXT_H__ */
