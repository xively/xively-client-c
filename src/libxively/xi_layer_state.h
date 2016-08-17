/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_LAYER_STATE_H__
#define __XI_LAYER_STATE_H__

/**
 * @brief describes the current state that the layer is in
 */
typedef enum
{
    XI_LAYER_STATE_NONE = 0,
    XI_LAYER_STATE_CONNECTING,
    XI_LAYER_STATE_CONNECTED,
    XI_LAYER_STATE_CLOSING,
    XI_LAYER_STATE_CLOSED
} xi_layer_state_t;

#endif /* __XI_LAYER_STATE_H__ */
