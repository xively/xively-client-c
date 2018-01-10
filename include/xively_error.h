/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_ERROR_H__
#define __XIVELY_ERROR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    xi_state_t
 * @brief   Internal error codes.
 *
 * XI_STATE_OK represents success. Others represent errors.
 */
typedef enum {
    XI_STATE_OK = 0,                       /* 0 */
    XI_STATE_TIMEOUT,                      /* 1 */
    XI_STATE_WANT_READ,                    /* 2 */
    XI_STATE_WANT_WRITE,                   /* 3 */
    XI_STATE_WRITTEN,                      /* 4 */
    XI_STATE_FAILED_WRITING,               /* 5 */
    XI_BACKOFF_TERMINAL,                   /* 6 */
    XI_OUT_OF_MEMORY,                      /* 7 */
    XI_SOCKET_INITIALIZATION_ERROR,        /* 8 */
    XI_SOCKET_GETHOSTBYNAME_ERROR,         /* 9 */
    XI_SOCKET_GETSOCKOPT_ERROR,            /* 10 */
    XI_SOCKET_ERROR,                       /* 11 */
    XI_SOCKET_CONNECTION_ERROR,            /* 12 */
    XI_SOCKET_SHUTDOWN_ERROR,              /* 13 */
    XI_SOCKET_WRITE_ERROR,                 /* 14 */
    XI_SOCKET_READ_ERROR,                  /* 15 */
    XI_SOCKET_NO_ACTIVE_CONNECTION_ERROR,  /* 16 */
    XI_CONNECTION_RESET_BY_PEER_ERROR,     /* 17 */
    XI_FD_HANDLER_NOT_FOUND,               /* 18 */
    XI_TLS_INITALIZATION_ERROR,            /* 19 */
    XI_TLS_FAILED_LOADING_CERTIFICATE,     /* 20 */
    XI_TLS_CONNECT_ERROR,                  /* 21 */
    XI_TLS_WRITE_ERROR,                    /* 22 */
    XI_TLS_READ_ERROR,                     /* 23 */
    XI_MQTT_SERIALIZER_ERROR,              /* 24 */
    XI_MQTT_PARSER_ERROR,                  /* 25 */
    XI_MQTT_UNKNOWN_MESSAGE_ID,            /* 26 */
    XI_MQTT_LOGIC_UNKNOWN_TASK_ID,         /* 27 */
    XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE,     /* 28 */
    XI_MQTT_LOGIC_WRONG_MESSAGE_RECEIVED,  /* 29 */
    XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION, /* 30 */
    XI_MQTT_IDENTIFIER_REJECTED,           /* 31 */
    XI_MQTT_SERVER_UNAVAILIBLE,            /* 32 */
    XI_MQTT_BAD_USERNAME_OR_PASSWORD,      /* 33 */
    XI_MQTT_NOT_AUTHORIZED,                /* 34 */
    XI_MQTT_CONNECT_UNKNOWN_RETURN_CODE,   /* 35 */
    XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR,   /* 36 */
    XI_MQTT_PAYLOAD_SIZE_TOO_LARGE,        /* 37 */
    XI_MQTT_SUBSCRIPTION_FAILED,           /* 38 */
    XI_MQTT_SUBSCRIPTION_SUCCESSFULL,      /* 39 */
    XI_INTERNAL_ERROR,                     /* 40 */
    XI_NOT_INITIALIZED,                    /* 41 */
    XI_FAILED_INITIALIZATION,              /* 42 */
    XI_ALREADY_INITIALIZED,                /* 43 */
    XI_INVALID_PARAMETER,                  /* 44 */
    XI_UNSET_HANDLER_ERROR,                /* 45 */
    XI_NOT_IMPLEMENTED,                    /* 46 */
    XI_ELEMENT_NOT_FOUND,                  /* 47 */
    XI_SERIALIZATION_ERROR,                /* 48 */
    XI_TRUNCATION_WARNING,                 /* 49 */
    XI_BUFFER_OVERFLOW,                    /* 50 */
    XI_THREAD_ERROR,                       /* 51 */
    XI_NULL_CONTEXT,                       /* 52 */
    XI_NULL_WILL_TOPIC,                    /* 53 */
    XI_NULL_WILL_MESSAGE,                  /* 54 */
    XI_NO_MORE_RESOURCE_AVAILABLE,         /* 55 */
    XI_FS_RESOURCE_NOT_AVAILABLE,          /* 56 */
    XI_FS_ERROR,                           /* 57 */
    XI_NOT_SUPPORTED,                      /* 58 */
    XI_EVENT_PROCESS_STOPPED,              /* 59 */
    XI_STATE_RESEND,                       /* 60 */
    XI_NULL_HOST,                          /* 61 */
    XI_TLS_FAILED_CERT_ERROR,              /* 62 */
    XI_FS_OPEN_ERROR,                      /* 63 */
    XI_FS_OPEN_READ_ONLY,                  /* 64 */
    XI_FS_READ_ERROR,                      /* 65 */
    XI_FS_WRITE_ERROR,                     /* 66 */
    XI_FS_CLOSE_ERROR,                     /* 67 */
    XI_FS_REMOVE_ERROR,                    /* 68 */
    XI_ERROR_COUNT /* add above this line, and this sould always be last. */
} xi_state_t;

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_ERR_H__ */
