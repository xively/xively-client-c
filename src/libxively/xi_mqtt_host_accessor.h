/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

/* #ifndef __XI_MQTT_HOST_ACCESSOR_H__ */
/* #define __XI_MQTT_HOST_ACCESSOR_H__ */

#include <stddef.h>

#include "xi_macros.h"
#include "xi_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_static_host_desc_s
{
    char* name;
    uint16_t port;
} xi_static_host_desc_t;

#define XI_MQTT_HOST_ACCESSOR ((xi_static_host_desc_t)XI_MQTT_HOST)

#ifdef __cplusplus
}
#endif

/* #endif */ /* __XI_MQTT_HOST_ACCESSOR_H__ */
