/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_TOPIC_LAYER_LAYER_DATA_H__
#define __XI_CONTROL_TOPIC_LAYER_LAYER_DATA_H__

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
#include <xi_sft_logic.h>
#else
#define xi_sft_context_t void
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    char* publish_topic_name;
    xi_sft_context_t* sft_context;
} xi_control_topic_layer_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_CONTROL_TOPIC_LAYER_LAYER_DATA_H__ */
