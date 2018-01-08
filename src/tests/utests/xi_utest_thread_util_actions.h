/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_UTEST_THREAD_UTIL_ACTIONS_H__
#define __XI_UTEST_THREAD_UTIL_ACTIONS_H__

#include <xi_err.h>
#include <xi_event_dispatcher_api.h>
#include <pthread.h>
#include "xi_critical_section_def.h"

struct xi_critical_section_s* xi_uteset_local_action_store_cs = NULL;

xi_state_t xi_utest_local_action_store_tid(
    xi_event_handle_arg1_t function_executed_communication_channel )
{
    if ( function_executed_communication_channel != NULL )
    {
        uint32_t* communication_channel =
            ( uint32_t* )function_executed_communication_channel;

        *communication_channel = ( uint32_t )pthread_self();
    }

    return XI_STATE_OK;
}

xi_state_t xi_utest_local_action_increase_by_one(
    xi_event_handle_arg1_t function_executed_communication_channel )
{
    if ( function_executed_communication_channel != NULL )
    {
        uint32_t* communication_channel =
            ( uint32_t* )function_executed_communication_channel;

        xi_lock_critical_section( xi_uteset_local_action_store_cs );

        *communication_channel = *communication_channel + 1;

        xi_unlock_critical_section( xi_uteset_local_action_store_cs );
    }

    return XI_STATE_OK;
}

xi_state_t xi_utest_local_action_decrease_by_11(
    xi_event_handle_arg1_t function_executed_communication_channel )
{
    if ( function_executed_communication_channel != NULL )
    {
        uint32_t* communication_channel =
            ( uint32_t* )function_executed_communication_channel;

        xi_lock_critical_section( xi_uteset_local_action_store_cs );

        *communication_channel = *communication_channel - 11;

        xi_unlock_critical_section( xi_uteset_local_action_store_cs );
    }

    return XI_STATE_OK;
}

#endif /* __XI_UTEST_THREAD_UTIL_ACTIONS_H__ */
