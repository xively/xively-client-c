/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_DEFAULT_ALLOCATORS_H__
#define __XI_LAYER_DEFAULT_ALLOCATORS_H__

#ifdef __cplusplus
extern "C" {
#endif

extern xi_layer_t* default_layer_heap_alloc( xi_layer_type_t* type );
extern void default_layer_heap_free( xi_layer_type_t* type, xi_layer_t* layer );

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_DEFAULT_ALLOCATORS_H__ */
