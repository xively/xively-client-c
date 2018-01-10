/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_message.h"

xi_mqtt_codec_layer_task_t* xi_mqtt_codec_layer_make_task( xi_mqtt_message_t* msg )
{
    xi_state_t state = XI_STATE_OK;

    assert( NULL != msg );

    XI_ALLOC( xi_mqtt_codec_layer_task_t, new_task, state );

    new_task->msg_id   = xi_mqtt_get_message_id( msg );
    new_task->msg_type = ( xi_mqtt_type_t )msg->common.common_u.common_bits.type;
    new_task->msg      = msg;

    return new_task;

err_handling:
    return NULL;
}

xi_mqtt_message_t* xi_mqtt_codec_layer_activate_task( xi_mqtt_codec_layer_task_t* task )
{
    assert( NULL != task );
    assert( NULL != task->msg );

    xi_mqtt_message_t* msg = task->msg;
    task->msg              = NULL;

    return msg;
}

void xi_mqtt_codec_layer_continue_task( xi_mqtt_codec_layer_task_t* task,
                                        xi_mqtt_message_t* msg )
{
    assert( NULL != task );
    assert( NULL != msg );
    assert( NULL == task->msg );

    task->msg = msg;
}

void xi_mqtt_codec_layer_free_task( xi_mqtt_codec_layer_task_t** task )
{
    if ( NULL == task || NULL == *task )
    {
        return;
    }

    xi_mqtt_message_free( &( *task )->msg );
    XI_SAFE_FREE( ( *task ) );
}
