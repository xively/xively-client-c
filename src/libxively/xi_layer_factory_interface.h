/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_FACTORY_INTERFACE_H__
#define __XI_LAYER_FACTORY_INTERFACE_H__

#include "xi_layer.h"
#include "xi_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_layer_factory_interface_s
{
    /* describes the type to create, mainly for sainity checks and nice error
     * reporting */
    xi_layer_type_id_t type_id_to_create;

    /* enables the placement initialization which is separeted from the
     * allocation/deallocation */
    xi_layer_t* ( *placement_create )( xi_layer_t* layer,
                                       void* user_data,
                                       struct xi_context_data_s* context_data );

    /* placement delete same as create but with oposite effect, may be used to
     * clean and/or deallocate memory */
    xi_layer_t* ( *placement_delete )( xi_layer_t* layer );

    /* strict layer allocation, may implement different strategies for allocation
     *  of the memory required for layer */
    xi_layer_t* ( *alloc )( xi_layer_type_t* type );

    /* strict deallocation of layer's memory */
    void ( *free )( xi_layer_type_t* type, xi_layer_t* layer );
} xi_layer_factory_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_FACTORY_INTERFACE_H__ */
