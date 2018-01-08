/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_H__
#define __XI_LAYER_H__

#include <stdint.h>

#include "xi_config.h"
#include "xi_layer_connectivity.h"
#include "xi_layer_debug_info.h"
#include "xi_layer_interface.h"
#include "xi_layer_type.h"
#include "xi_layer_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_layer_s
{
    xi_layer_interface_t* layer_funcs;
    xi_layer_connectivity_t layer_connection;
    xi_layer_type_id_t layer_type_id;
    void* user_data;
    struct xi_context_data_s* context_data;
    xi_layer_state_t layer_state;
#if XI_DEBUG_EXTRA_INFO
    xi_layer_debug_info_t debug_info;
#endif
} xi_layer_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_H__ */
