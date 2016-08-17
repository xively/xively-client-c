/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_SL_LAYER_STATE_H__
#define __XI_IO_SL_LAYER_STATE_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_io_sl_layer_state_s
{
    int socket_fd;
    uint16_t layer_cs;
} xi_io_sl_layer_state_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_SL_LAYER_STATE_H__ */
