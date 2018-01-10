/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_TYPE_H__
#define __XI_LAYER_TYPE_H__

#include "xi_layer_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char xi_layer_type_id_t;

typedef struct xi_layer_type_s
{
    xi_layer_type_id_t layer_type_id;
    xi_layer_interface_t layer_interface;
} xi_layer_type_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_TYPE_H__ */
