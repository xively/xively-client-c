/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
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
    "XI_STATE_OK",                           /* 0 XI_STATE_OK */
    "XI_STATE_TIMEOUT",                      /* 1 XI_STATE_TIMEOUT */
    "XI_STATE_WANT_READ",                    /* 2 XI_STATE_WANT_READ */
    "XI_STATE_WANT_WRITE",                   /* 3 XI_STATE_WANT_WRITE */
    "XI_STATE_WRITTEN",                      /* 4 XI_STATE_WRITTEN */
    "XI_STATE_FAILED_WRITING",               /* 5 XI_STATE_FAILED_WRITING */
    "XI_BACKOFF_TERMINAL",                   /* 6 XI_BACKOFF_TERMINAL */
    "XI_OUT_OF_MEMORY",                      /* 7 XI_OUT_OF_MEMORY */
    "XI_SOCKET_INITIALIZATION_ERROR",        /* 8 XI_SOCKET_INITIALIZATION_ERROR */
    "XI_SOCKET_GETHOSTBYNAME_ERROR",         /* 9 XI_SOCKET_GETHOSTBYNAME_ERROR */
    "XI_SOCKET_GETSOCKOPT_ERROR",            /* 10 XI_SOCKET_GETSOCKOPT_ERROR */
    "XI_SOCKET_ERROR",                       /* 11 XI_SOCKET_ERROR */
    "XI_SOCKET_CONNECTION_ERROR",            /* 12 XI_SOCKET_CONNECTION_ERROR */
    "XI_SOCKET_SHUTDOWN_ERROR",              /* 13 XI_SOCKET_SHUTDOWN_ERROR */
    "XI_SOCKET_WRITE_ERROR",                 /* 14 XI_SOCKET_WRITE_ERROR */
    "XI_SOCKET_READ_ERROR",                  /* 15 XI_SOCKET_READ_ERROR */
    "XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR",  /* 16 XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR */
    "XI_CONNECTION_RESET_BY_PEER_ERROR",     /* 17 XI_CONNECTION_RESET_BY_PEER_ERROR */
    "XI_FD_HANDLER_NOT_FOUND",               /* 18 XI_FD_HANDLER_NOT_FOUND */
    "XI_TLS_INITALIZATION_ERROR",            /* 19 XI_TLS_INITALIZATION_ERROR */
    "XI_TLS_FAILED_LOADING_CERTIFICATE",     /* 20 XI_TLS_FAILED_LOADING_CERTIFICATE */
    "XI_TLS_CONNECT_ERROR",                  /* 21 XI_TLS_CONNECT_ERROR */
    "XI_TLS_WRITE_ERROR",                    /* 22 XI_TLS_WRITE_ERROR */
    "XI_TLS_READ_ERROR",                     /* 23 XI_TLS_READ_ERROR */
    "XI_MQTT_SERIALIZER_ERROR",              /* 24 XI_MQTT_SERIALIZER_ERROR */
    "XI_MQTT_PARSER_ERROR",                  /* 25 XI_MQTT_PARSER_ERROR */
    "XI_MQTT_UNKNOWN_MESSAGE_ID",            /* 26 XI_MQTT_UNKNOWN_MESSAGE_ID */
    "XI_MQTT_LOGIC_UNKNOWN_TASK_ID",         /* 27 XI_MQTT_LOGIC_UNKNOWN_TASK_ID */
    "XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE",     /* 28 XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE */
    "XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED",  /* 29 XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED */
    "XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION", /* 30 XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION
                                                */
    "XI_MQTT_IDENTIFIER_REJECTED",           /* 31 XI_MQTT_IDENTIFIER_REJECTED */
    "XI_MQTT_SERVER_UNAVAILIBLE",            /* 32 XI_MQTT_SERVER_UNAVAILIBLE */
    "XI_MQTT_BAD_USERNAME_OR_PASSWORD",      /* 33 XI_MQTT_BAD_USERNAME_OR_PASSWORD */
    "XI_MQTT_NOT_AUTHORIZED",                /* 34 XI_MQTT_NOT_AUTHORIZED */
    "XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE",   /* 35 XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE */
    "XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR",   /* 36 XI_MQTT_MESSAGE_CLASS_UNKNOWN */
    "XI_MQTT_PAYLOAD_SIZE_TOO_LARGE",        /* 37 XI_MQTT_PAYLOAD_SIZE_TOO_LARGE */
    "XI_MQTT_SUBSCRIBE_FAILED",              /* 38 XI_MQTT_SUBSCRIBE_FAILED */
    "XI_MQTT_SUBSCRIBE_SUCCESSFULL",         /* 39 XI_MQTT_SUBSCRIBE_SUCCESSFULL */
    "XI_INTERNAL_ERROR",                     /* 40 XI_INTERNAL_ERROR */
    "XI_NOT_INITIALIZED",                    /* 41 XI_NOT_INITIALIZED */
    "XI_FAILED_INITIALIZATION",              /* 42 XI_FAILED_INITIALIZATION */
    "XI_ALREADY_INITIALIZED",                /* 43 XI_ALREADY_INITIALIZED */
    "XI_INVALID_PARAMETER",                  /* 44 XI_INVALID_PARAMETER */
    "XI_UNSET_HANDLER_ERROR",                /* 45 XI_UNSET_HANDLER_ERROR */
    "XI_NOT_IMPLEMENTED",                    /* 46 XI_NOT_IMPLEMENTED */
    "XI_ELEMENT_NOT_FOUND",                  /* 47 XI_ELEMENT_NOT_FOUND */
    "XI_SERIALIZATION_ERROR",                /* 48 XI_SERIALIZATION_ERROR */
    "XI_TRUNCATION_WARNING",                 /* 49 XI_TRUNCATION_WARNING */
    "XI_BUFFER_OVERFLOW",                    /* 50 XI_BUFFER_OVERFLOW */
    "XI_THREAD_ERROR",                       /* 51 XI_THREAD_ERROR */
    "The passed or default context is NULL", /* 52 XI_NULL_CONTEXT */
    "The last will topic cannot be NULL",    /* 53 XI_NULL_WILL_TOPIC */
    "The last will message cannot be NULL",  /* 54 XI_NULL_WILL_MESSAGE */
    "XI_NO_MORE_RESOURCE_AVAILABLE",         /* 55 XI_NO_MORE_RESOURCE_AVAILABLE */
    "XI_FS_RESOURCE_NOT_AVAILABLE",          /* 56 XI_FS_RESOURCE_NOT_AVAILABLE*/
    "XI_FS_ERROR",                           /* 57 XI_FS_ERROR */
    "XI_NOT_SUPPORTED",                      /* 58 XI_NOT_SUPPORTED */
    "XI_EVENT_PROCESS_STOPPED",              /* 59 XI_EVENT_PROCESS_STOPPED */
    "XI_STATE_RESEND",                       /* 60 XI_STATE_RESEND */
    "XI_NULL_HOST",                          /* 61 XI_STATE_RESEND */
    "XI_TLS_FAILED_CERT_ERROR"               /* 62 XI_TLS_FAILED_CERT_ERROR */
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
