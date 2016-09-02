/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_LAYER_STACK_H__
#define __XI_LAYER_STACK_H__

#include "xi_layer_macros.h"
#include "xi_layer_factory_interface.h"
#include "xi_layers_ids.h"
#include "xi_io_layer_ids.h"
#include "xi_layer_default_functions.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -----------------------------------------------------------------------
 * LAYERS SETTINGS
 * ----------------------------------------------------------------------- */
#ifndef XI_DEBUG_NO_TLS
#define XI_DEFAULT_LAYER_CHAIN                                                           \
    XI_LAYER_TYPE_IO                                                                     \
    , XI_LAYER_TYPE_TLS, XI_LAYER_TYPE_MQTT_CODEC, XI_LAYER_TYPE_MQTT_LOGIC,             \
        XI_LAYER_TYPE_CONTROL_TOPIC
#else
#define XI_DEFAULT_LAYER_CHAIN                                                           \
    XI_LAYER_TYPE_IO                                                                     \
    , XI_LAYER_TYPE_MQTT_CODEC, XI_LAYER_TYPE_MQTT_LOGIC, XI_LAYER_TYPE_CONTROL_TOPIC
#endif

XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_DEFAULT, XI_DEFAULT_LAYER_CHAIN );

#if XI_IO_LAYER == XI_IO_POSIX
/* posix io layer */
#include "xi_io_posix_layer.h"
#ifndef XI_DEBUG_NO_TLS
#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"
#endif

/*-----------------------------------------------------------------------*/
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_control_topic_layer.h"

XI_DECLARE_LAYER_TYPES_BEGIN( xi_layer_types_g )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_posix_layer_push,
                    &xi_io_posix_layer_pull,
                    &xi_io_posix_layer_close,
                    &xi_io_posix_layer_close_externally,
                    &xi_io_posix_layer_init,
                    &xi_io_posix_layer_connect,
                    &xi_layer_default_post_connect )
#ifndef XI_DEBUG_NO_TLS
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_TLS,
                      &xi_tls_layer_push,
                      &xi_tls_layer_pull,
                      &xi_tls_layer_close,
                      &xi_tls_layer_close_externally,
                      &xi_tls_layer_init,
                      &xi_tls_layer_connect,
                      &xi_layer_default_post_connect )
#endif
      ,
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC,
                        &xi_mqtt_codec_layer_push,
                        &xi_mqtt_codec_layer_pull,
                        &xi_mqtt_codec_layer_close,
                        &xi_mqtt_codec_layer_close_externally,
                        &xi_mqtt_codec_layer_init,
                        &xi_mqtt_codec_layer_connect,
                        &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC,
                        &xi_mqtt_logic_layer_push,
                        &xi_mqtt_logic_layer_pull,
                        &xi_mqtt_logic_layer_close,
                        &xi_mqtt_logic_layer_close_externally,
                        &xi_mqtt_logic_layer_init,
                        &xi_mqtt_logic_layer_connect,
                        &xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#elif XI_IO_LAYER == XI_IO_BSP

#include "xi_io_net_layer.h"
#ifndef XI_DEBUG_NO_TLS
#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"
#endif

/*-----------------------------------------------------------------------*/
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_control_topic_layer.h"

XI_DECLARE_LAYER_TYPES_BEGIN( xi_layer_types_g )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_net_layer_push,
                    &xi_io_net_layer_pull,
                    &xi_io_net_layer_close,
                    &xi_io_net_layer_close_externally,
                    &xi_io_net_layer_init,
                    &xi_io_net_layer_connect,
                    &xi_layer_default_post_connect )
#ifndef XI_DEBUG_NO_TLS
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_TLS,
                      &xi_tls_layer_push,
                      &xi_tls_layer_pull,
                      &xi_tls_layer_close,
                      &xi_tls_layer_close_externally,
                      &xi_tls_layer_init,
                      &xi_tls_layer_connect,
                      &xi_layer_default_post_connect )
#endif
      ,
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC,
                        &xi_mqtt_codec_layer_push,
                        &xi_mqtt_codec_layer_pull,
                        &xi_mqtt_codec_layer_close,
                        &xi_mqtt_codec_layer_close_externally,
                        &xi_mqtt_codec_layer_init,
                        &xi_mqtt_codec_layer_connect,
                        &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC,
                        &xi_mqtt_logic_layer_push,
                        &xi_mqtt_logic_layer_pull,
                        &xi_mqtt_logic_layer_close,
                        &xi_mqtt_logic_layer_close_externally,
                        &xi_mqtt_logic_layer_init,
                        &xi_mqtt_logic_layer_connect,
                        &xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#elif XI_IO_LAYER == XI_IO_DUMMY
/* dummy io layer */
#include "xi_io_dummy_layer.h"

/*-----------------------------------------------------------------------*/
#ifndef XI_DEBUG_NO_TLS
#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"
#endif
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_control_topic_layer.h"

XI_DECLARE_LAYER_TYPES_BEGIN( xi_layer_types_g )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_dummy_layer_push,
                    &xi_io_dummy_layer_pull,
                    &xi_io_dummy_layer_close,
                    &xi_io_dummy_layer_close_externally,
                    &xi_io_dummy_layer_init,
                    &xi_io_dummy_layer_connect,
                    &xi_layer_default_post_connect )
#ifndef XI_DEBUG_NO_TLS
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_TLS,
                      &xi_tls_layer_push,
                      &xi_tls_layer_pull,
                      &xi_tls_layer_close,
                      &xi_tls_layer_close_externally,
                      &xi_tls_layer_init,
                      &xi_tls_layer_connect,
                      &xi_layer_default_post_connect )
#endif
      ,
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC,
                        &xi_mqtt_codec_layer_push,
                        &xi_mqtt_codec_layer_pull,
                        &xi_mqtt_codec_layer_close,
                        &xi_mqtt_codec_layer_close_externally,
                        &xi_mqtt_codec_layer_init,
                        &xi_mqtt_codec_layer_connect,
                        &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC,
                        &xi_mqtt_logic_layer_push,
                        &xi_mqtt_logic_layer_pull,
                        &xi_mqtt_logic_layer_close,
                        &xi_mqtt_logic_layer_close_externally,
                        &xi_mqtt_logic_layer_init,
                        &xi_mqtt_logic_layer_connect,
                        &xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#elif XI_IO_LAYER == XI_IO_MBED
/* mbed io layer */
#include "xi_io_mbed_layer.h"

/*-----------------------------------------------------------------------*/
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"

XI_DECLARE_LAYER_TYPES_BEGIN( xi_layer_types_g )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_mbed_layer_push,
                    &xi_io_mbed_layer_pull,
                    &xi_io_mbed_layer_close,
                    &xi_io_mbed_layer_close_externally,
                    &xi_io_mbed_layer_init,
                    &xi_io_mbed_layer_connect,
                    &xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC,
                      &xi_mqtt_codec_layer_push,
                      &xi_mqtt_codec_layer_pull,
                      &xi_mqtt_codec_layer_close,
                      &xi_mqtt_codec_layer_close_externally,
                      &xi_mqtt_codec_layer_init,
                      &xi_mqtt_codec_layer_connect,
                      &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC,
                        &xi_mqtt_logic_layer_push,
                        &xi_mqtt_logic_layer_pull,
                        &xi_mqtt_logic_layer_close,
                        &xi_mqtt_logic_layer_close_externally,
                        &xi_mqtt_logic_layer_init,
                        &xi_mqtt_logic_layer_connect,
                        &xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#elif XI_IO_LAYER == XI_IO_MICROCHIP
#include "xi_io_microchip_layer.h"

#ifndef XI_DEBUG_NO_TLS
#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"
#endif

#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_control_topic_layer.h"

XI_DECLARE_LAYER_TYPES_BEGIN( xi_layer_types_g )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_microchip_layer_push,
                    &xi_io_microchip_layer_pull,
                    &xi_io_microchip_layer_close,
                    &xi_io_microchip_layer_close_externally,
                    &xi_io_microchip_layer_init,
                    &xi_io_microchip_layer_connect,
                    &xi_layer_default_post_connect )
#ifndef XI_DEBUG_NO_TLS
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_TLS,
                      &xi_tls_layer_push,
                      &xi_tls_layer_pull,
                      &xi_tls_layer_close,
                      &xi_tls_layer_close_externally,
                      &xi_tls_layer_init,
                      &xi_tls_layer_connect,
                      &xi_layer_default_post_connect )
#endif
      ,
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_CODEC,
                        &xi_mqtt_codec_layer_push,
                        &xi_mqtt_codec_layer_pull,
                        &xi_mqtt_codec_layer_close,
                        &xi_mqtt_codec_layer_close_externally,
                        &xi_mqtt_codec_layer_init,
                        &xi_mqtt_codec_layer_connect,
                        &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MQTT_LOGIC,
                        &xi_mqtt_logic_layer_push,
                        &xi_mqtt_logic_layer_pull,
                        &xi_mqtt_logic_layer_close,
                        &xi_mqtt_logic_layer_close_externally,
                        &xi_mqtt_logic_layer_init,
                        &xi_mqtt_logic_layer_connect,
                        &xi_mqtt_logic_layer_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

#else

#error Unknown layer index specified!

#endif

#ifdef __cplusplus
}
#endif

#else

#error Cannot include more then once!

#endif /* __XI_LAYER_STACK_H__ */
