/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_backoff_status_api.h"
#include "xi_coroutine.h"
#include "xi_event_handle.h"
#include "xi_event_thread_dispatcher.h"
#include "xi_globals.h"
#include "xi_layer_default_functions.h"
#include "xi_layer_macros.h"
#include "xi_list.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_commands.h"
#include "xi_mqtt_logic_layer_data.h"
#include "xi_mqtt_logic_layer_handlers.h"
#include "xi_mqtt_logic_layer_helpers.h"
#include "xi_mqtt_message.h"
#include "xi_mqtt_parser.h"
#include "xi_mqtt_serialiser.h"
#include "xi_tuples.h"
#include "xively.h"

#ifdef __cplusplus
extern "C" {
#endif

static void xi_mqtt_logic_task_queue_shutdown( xi_mqtt_logic_task_t** task_queue );

static void xi_mqtt_logic_task_queue_shutdown_wrap( void** task_queue )
{
    assert( NULL != task_queue );
    assert( NULL != *task_queue );

    xi_mqtt_logic_task_queue_shutdown( ( xi_mqtt_logic_task_t** )task_queue );
}

uint8_t xi_mqtt_logic_layer_task_should_be_stored_predicate( void* context,
                                                             xi_mqtt_logic_task_t* task,
                                                             int i )
{
    XI_UNUSED( i );
    XI_UNUSED( context );

    assert( NULL != task );

    if ( task->session_state == XI_MQTT_LOGIC_TASK_SESSION_STORE )
    {
        return 1;
    }

    return 0;
}

static void xi_mqtt_logic_layer_task_make_context_null( xi_mqtt_logic_task_t* task )
{
    assert( NULL != task );
    task->logic.handlers.h4.a1 = NULL;
}

xi_state_t xi_mqtt_logic_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    xi_mqtt_logic_task_t* task = data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == 0 )
    {
        xi_debug_logger( "no layer_data" );
        goto err_handling;
    }

    /* update backoff penalty */
    if ( xi_backoff_classify_state( in_out_state ) == XI_BACKOFF_CLASS_TERMINAL )
    {
        XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, XI_BACKOFF_TERMINAL );
        goto err_handling;
    }

    /* when it's just a sent-confirmation */
    if ( in_out_state == XI_STATE_WRITTEN || in_out_state == XI_STATE_FAILED_WRITING )
    {
        /* means that we have to continue on execution
         * within one of the task, we can find the task
         * by checking the msg id, msg id is encoded
         * via simple casting */
        xi_mqtt_written_data_t* written_data = ( xi_mqtt_written_data_t* )data;

        uint16_t msg_id         = written_data->a1;
        xi_mqtt_type_t msg_type = written_data->a2;

        XI_SAFE_FREE_TUPLE( written_data );

        xi_mqtt_logic_task_t* task_to_be_called = NULL;

        if ( 0 == msg_id )
        {
            /* must have been a current_q0 task */
            task_to_be_called = layer_data->current_q0_task;
        }
        else
        {
            xi_mqtt_message_class_t msg_class =
                xi_mqtt_class_msg_type_sending( msg_type );

            xi_mqtt_logic_task_t* task_queue = NULL;

            switch ( msg_class )
            {
                case XI_MQTT_MESSAGE_CLASS_FROM_SERVER:
                    task_queue = layer_data->q12_recv_tasks_queue;
                    break;
                case XI_MQTT_MESSAGE_CLASS_TO_SERVER:
                    task_queue = layer_data->q12_tasks_queue;
                    break;
                case XI_MQTT_MESSAGE_CLASS_UNKNOWN:
                    in_out_state = XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR;
                    goto err_handling;
                default:
                    assert( 1 == 0 );
                    break;
            }

            /* it is one of the qos12 task, find a proper task */
            XI_LIST_FIND( xi_mqtt_logic_task_t, task_queue, CMP_TASK_MSG_ID, msg_id,
                          task_to_be_called );
        }

        /* restart layer keepalive - centralized for every successful send */
        if ( XI_STATE_WRITTEN == in_out_state &&
             NULL != layer_data->keepalive_event.ptr_to_position )
        {
            xi_state_t local_state = xi_evtd_restart(
                XI_CONTEXT_DATA( context )->evtd_instance, &layer_data->keepalive_event,
                XI_CONTEXT_DATA( context )->connection_data->keepalive_timeout );

            XI_CHECK_STATE( local_state );
        }

        if ( task_to_be_called != 0 )
        {
            task_to_be_called->logic.handlers.h4.a3 = in_out_state;
            return xi_evtd_execute_handle( &task_to_be_called->logic );
        }

        xi_debug_logger( "task to be called not be found during processing of "
                         "send-confirmation!" );
        return XI_MQTT_LOGIC_UNKNOWN_TASK_ID;
    }

    assert( task->data.mqtt_settings.scenario <= XI_MQTT_SHUTDOWN );
    assert( task->priority <= XI_MQTT_LOGIC_TASK_IMMEDIATE );

    /* assign the proper handler we are passing the msg pointer as 0 it can
     * be modified by injecting proper parameter later on */
    switch ( task->data.mqtt_settings.scenario )
    {
        case XI_MQTT_CONNECT:
        {
            task->logic =
                xi_make_handle( &do_mqtt_connect, context, task, XI_STATE_OK, 0 );
        }
        break;
        case XI_MQTT_PUBLISH:
        {
            switch ( task->data.mqtt_settings.qos )
            {
                case XI_MQTT_QOS_AT_MOST_ONCE:
                {
                    task->logic = xi_make_handle( &do_mqtt_publish_q0, context, task,
                                                  XI_STATE_OK, 0 );
                    break;
                }

                case XI_MQTT_QOS_AT_LEAST_ONCE:
                {
                    task->logic = xi_make_handle( &do_mqtt_publish_q1, context, task,
                                                  XI_STATE_OK, 0 );
                    break;
                }

                case XI_MQTT_QOS_EXACTLY_ONCE:
                {
                    xi_debug_logger( "Sending publish with qos = "
                                     "XI_MQTT_QOS_EXACTLY_ONCE not implemented "
                                     "yet!" );

                    if ( task->data.data_u != 0 )
                    {
                        xi_mqtt_logic_free_task_data( task );
                    }

                    break;
                }
            }
        }
        break;
        case XI_MQTT_SUBSCRIBE:
        {
            task->logic =
                xi_make_handle( &do_mqtt_subscribe, context, task, XI_STATE_OK, 0 );
        }
        break;
        case XI_MQTT_KEEPALIVE:
        {
            task->logic =
                xi_make_handle( &do_mqtt_keepalive_task, context, task, XI_STATE_OK, 0 );
        }
        break;
        case XI_MQTT_SHUTDOWN:
        {
            /* means that shutdown already in progress let's don't add another
             * one on top */
            if ( layer_data->current_q0_task != NULL &&
                 layer_data->current_q0_task->data.mqtt_settings.scenario ==
                     XI_MQTT_SHUTDOWN )
            {
                xi_mqtt_logic_free_task( &task );
                return XI_STATE_OK;
            }

            task->logic = xi_make_handle( &do_shutdown, context, task, XI_STATE_OK, 0 );
        }
        break;
        default:
            return XI_MQTT_LOGIC_WRONG_SCENARIO_TYPE;
    }

    run_task( context, task );

    return XI_STATE_OK;

err_handling:
    if ( in_out_state == XI_STATE_WRITTEN || in_out_state == XI_STATE_FAILED_WRITING )
    {
        XI_SAFE_FREE_TUPLE( data );
    }
    else
    {
        xi_mqtt_logic_free_task( &task );
    }

    return in_out_state;
}

xi_state_t xi_mqtt_logic_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );

    xi_mqtt_message_t* recvd_msg = ( xi_mqtt_message_t* )data;

    xi_mqtt_logic_layer_data_t* layer_data = XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || layer_data == 0 )
    {
        xi_debug_logger( "no layer_data" );
        goto err_handling;
    }

    /* update backoff penalty */
    if ( xi_update_backoff_penalty( in_out_state ) == XI_BACKOFF_CLASS_TERMINAL )
    {
        XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, XI_BACKOFF_TERMINAL );
        goto err_handling;
    }

    if ( in_out_state == XI_STATE_OK )
    {
        assert( recvd_msg != 0 ); /* sanity check */

        uint16_t msg_id = xi_mqtt_get_message_id( recvd_msg );

        /* first we have to make sure that it's not a publish
         * publish can be sent because of some request to the server
         * that was not started by our program so the state and tasks
         * were not initialised so we have to check it separatedly */

        xi_debug_format( "[m.id[%d] m.type[%d]] received msg", msg_id,
                         recvd_msg->common.common_u.common_bits.type );

        if ( recvd_msg->common.common_u.common_bits.type == XI_MQTT_TYPE_PUBLISH )
        {
            return on_publish_recieved( context, recvd_msg, XI_STATE_OK );
        }

        /* qos > 0 than the id of the message is going to help the handler
         * otherway we are going to use the current qos_0 task */
        if ( msg_id > 0 )
        {
            xi_mqtt_logic_task_t* task       = 0;
            xi_mqtt_logic_task_t* task_queue = 0;

            /** store the msg class */
            xi_mqtt_message_class_t msg_class = xi_mqtt_class_msg_type_receiving(
                ( xi_mqtt_type_t )recvd_msg->common.common_u.common_bits.type );

            /** pick proper msg queue */
            switch ( msg_class )
            {
                case XI_MQTT_MESSAGE_CLASS_FROM_SERVER:
                    task_queue = layer_data->q12_recv_tasks_queue;
                    break;
                case XI_MQTT_MESSAGE_CLASS_TO_SERVER:
                    task_queue = layer_data->q12_tasks_queue;
                    break;
                case XI_MQTT_MESSAGE_CLASS_UNKNOWN:
                default:
                    in_out_state = XI_MQTT_MESSAGE_CLASS_UNKNOWN_ERROR;
                    goto err_handling;
            }

            XI_LIST_FIND( xi_mqtt_logic_task_t, task_queue, CMP_TASK_MSG_ID,
                          msg_id, /* this can be optimized through
                                   * the structure optimization */
                          task );

            if ( task != 0 ) /* got the task let's call the proper handler */
            {
                /* assign the message as a fourth parameter */
                task->logic.handlers.h4.a3 = in_out_state;
                task->logic.handlers.h4.a4 = recvd_msg;
                return xi_evtd_execute_handle( &task->logic );
            }
            else
            {
                /* terrible error if we end up here it means that the message of
                 * received id is broken or has been made very serious mistake
                 * in logic of one of a handler function */
                xi_debug_format( "Error, can't find message with: id = %d", msg_id );

                xi_debug_mqtt_message_dump( recvd_msg );
                xi_mqtt_message_free( &recvd_msg );

                return XI_MQTT_UNKNOWN_MESSAGE_ID;
            }
        }
        else
        {
            /* sanity checks */
            assert( layer_data->current_q0_task != 0 );
            assert( layer_data->current_q0_task->logic.handle_type !=
                    XI_EVENT_HANDLE_UNSET );

            layer_data->current_q0_task->logic.handlers.h4.a4 = recvd_msg;
            layer_data->current_q0_task->logic.handlers.h4.a3 = in_out_state;

            return xi_evtd_execute_handle( &layer_data->current_q0_task->logic );
        }
    }

err_handling:
    xi_mqtt_message_free( &recvd_msg );
    return in_out_state;
}

xi_state_t xi_mqtt_logic_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_mqtt_logic_layer_data_t* layer_data = XI_THIS_LAYER( context )->user_data;

    /* only if the layer_data has been wiped out that means that we are connecting for
     * the first time */
    if ( layer_data == 0 )
    {
        XI_ALLOC_AT( xi_mqtt_logic_layer_data_t, XI_THIS_LAYER( context )->user_data,
                     in_out_state );

        /* initialisation of the unacked message dstr */
        XI_THIS_LAYER( context )
            ->context_data->copy_of_q12_unacked_messages_queue_dtor_ptr =
            &xi_mqtt_logic_task_queue_shutdown_wrap;

        layer_data = XI_THIS_LAYER( context )->user_data;
    }

    assert( layer_data != 0 );

    if ( XI_SESSION_CONTINUE ==
         XI_CONTEXT_DATA( context )->connection_data->session_type )
    {
        /* let's swap them with values so we are going to re-use the
         * handlers for topics from last session */
        layer_data->handlers_for_topics =
            XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics;
        XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics = NULL;

        /* same story goes with the qos1&2 unacked messages what we have to do is to
         * re-plug them into the queue and connect task will restart the tasks */
        layer_data->q12_tasks_queue =
            XI_THIS_LAYER( context )->context_data->copy_of_q12_unacked_messages_queue;
        XI_THIS_LAYER( context )->context_data->copy_of_q12_unacked_messages_queue = NULL;

        /* restoring the last_msg_id */
        layer_data->last_msg_id =
            XI_THIS_LAYER( context )->context_data->copy_of_last_msg_id;
        XI_THIS_LAYER( context )->context_data->copy_of_last_msg_id = 0;
    }
    else
    {
        if ( NULL != XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics )
        {
            xi_vector_for_each(
                XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics,
                &xi_mqtt_task_spec_data_free_subscribe_data_vec, NULL, 0 );

            XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics =
                xi_vector_destroy(
                    XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics );
        }

        /* clean the unsent QoS12 unacked tasks */
        xi_mqtt_logic_task_t* saved_unacked_qos_12_queue =
            ( xi_mqtt_logic_task_t* )XI_THIS_LAYER( context )
                ->context_data->copy_of_q12_unacked_messages_queue;
        xi_mqtt_logic_task_queue_shutdown( &saved_unacked_qos_12_queue );
        XI_THIS_LAYER( context )->context_data->copy_of_q12_unacked_messages_queue = NULL;
    }

    /* if there was no copy or this is the fresh (re)start */
    if ( NULL == layer_data->handlers_for_topics )
    {
        /* let's create fresh one */
        layer_data->handlers_for_topics = xi_vector_create();
        XI_CHECK_MEMORY( layer_data->handlers_for_topics, in_out_state );
    }

    XI_CONTEXT_DATA( context )->connection_data->connection_state =
        XI_CONNECTION_STATE_OPENING;

    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );

err_handling:
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    return in_out_state;
}

xi_state_t
xi_mqtt_logic_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    /* if the state is not ok, let's just pass it to the next layer */
    if ( in_out_state != XI_STATE_OK )
    {
        return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
    }

    XI_ALLOC( xi_mqtt_logic_task_t, task, in_out_state );

    task->data.mqtt_settings.scenario = XI_MQTT_CONNECT;
    task->data.mqtt_settings.qos      = XI_MQTT_QOS_AT_MOST_ONCE;
    task->priority                    = XI_MQTT_LOGIC_TASK_IMMEDIATE;

    /* push the task directly into the queue */
    return xi_mqtt_logic_layer_push( context, task, XI_STATE_OK );

err_handling:
    return in_out_state;
}

xi_state_t
xi_mqtt_logic_layer_post_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    const xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    if ( NULL == layer_data )
    {
        return in_out_state;
    }

    /* set new context and send timeout which will make the qos12 tasks to  continue they
     * work just where they were stopped */
    XI_LIST_FOREACH_WITH_ARG( xi_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                              set_new_context_and_call_resend, context );

    return xi_layer_default_post_connect( context, data, in_out_state );
}


xi_state_t xi_mqtt_logic_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    const xi_mqtt_logic_layer_data_t* layer_data =
        ( xi_mqtt_logic_layer_data_t* )XI_THIS_LAYER( context )->user_data;

    return ( NULL == layer_data )
               ? XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state )
               : XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

static void xi_mqtt_logic_task_queue_shutdown( xi_mqtt_logic_task_t** task_queue )
{
    /* PRE-CONDITION */
    assert( NULL != task_queue );

    /* each q12 task shall delete it's internal data on any try to continue */
    while ( *task_queue )
    {
        xi_mqtt_logic_task_t* tmp_task = NULL;

        XI_LIST_POP( xi_mqtt_logic_task_t, *task_queue, tmp_task );

        /* sanity check */
        assert( NULL != tmp_task );
        assert( NULL == tmp_task->timeout.ptr_to_position );

        xi_mqtt_logic_free_task( &tmp_task );
    }

    /* POST-CONDITION */
    assert( NULL == *task_queue );
}

xi_state_t
xi_mqtt_logic_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();
    XI_UNUSED( in_out_state );

    xi_context_data_t* context_data      = XI_CONTEXT_DATA( context );
    xi_evtd_instance_t* event_dispatcher = context_data->evtd_instance;

    xi_layer_state_t layer_state = XI_THIS_LAYER_STATE( context );

    if ( XI_STATE_OK != in_out_state && XI_LAYER_STATE_CONNECTING == layer_state )
    {
        /* error handling */
        return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
    }

    xi_mqtt_logic_layer_data_t* layer_data = XI_THIS_LAYER( context )->user_data;

    if ( NULL == layer_data )
    {
        return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
    }

    xi_update_backoff_penalty( in_out_state );

    /* disable timeouts of all tasks */
    XI_LIST_FOREACH_WITH_ARG( xi_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                              cancel_task_timeout, context );

    XI_LIST_FOREACH_WITH_ARG( xi_mqtt_logic_task_t, layer_data->q12_recv_tasks_queue,
                              cancel_task_timeout, context );

    /* if clean session not set check if we have anything to copy */
    if ( XI_SESSION_CONTINUE == context_data->connection_data->session_type )
    {
        /* this sets the copy of last_msg_id */
        XI_THIS_LAYER( context )->context_data->copy_of_last_msg_id =
            layer_data->last_msg_id;

        /* this will copy the handlers for topics */
        if ( layer_data->handlers_for_topics != NULL &&
             layer_data->handlers_for_topics->elem_no > 0 )
        {
            /* sanity check, let's make sure that it is empty */
            assert( XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics ==
                    NULL );

            XI_THIS_LAYER( context )->context_data->copy_of_handlers_for_topics =
                layer_data->handlers_for_topics;

            layer_data->handlers_for_topics = NULL;
        }

        /* here we are going to prereserve the qos12 send queue */
        /* we have to filter it in order to see which one has already been started */
        /* we will construct new list out of them */
        xi_mqtt_logic_task_t* unacked_list = NULL;

        XI_LIST_SPLIT_I( xi_mqtt_logic_task_t, layer_data->q12_tasks_queue,
                         xi_mqtt_logic_layer_task_should_be_stored_predicate, context,
                         unacked_list );

        XI_THIS_LAYER( context )->context_data->copy_of_q12_unacked_messages_queue =
            unacked_list;

        XI_LIST_FOREACH( xi_mqtt_logic_task_t, unacked_list,
                         xi_mqtt_logic_layer_task_make_context_null );
    }

    /* if the handlers for topics are left alone than it means
     * that it has to be freed */
    if ( layer_data->handlers_for_topics != NULL )
    {
        xi_vector_for_each( layer_data->handlers_for_topics,
                            &xi_mqtt_task_spec_data_free_subscribe_data_vec, NULL, 0 );
        xi_vector_destroy( layer_data->handlers_for_topics );
    }

    /* let's stop the current task */
    if ( layer_data->current_q0_task != 0 )
    {
        layer_data->current_q0_task->logic.handlers.h4.a3 = XI_STATE_TIMEOUT;
        xi_evtd_execute_handle( &layer_data->current_q0_task->logic );
        xi_mqtt_logic_free_task( &layer_data->current_q0_task );
    }

    /* unregister keepalive */
    if ( NULL != layer_data->keepalive_event.ptr_to_position )
    {
        xi_evtd_cancel( event_dispatcher, &layer_data->keepalive_event );
    }

    /* sanity check */
    assert( layer_data != 0 );

    /* save queues */
    xi_mqtt_logic_task_t* current_q0     = layer_data->current_q0_task;
    xi_mqtt_logic_task_t* q12_queue      = layer_data->q12_tasks_queue;
    xi_mqtt_logic_task_t* q12_recv_queue = layer_data->q12_recv_tasks_queue;
    xi_mqtt_logic_task_t* q0_queue       = layer_data->q0_tasks_queue;

    /* destroy user's data */
    XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );

    xi_mqtt_logic_task_queue_shutdown( &q12_queue );
    xi_mqtt_logic_task_queue_shutdown( &q12_recv_queue );

    /* special case for q0 task */
    if ( current_q0 )
    {
        abort_task( current_q0 );
        xi_mqtt_logic_free_task( &current_q0 );
    }

    /* as for q0, they haven't been started so they must be just deleted */
    while ( q0_queue )
    {
        xi_mqtt_logic_task_t* tmp_task = 0;

        XI_LIST_POP( xi_mqtt_logic_task_t, q0_queue, tmp_task );

        /* sanity check */
        assert( tmp_task != 0 );
        assert( tmp_task->timeout.ptr_to_position == 0 );

        xi_mqtt_logic_free_task( &tmp_task );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}

#ifdef __cplusplus
}
#endif
