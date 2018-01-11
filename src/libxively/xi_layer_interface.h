/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_INTERFACE_H__
#define __XI_LAYER_INTERFACE_H__

#include <stdio.h>

#include "xi_layer_connectivity.h"
#include "xi_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is part of the standarized protocol of communication that
 * has been designed in order to provide the minimum restrictions,
 * in order to make the usage safe, with maximum capabilities. This
 * structure may be used to provide very simple communication between layers. */

typedef xi_state_t( xi_layer_func_t )( void* context, void* data, xi_state_t state );

/* The raw interface of the purly raw layer that combines both: simplicity and
 * functionality of the 'on demand processing' idea.
 *
 * The design idea is based on the assumption that the communication with the
 * server is bidirectional and the data is processed in packages. This interface design
 * is very generic and allows to implement different approaches as an extension of
 * the basic operations that the interface is capable of. In very basic scenario the
 * client wants to send some number of bytes to the server and then awaits for the
 * response.
 *
 * After receiving the response the response is passed by to the processing
 * layer through pull function which signals the processing layer that there is new data
 * to process.
 *
 * Term 'on demand processing' refers to the processing managed by the
 * connection. With the observation that all of the decision in the software depends on
 * the network capabilities it is clear that only the connection has ability to give the
 * signals about the processing to the rest of the components. The 'demand' takes it
 * source in the connection and that is the layer that has command over the another
 * layers.
 */
typedef struct xi_layer_interface_s
{
    /* called whenever the prev layer wants more data to process/send over some
     * kind of theconnection */
    xi_layer_func_t* push;

    /* called whenever there is data that is ready and it's source is the prev
     * layer */
    xi_layer_func_t* pull;

    /* whenever the processing chain supposed to be closed */
    xi_layer_func_t* close;

    /* whenver the processing chain is going to be closed it's source is in the
     * prev layer */
    xi_layer_func_t* close_externally;

    /* whenever we want to init the layer */
    xi_layer_func_t* init;

    /* whenever we want to connect the layer */
    xi_layer_func_t* connect;

    /* whenever we want to cause layers to do something in terms of post-connect */
    xi_layer_func_t* post_connect;
} xi_layer_interface_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_INTERFACE_H__ */
