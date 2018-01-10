/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_CONNECTION_DATA_H__
#define __XIVELY_CONNECTION_DATA_H__

#include <xively_mqtt.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @enum xi_connection_state_e
 * @brief Defines all possible states of the connection within the libxively
 */
typedef enum xi_connection_state_e {
    XI_CONNECTION_STATE_UNINITIALIZED =
        0,                          /**< the connection is not estblished and there
                                       is no pending connect operation in libxively */
    XI_CONNECTION_STATE_OPENING,    /**< the connect operation has been started */
    XI_CONNECTION_STATE_OPENED,     /**< connect operation has been finished
                                       successfully */
    XI_CONNECTION_STATE_CLOSING,    /**< disconnect operation has been started */
    XI_CONNECTION_STATE_CLOSED,     /**< connection is closed */
    XI_CONNECTION_STATE_OPEN_FAILED /**< there was an error during connect
                                       operation*/
} xi_connection_state_t;

/**
 * @enum xi_session_type_t
 * @brief MQTT session types
 */
typedef enum xi_session_type_e {
    XI_SESSION_CLEAN,   /**< MQTT clean session */
    XI_SESSION_CONTINUE /**< MQTT unclean session */
} xi_session_type_t;

/**
 * @struct  xi_connection_data_t
 * @brief   Connection parameters received by xi_connect's callback function e.g.
 *          to help reconnection with slightly different settings.
 */
typedef struct
{
    char* host;
    char* username;
    char* password;
    uint16_t port;
    uint16_t connection_timeout;
    uint16_t keepalive_timeout;
    xi_connection_state_t connection_state;
    xi_session_type_t session_type;
    char* will_topic;
    char* will_message;
    xi_mqtt_qos_t will_qos;
    xi_mqtt_retain_t will_retain;
} xi_connection_data_t;

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_CONNECTION_DATA_H__ */
