/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_IO_NET_LAYER_STATE_H__
#define __XI_IO_NET_LAYER_STATE_H__

#include <stdint.h>
#include "xi_bsp_io_net.h"

typedef struct xi_io_net_layer_state_s
{
    xi_bsp_socket_t socket;

    uint16_t layer_connect_cs;
} xi_io_net_layer_state_t;

#endif /* __XI_IO_NET_LAYER_STATE_H__ */
