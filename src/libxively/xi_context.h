/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTEXT_H__
#define __XI_CONTEXT_H__

#include <xi_types.h>

xi_state_t
xi_create_context_with_custom_layers_and_evtd( xi_context_t** context,
                                               xi_layer_type_t layer_config[],
                                               xi_layer_type_id_t layer_chain[],
                                               size_t layer_chain_size,
                                               xi_evtd_instance_t* event_dispatcher,
                                               uint8_t handle_support );

xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                 xi_layer_type_t layer_config[],
                                                 size_t layer_chain_size );

xi_state_t xi_connect_with_lastwill_to_impl( xi_context_t* xi,
                                             const char* host,
                                             uint16_t port,
                                             const char* username,
                                             const char* password,
                                             uint16_t connection_timeout,
                                             uint16_t keepalive_timeout,
                                             xi_session_type_t session_type,
                                             const char* will_topic,
                                             const char* will_message,
                                             xi_mqtt_qos_t will_qos,
                                             xi_mqtt_retain_t will_retain,
                                             xi_user_callback_t* client_callback );

xi_state_t xi_shutdown_connection_impl( xi_context_t* xi );

#endif /* __XI_CONTEXT_H__ */
