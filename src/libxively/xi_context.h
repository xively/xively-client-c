/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTEXT_H__
#define __XI_CONTEXT_H__

#include <xi_types.h>

xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                 xi_layer_type_t layer_config[],
                                                 size_t layer_chain_size );

xi_state_t xi_delete_context( xi_context_handle_t context_handle );

#endif /* __XI_CONTEXT_H__ */
