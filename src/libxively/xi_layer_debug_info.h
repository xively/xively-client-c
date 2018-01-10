/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_DEBUG_INFO_H__
#define __XI_LAYER_DEBUG_INFO_H__

#include "xi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if XI_DEBUG_EXTRA_INFO
typedef struct xi_layer_debug_info
{
    int debug_line_init;
    const char* debug_file_init;

    int debug_line_connect;
    const char* debug_file_connect;

    int debug_line_last_call;
    const char* debug_file_last_call;
} xi_layer_debug_info_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_DEBUG_INFO_H__ */
