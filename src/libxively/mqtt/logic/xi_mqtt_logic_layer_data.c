/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_helpers.h"
#include "xi_macros.h"
#include "xi_mqtt_logic_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_mqtt_logic_task_t* xi_mqtt_logic_make_publish_task( const char* topic,
                                                       xi_data_desc_t* data,
                                                       const xi_mqtt_qos_t qos,
                                                       const xi_mqtt_retain_t retain,
                                                       xi_event_handle_t callback )
{
    /* PRECONDITIONS */
    assert( NULL != topic );
    assert( NULL != data );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_mqtt_logic_task_t, task, state );

    task->data.mqtt_settings.scenario = XI_MQTT_PUBLISH;
    task->data.mqtt_settings.qos      = qos;

    task->callback = callback;

    XI_ALLOC_AT( xi_mqtt_task_specific_data_t, task->data.data_u, state );

    task->data.data_u->publish.retain = retain;
    task->data.data_u->publish.topic  = xi_str_dup( topic );
    task->data.data_u->publish.data   = data;

    return task;

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }
    return NULL;
}

xi_mqtt_logic_task_t* xi_mqtt_logic_make_subscribe_task( char* topic,
                                                         const xi_mqtt_qos_t qos,
                                                         xi_event_handle_t handler )
{
    /* PRECONDITIONS */
    assert( NULL != topic );

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_mqtt_logic_task_t, task, state );

    task->data.mqtt_settings.scenario = XI_MQTT_SUBSCRIBE;
    task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_LEAST_ONCE;

    XI_ALLOC_AT( xi_mqtt_task_specific_data_t, task->data.data_u, state );

    task->data.data_u->subscribe.topic   = topic;
    task->data.data_u->subscribe.qos     = qos;
    task->data.data_u->subscribe.handler = handler;

    return task;

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }
    return NULL;
}

xi_mqtt_logic_task_t* xi_mqtt_logic_make_shutdown_task( void )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_mqtt_logic_task_t, task, state );

    task->data.mqtt_settings.scenario = XI_MQTT_SHUTDOWN;
    task->priority                    = XI_MQTT_LOGIC_TASK_IMMEDIATE;

    return task;

err_handling:
    if ( task )
    {
        xi_mqtt_logic_free_task( &task );
    }
    return NULL;
}

void xi_mqtt_task_spec_data_free_publish_data( xi_mqtt_task_specific_data_t** data )
{
    /* PRECONDITIONS */
    assert( NULL != data );
    assert( NULL != *data );

    xi_free_desc( &( *data )->publish.data );
    XI_SAFE_FREE( ( *data )->publish.topic );
    XI_SAFE_FREE( ( *data ) );
}

void xi_mqtt_task_spec_data_free_subscribe_data( xi_mqtt_task_specific_data_t** data )
{
    /* PRECONDITIONS */
    assert( NULL != data );
    assert( NULL != *data );

    XI_SAFE_FREE( ( *data )->subscribe.topic );
    XI_SAFE_FREE( ( *data ) );
}

void xi_mqtt_task_spec_data_free_subscribe_data_vec( union xi_vector_selector_u* data,
                                                     void* arg )
{
    XI_UNUSED( arg );

    xi_mqtt_task_spec_data_free_subscribe_data(
        ( xi_mqtt_task_specific_data_t** )&data->ptr_value );
}

xi_mqtt_logic_task_t* xi_mqtt_logic_free_task_data( xi_mqtt_logic_task_t* task )
{
    /* PRECONDITIONS */
    assert( NULL != task );
    assert( NULL != task->data.data_u );
    assert( NULL == task->timeout.ptr_to_position );

    switch ( task->data.mqtt_settings.scenario )
    {
        case XI_MQTT_PUBLISH:
            xi_mqtt_task_spec_data_free_publish_data( &task->data.data_u );
            break;
        case XI_MQTT_SUBSCRIBE:
            xi_mqtt_task_spec_data_free_subscribe_data( &task->data.data_u );
            break;
        case XI_MQTT_SHUTDOWN:
            break;
        default:
            xi_debug_format( "unhandled task type: %d",
                             task->data.mqtt_settings.scenario );
    }

    /* POSTCONDITIONS */
    assert( task->data.data_u == 0 );

    return task;
}

xi_mqtt_logic_task_t* xi_mqtt_logic_free_task( xi_mqtt_logic_task_t** task )
{
    if ( NULL != task && NULL != *task )
    {
        if ( NULL != ( *task )->data.data_u )
        {
            xi_mqtt_logic_free_task_data( *task );
        }

        XI_SAFE_FREE( *task );
    }

    return 0;
}

#ifdef __cplusplus
}
#endif
