/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_USER_SUB_CALL_WRAPPER_H__
#define __XI_USER_SUB_CALL_WRAPPER_H__

#include "xively_types.h"
#include "xi_handle.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_globals.h"

xi_state_t xi_user_sub_call_wrapper( void* context,
                                     void* data,
                                     xi_state_t in_state,
                                     void* client_callback,
                                     void* user_data,
                                     void* task_data );

#endif /* __XI_USER_SUB_CALL_WRAPPER_H__ */
