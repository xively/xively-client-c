/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_TLS_LAYER_STATE_H__
#define __XI_TLS_LAYER_STATE_H__

#include "xi_resource_manager.h"
#include "xi_common.h"

#include <cyassl/ssl.h>
#include <wolfssl/ssl.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_tls_layer_data_write_state_e {
    XI_TLS_LAYER_DATA_NONE = 0,
    XI_TLS_LAYER_DATA_WRITING,
    XI_TLS_LAYER_DATA_WRITTEN,
} xi_tls_layer_data_write_state_t;

typedef struct xi_tls_layer_state_s
{
    CYASSL_CTX* cyassl_ctx;
    CYASSL* cyassl_obj;
    xi_data_desc_t* raw_buffer;
    xi_data_desc_t* decoded_buffer;
    xi_data_desc_t* to_write_buffer;
    xi_event_handle_func_argc3_ptr tls_layer_logic_recv_handler;
    xi_event_handle_func_argc3_ptr tls_layer_logic_send_handler;
    xi_resource_manager_context_t* rm_context;
    xi_tls_layer_data_write_state_t tls_layer_write_state;
    uint16_t tls_lib_handler_sending_cs;
    uint16_t tls_layer_send_cs;
    uint16_t tls_layer_recv_cs;
    uint16_t tls_layer_conn_cs;
} xi_tls_layer_state_t;

#ifdef __cplusplus
}
#endif

#endif /* __XI_TLS_LAYER_STATE_H__ */
