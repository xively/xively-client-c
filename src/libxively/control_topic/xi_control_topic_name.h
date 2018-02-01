/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_TOPIC_NAME_H__
#define __XI_CONTROL_TOPIC_NAME_H__

#include "xi_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Xively control topic prefix used to create the topic name and identify it */
extern const char* const XI_TOPIC_DOMAIN;

/**
 * @brief xi_control_topic_get_topic_name
 *
 * Creates and returns the platform specific topic name
 *
 */
extern xi_state_t xi_control_topic_create_topic_name( const char* device_id,
                                                      char** subscribe_topic_name,
                                                      char** publish_topic_name );

#ifdef __cplusplus
}
#endif

#endif /* __XI_CONTROL_TOPIC_NAME_H__ */
