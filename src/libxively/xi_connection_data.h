/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONNECTION_DATA_H__
#define __XI_CONNECTION_DATA_H__

#include <stdint.h>

#include "xi_config.h"
#include "xi_macros.h"
#include "xi_allocator.h"

#include "xi_event_dispatcher_api.h"
#include "xi_event_handle_typedef.h"

#include "xi_mqtt_message.h"
#include <xively_connection_data.h>

#ifdef __cplusplus
extern "C" {
#endif

extern xi_connection_data_t* xi_alloc_connection_data( const char* host,
                                                       uint16_t port,
                                                       const char* username,
                                                       const char* password,
                                                       uint16_t keepalive_timeout,
                                                       uint16_t connection_timeout,
                                                       xi_session_type_t session_type );

extern xi_connection_data_t*
xi_alloc_connection_data_lastwill( const char* host,
                                   uint16_t port,
                                   const char* username,
                                   const char* password,
                                   uint16_t keepalive_timeout,
                                   uint16_t connection_timeout,
                                   xi_session_type_t session_type,
                                   const char* will_topic,
                                   const char* will_apssword,
                                   xi_mqtt_qos_t will_qios,
                                   xi_mqtt_retain_t will_retain );

xi_state_t xi_connection_data_update( xi_connection_data_t* conn_data,
                                      const char* host,
                                      uint16_t port,
                                      const char* username,
                                      const char* password,
                                      uint16_t connection_timeout,
                                      uint16_t keepalive_timeout,
                                      xi_session_type_t session_type );

xi_state_t xi_connection_data_update_lastwill( xi_connection_data_t* conn_data,
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
                                               xi_mqtt_retain_t will_retain );

extern void xi_free_connection_data( xi_connection_data_t** data );

#ifdef __cplusplus
}
#endif

#endif /* __XI_CONNECTION_DATA_H__ */
