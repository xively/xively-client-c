/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_CONNECTIVITY_H__
#define __XI_LAYER_CONNECTIVITY_H__

#ifdef __cplusplus
extern "C" {
#endif

struct xi_layer_s;

typedef struct xi_layer_connectivity_s
{
    struct xi_layer_s* self;
    struct xi_layer_s* next;
    struct xi_layer_s* prev;
} xi_layer_connectivity_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_CONNECTIVITY_H__ */
