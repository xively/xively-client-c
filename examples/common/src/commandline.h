/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * This module implements a command line argument parser
 */

#include <xively_mqtt.h>

/* Flags set by commandline arguments. */
extern int xi_quiet_flag;
extern xi_mqtt_retain_t xi_will_retain;
extern int xi_abnormalexit_flag;
extern int xi_help_flag;
extern xi_mqtt_qos_t xi_example_qos;

/* Parameters returned by the parser. These will be in a structure someday */
extern const char* xi_account_id;
extern const char* xi_username;
extern const char* xi_password;
extern const char* xi_publishtopic;
extern const char* xi_subscribetopic;
extern const char* xi_will_topic;
extern const char* xi_will_message;
extern int xi_memorylimit;
extern unsigned int xi_numberofpublishes;

/* Container for parameters the examples are called as functions from an embedded
 * application */
#ifdef XI_CROSS_TARGET
typedef struct xi_embedded_args_s
{
    int xi_quiet_flag;
    xi_mqtt_retain_t xi_will_retain;
    int xi_abnormalexit_flag;
    int xi_help_flag;
    const char* xi_account_id;
    const char* xi_username;
    const char* xi_password;
    const char* xi_publishtopic;
    const char* xi_subscribetopic;
    const char* xi_will_topic;
    const char* xi_will_message;
    const int xi_memorylimit;
    const unsigned int xi_numberofpublishes;
} xi_embedded_args_t;

#define XI_EMBEDDED_ARGS_INIT_BLOCK                                                      \
    {                                                                                    \
        0, XI_MQTT_RETAIN_FALSE, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 1    \
    }
#endif /* XI_CROSS_TARGET */

#ifndef XI_CROSS_TARGET
int xi_parse( int argc, char** argv, char* valid_options, const unsigned options_length );
#else
int xi_embedded_parse( xi_embedded_args_t* xi_embedded_args,
                       char* valid_options,
                       const unsigned options_length );
#endif /* XI_CROSS_TARGET */
