/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_API_IMPLS_H__
#define __XIVELY_API_IMPLS_H__

#include <xively_error.h>
#include <xively_types.h>
#include <xi_data_desc.h>
#include <xi_thread_ids.h>

xi_state_t xi_publish_data_impl( xi_context_handle_t xih,
                                 const char* topic,
                                 xi_data_desc_t* data,
                                 const xi_mqtt_qos_t qos,
                                 const xi_mqtt_retain_t retain,
                                 xi_user_callback_t* callback,
                                 void* user_data,
                                 xi_thread_id_t callback_target_thread_id );

xi_state_t xi_subscribe_impl( xi_context_handle_t xih,
                              const char* topic,
                              const xi_mqtt_qos_t qos,
                              xi_user_subscription_callback_t* callback,
                              void* user_data,
                              xi_thread_id_t callback_target_thread_id );

#endif /* __XIVELY_API_IMPLS_H__ */
