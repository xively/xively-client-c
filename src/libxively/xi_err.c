/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_err.h"
#include "xi_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XI_OPT_NO_ERROR_STRINGS
const char* xi_error_string[XI_ERROR_COUNT] = {
    "XI_STATE_OK",                           /* XI_STATE_OK */
    "XI_STATE_TIMEOUT",                      /* XI_STATE_TIMEOUT */
    "XI_STATE_WANT_READ",                    /* XI_STATE_WANT_READ */
    "XI_STATE_WANT_WRITE",                   /* XI_STATE_WANT_WRITE */
    "XI_STATE_WRITTEN",                      /* XI_STATE_WRITTEN */
    "XI_STATE_FAILED_WRITING",               /* XI_STATE_FAILED_WRITING */
    "XI_BACKOFF_TERMINAL",                   /* XI_BACKOFF_TERMINAL */
    "XI_OUT_OF_MEMORY",                      /* XI_OUT_OF_MEMORY */
    "XI_SOCKET_INITIALIZATION_ERROR",        /* XI_SOCKET_INITIALIZATION_ERROR */
    "XI_SOCKET_GETHOSTBYNAME_ERROR",         /* XI_SOCKET_GETHOSTBYNAME_ERROR */
    "XI_SOCKET_GETSOCKOPT_ERROR",            /* XI_SOCKET_GETSOCKOPT_ERROR */
    "XI_SOCKET_ERROR",                       /* XI_SOCKET_ERROR */
    "XI_SOCKET_CONNECTION_ERROR",            /* XI_SOCKET_CONNECTION_ERROR */
    "XI_SOCKET_SHUTDOWN_ERROR",              /* XI_SOCKET_SHUTDOWN_ERROR */
    "XI_SOCKET_WRITE_ERROR",                 /* XI_SOCKET_WRITE_ERROR */
    "XI_SOCKET_READ_ERROR",                  /* XI_SOCKET_READ_ERROR */
    "XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR",  /* XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR */
    "XI_CONNECTION_RESET_BY_PEER_ERROR",     /* XI_CONNECTION_RESET_BY_PEER_ERROR */
    "XI_FD_HANDLER_NOT_FOUND",               /* XI_FD_HANDLER_NOT_FOUND */
    "XI_TLS_INITALIZATION_ERROR",            /* XI_TLS_INITALIZATION_ERROR */
    "XI_TLS_FAILED_LOADING_CERTIFICATE",     /* XI_TLS_FAILED_LOADING_CERTIFICATE */
    "XI_TLS_CONNECT_ERROR",                  /* XI_TLS_CONNECT_ERROR */
    "XI_TLS_WRITE_ERROR",                    /* XI_TLS_WRITE_ERROR */
    "XI_TLS_READ_ERROR",                     /* XI_TLS_READ_ERROR */
    "XI_MQTT_SERIALIZER_ERROR",              /* XI_MQTT_SERIALIZER_ERROR */
    "XI_MQTT_PARSER_ERROR",                  /* XI_MQTT_PARSER_ERROR */
    "XI_MQTT_UNKNOWN_MESSAGE_ID",            /* XI_MQTT_UNKNOWN_MESSAGE_ID */
    "XI_MQTT_LOGIC_UNKNOWN_TASK_ID",         /* XI_MQTT_LOGIC_UNKNOWN_TASK_ID */
    "XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE",     /* XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE */
    "XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED",  /* XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED */
    "XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION", /* XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION */
    "XI_MQTT_IDENTIFIER_REJECTED",           /* XI_MQTT_IDENTIFIER_REJECTED */
    "XI_MQTT_SERVER_UNAVAILIBLE",            /* XI_MQTT_SERVER_UNAVAILIBLE */
    "XI_MQTT_BAD_USERNAME_OR_PASSWORD",      /* XI_MQTT_BAD_USERNAME_OR_PASSWORD */
    "XI_MQTT_NOT_AUTHORIZED",                /* XI_MQTT_NOT_AUTHORIZED */
    "XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE",   /* XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE */
    "XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR",   /* XI_MQTT_MESSAGE_CLASS_UNKNOWN */
    "XI_MQTT_PAYLOAD_SIZE_TOO_LARGE",        /* XI_MQTT_PAYLOAD_SIZE_TOO_LARGE */
    "XI_MQTT_SUBSCRIBE_FAILED",              /* XI_MQTT_SUBSCRIBE_FAILED */
    "XI_MQTT_SUBSCRIBE_SUCCESSFULL",         /* XI_MQTT_SUBSCRIBE_SUCCESSFULL */
    "XI_INTERNAL_ERROR",                     /* XI_INTERNAL_ERROR */
    "XI_NOT_INITIALIZED",                    /* XI_NOT_INITIALIZED */
    "XI_FAILED_INITIALIZATION",              /* XI_FAILED_INITIALIZATION */
    "XI_ALREADY_INITIALIZED",                /* XI_ALREADY_INITIALIZED */
    "XI_INVALID_PARAMETER",                  /* XI_INVALID_PARAMETER */
    "XI_UNSET_HANDLER_ERROR",                /* XI_UNSET_HANDLER_ERROR */
    "XI_NOT_IMPLEMENTED",                    /* XI_NOT_IMPLEMENTED */
    "XI_ELEMENT_NOT_FOUND",                  /* XI_ELEMENT_NOT_FOUND */
    "XI_SERIALIZATION_ERROR",                /* XI_SERIALIZATION_ERROR */
    "XI_TRUNCATION_WARNING",                 /* XI_TRUNCATION_WARNING */
    "XI_BUFFER_OVERFLOW",                    /* XI_BUFFER_OVERFLOW */
    "XI_THREAD_ERROR",                       /* XI_THREAD_ERROR */
    "The passed or default context is NULL", /* XI_NULL_CONTEXT */
    "The last will topic cannot be NULL",    /* XI_NULL_WILL_TOPIC */
    "The last will message cannot be NULL",  /* XI_NULL_WILL_MESSAGE */
    "XI_NO_MORE_RESOURCE_AVAILABLE",         /* XI_NO_MORE_RESOURCE_AVAILABLE */
    "XI_FS_RESOURCE_NOT_AVAILABLE",          /* XI_FS_RESOURCE_NOT_AVAILABLE*/
    "XI_NOT_SUPPORTED",                      /* XI_NOT_SUPPORTED */
    "XI_EVENT_PROCESS_STOPPED",              /* XI_EVENT_PROCESS_STOPPED */
    "XI_FS_ERROR"                            /* XI_FS_ERROR */
};
#else
const char empty_sting[] = "";
#endif /* XI_OPT_NO_ERROR_STRINGS */

const char* xi_get_state_string( xi_state_t e )
{
#ifdef XI_OPT_NO_ERROR_STRINGS
    XI_UNUSED( e );
    return empty_sting;
#else
    return xi_error_string[XI_MIN( ( short )e, XI_ERROR_COUNT - 1 )];
#endif
}

#ifdef __cplusplus
}
#endif
