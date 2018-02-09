/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TYPES_H__
#define __XI_TYPES_H__

#include "xi_layer_chain.h"
#include "xi_connection_data.h"
#include "xi_vector.h"
#include "xi_event_dispatcher_api.h"
#include <xively_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * TYPES AND STRUCTURES
 * ----------------------------------------------------------------------- */

typedef enum xi_protocol_e {
    XI_MQTT,
} xi_protocol_t;

typedef enum xi_shutdown_state_e {
    XI_SHUTDOWN_UNINITIALISED,
    XI_SHUTDOWN_STARTED,
} xi_shutdown_state_t;

/**
 * @brief holds context sensitive data
 *
 *
 **/
typedef struct xi_context_data_s
{
/* atm this switch contains only the MQTT data
 * but it may happen that we will add more protocols
 * so the idea is to keep each protocol part here and make
 * it enable/disable via some flags or defines so for now
 * let's use that simplified form */
#if 1 /* MQTT context part */
    xi_vector_t* copy_of_handlers_for_topics;
    void* copy_of_q12_unacked_messages_queue; /* we have to use void* cause we don't want
                                              to
                                              create mqtt logic layer dependency */
    void ( *copy_of_q12_unacked_messages_queue_dtor_ptr )(
        void** ); /* this is dstr for unacked messages, just a very simplified
                            solution to the problem of not binding xively interface with
                            layers directly */
    uint16_t copy_of_last_msg_id; /* value of the msg_id for continious session */
#endif
    /* this is the common part */
    xi_time_event_handle_t connect_handler;
    /* vector or a list of timeouts */
    xi_vector_t* io_timeouts;
    xi_connection_data_t* connection_data;
    xi_evtd_instance_t* evtd_instance;
    xi_event_handle_t connection_callback;
    xi_shutdown_state_t shutdown_state;

    char** updateable_files;
    uint16_t updateable_files_count;
    xi_sft_url_handler_callback_t* sft_url_handler_callback;

    xi_context_handle_t main_context_handle;
} xi_context_data_t;

typedef struct xi_context_s
{
    xi_protocol_t protocol;
    xi_layer_chain_t layer_chain;
    xi_context_data_t context_data;
} xi_context_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_TYPES_H__ */
