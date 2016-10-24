/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_IO_MBED_LAYER_STATE_H__
#define __XI_IO_MBED_LAYER_STATE_H__

#include "TCPSocketConnection.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_io_mbed_layer_state_s
{
    TCPSocketConnection* socket_ptr;
} xi_io_mbed_layer_state_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_IO_MBED_LAYER_STATE_H__ */
