/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONFIG_H__
#define __XI_CONFIG_H__

#ifdef __MBED__
#include "xi_config_mbed.h"
#endif

#ifndef XI_IO_BUFFER_SIZE
#define XI_IO_BUFFER_SIZE 32
#endif

#ifndef XI_BACKOFF_CHECK_TIME
#define XI_BACKOFF_CHECK_TIME 60
#endif

#ifndef XI_MQTT_MAX_PAYLOAD_SIZE
#define XI_MQTT_MAX_PAYLOAD_SIZE 1024 * 128
#endif

#ifndef XI_DEFAULT_IDLE_TIMEOUT
#define XI_DEFAULT_IDLE_TIMEOUT 1
#endif

#ifndef XI_MAX_IDLE_TIMEOUT
#define XI_MAX_IDLE_TIMEOUT 5
#endif

#ifndef XI_MQTT_PORT
#ifdef XI_DEBUG_NO_TLS
#define XI_MQTT_PORT 8883
#else
#define XI_MQTT_PORT 8883
#endif
#endif

#ifndef XI_MQTT_HOST
#define XI_MQTT_HOST                                                                     \
    {                                                                                    \
        "broker.xively.com", XI_MQTT_PORT                                                \
    }
#endif

#endif /* __XI_CONFIG_H__ */
