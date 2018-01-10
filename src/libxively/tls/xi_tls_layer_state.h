/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TLS_LAYER_STATE_H__
#define __XI_TLS_LAYER_STATE_H__

#include <xi_bsp_tls.h>
#include <xi_resource_manager.h>

typedef enum xi_tls_layer_data_write_state_e {
    XI_TLS_LAYER_DATA_NONE = 0,
    XI_TLS_LAYER_DATA_WRITING,
    XI_TLS_LAYER_DATA_WRITTEN,
} xi_tls_layer_data_write_state_t;

typedef struct xi_tls_layer_state_s
{
    xi_bsp_tls_context_t* tls_context;

    xi_data_desc_t* raw_buffer;
    xi_data_desc_t* decoded_buffer;
    xi_data_desc_t* to_write_buffer;

    xi_event_handle_func_argc3_ptr tls_layer_logic_recv_handler;
    xi_event_handle_func_argc3_ptr tls_layer_logic_send_handler;

    xi_resource_manager_context_t* rm_context;

    uint16_t tls_lib_handler_sending_cs;
    uint16_t tls_layer_conn_cs;
    uint16_t tls_layer_send_cs;
    uint16_t tls_layer_recv_cs;

    xi_tls_layer_data_write_state_t tls_layer_write_state;

} xi_tls_layer_state_t;

#endif /* __XI_TLS_LAYER_STATE_H__ */
