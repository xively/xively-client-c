/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_MQTT_PARSER_H__
#define __XI_MQTT_PARSER_H__

#include <stdint.h>
#include <stddef.h>

#include "xi_common.h"
#include "xi_layer_interface.h"
#include "xi_mqtt_errors.h"
#include "xi_mqtt_message.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xi_mqtt_parser_rc_e {
    XI_MQTT_PARSER_RC_ERROR,
    XI_MQTT_PARSER_RC_CONTINUE,
    XI_MQTT_PARSER_RC_INCOMPLETE,
    XI_MQTT_PARSER_RC_DONE,
    XI_MQTT_PARSER_RC_WANT_MEMORY,
} xi_mqtt_parser_rc_t;

typedef struct xi_mqtt_parser_s
{
    xi_mqtt_error_t error;
    uint16_t cs;
    uint16_t read_cs;
    char buffer_pending;
    uint8_t* buffer;
    size_t buffer_length;
    size_t digit_bytes;
    size_t multiplier;
    size_t remaining_length;
    size_t str_length;
    size_t data_length;
} xi_mqtt_parser_t;

extern void xi_mqtt_parser_init( xi_mqtt_parser_t* parser );
extern void
xi_mqtt_parser_buffer( xi_mqtt_parser_t* parser, uint8_t* buffer, size_t buffer_length );

extern xi_state_t
xi_mqtt_parse_suback_response( xi_mqtt_suback_status_t* dst, const uint8_t resp );
extern xi_state_t xi_mqtt_parser_execute( xi_mqtt_parser_t* parser,
                                          xi_mqtt_message_t* message,
                                          xi_data_desc_t* );

#ifdef __cplusplus
}
#endif

#endif /* __XI_MQTT_PARSER_H__ */
