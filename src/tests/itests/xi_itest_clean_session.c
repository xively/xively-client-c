/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_clean_session.h"

#include <time.h>

#include <xi_globals.h>
#include <xi_helpers.h>
#include <xi_allocator.h>
#include <xi_layer_macros.h>
#include <xi_bsp_time.h>

// dummy io layer
#include <xi_io_dummy_layer.h>

#include "xi_handle.h"
#include "xi_memory_checks.h"
#include "xi_layer_default_functions.h"

/*-----------------------------------------------------------------------*/
#ifndef XI_NO_TLS_LAYER
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

// some helpers
xi_mqtt_task_specific_data_t handlers_b[] = {
    {.subscribe = {"test",
                   {XI_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   XI_MQTT_QOS_AT_LEAST_ONCE}},
    {.subscribe = {"test2",
                   {XI_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   XI_MQTT_QOS_AT_LEAST_ONCE}},
    {.subscribe = {NULL,
                   {XI_EVENT_HANDLE_UNSET, .handlers.h0 = {0}, 0},
                   XI_MQTT_QOS_AT_MOST_ONCE}}};

#ifndef XI_NO_TLS_LAYER
#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"
#endif
#include "xi_control_topic_layer.h"

/*-----------------------------------------------------------------------*/
#include "xi_mqtt_codec_layer.h"
#include "xi_mqtt_codec_layer_data.h"
#include "xi_mqtt_logic_layer.h"
#include "xi_mqtt_logic_layer_data.h"

XI_DECLARE_LAYER_TYPES_BEGIN( itest_cyassl_context )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_IO,
                    &xi_io_dummy_layer_push,
                    &xi_io_dummy_layer_pull,
                    &xi_io_dummy_layer_close,
                    &xi_io_dummy_layer_close_externally,
                    &xi_io_dummy_layer_init,
                    &xi_io_dummy_layer_connect,
                    &xi_layer_default_post_connect )
#ifndef XI_NO_TLS_LAYER
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
                        &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_CONTROL_TOPIC,
                        &xi_control_topic_layer_push,
                        &xi_control_topic_layer_pull,
                        &xi_control_topic_layer_close,
                        &xi_control_topic_layer_close_externally,
                        &xi_control_topic_layer_init,
                        &xi_control_topic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        static void xi_inject_subscribe_handler( xi_vector_t** handler_vector,
                                                 xi_mqtt_task_specific_data_t* subs )
{
    int i = 0;

    if ( *handler_vector == NULL )
    {
        *handler_vector = xi_vector_create();
    }

    while ( subs[i].subscribe.topic != NULL )
    {
        size_t len                         = sizeof( subs[i] );
        xi_mqtt_task_specific_data_t* data = xi_alloc( len );

        memcpy( data, &subs[i], sizeof( subs[i] ) );
        data->subscribe.topic = xi_str_dup( subs[i].subscribe.topic );

        xi_vector_push( *handler_vector, XI_VEC_VALUE_PARAM( XI_VEC_VALUE_PTR( data ) ) );

        i++;
    }
    printf( "\n----> Done with the inject\n" );
}

static xi_context_t* xi_context              = NULL;
static xi_context_handle_t xi_context_handle = XI_INVALID_CONTEXT_HANDLE;

int xi_itest_clean_session_setup( void** state )
{
    XI_UNUSED( state );

    xi_memory_limiter_tearup();

    assert_int_equal( XI_STATE_OK, xi_initialize( "itest-acc-id", "itest-dev-id" ) );

    XI_CHECK_STATE( xi_create_context_with_custom_layers(
        &xi_context, itest_cyassl_context, XI_LAYER_CHAIN_DEFAULT,
        XI_LAYER_CHAIN_DEFAULTSIZE_SUFFIX ) );

    xi_find_handle_for_object( xi_globals.context_handles_vector, xi_context,
                               &xi_context_handle );

    return 0;

err_handling:
    fail();

    return 1;
}

int xi_itest_clean_session_teardown( void** state )
{
    XI_UNUSED( state );

    xi_delete_context_with_custom_layers(
        &xi_context, itest_cyassl_context,
        XI_LAYER_CHAIN_SCHEME_LENGTH( XI_LAYER_CHAIN_DEFAULT ) );
    xi_shutdown();

    return !xi_memory_limiter_teardown();
}

xi_state_t
xi_mockfunction__layerfunction_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

#if 0
    printf("in_out_state = %d\n", in_out_state);
    printf("this layer type id   = %d\n", XI_THIS_LAYER( context )->layer_type_id);
    printf("this layer user_data = %p\n", XI_THIS_LAYER( context )->user_data);
    printf("next layer type id   = %d\n", XI_NEXT_LAYER( context )->layer_type_id);
    printf("next layer user_data = %p\n", XI_NEXT_LAYER( context )->user_data);
#endif

    // check_expected_ptr(((xi_mqtt_logic_layer_data_t*)XI_PREV_LAYER(context)->user_data)->handlers_for_topics);
    check_expected( in_out_state );

    const xi_mqtt_logic_layer_data_t* mqtt_logic_layer_user_data =
        ( xi_mqtt_logic_layer_data_t* )XI_NEXT_LAYER( context )->user_data;
    check_expected( mqtt_logic_layer_user_data );

    if ( mqtt_logic_layer_user_data != NULL )
    {
        /* printf("_init next layer handlers_for_topics = %p, size = %d\n",
            mqtt_logic_layer_user_data->handlers_for_topics,
            mqtt_logic_layer_user_data->handlers_for_topics->elem_no); */

        const xi_vector_t* handlers_for_topics =
            mqtt_logic_layer_user_data->handlers_for_topics;

        check_expected( handlers_for_topics );
        check_expected( handlers_for_topics->elem_no );
    }

    // extension related to the next phase of processing
    enum PROC_TYPE proc_type = ( enum PROC_TYPE )mock();

    if ( proc_type == PROC_TYPE_DO )
    {
        // this is where we are going to pass the desired input to the next
        // layer
        void* next_data       = ( void* )mock();
        xi_state_t next_state = ( xi_state_t )mock();

        XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, next_data, next_state );
    }

    return XI_STATE_OK;
}

xi_state_t
xi_mockfunction__layerfunction_close( void* context, void* data, xi_state_t in_out_state )
{
    check_expected( in_out_state );

    const xi_mqtt_logic_layer_data_t* mqtt_logic_layer_user_data =
        ( xi_mqtt_logic_layer_data_t* )XI_NEXT_LAYER( context )->user_data;
    check_expected( mqtt_logic_layer_user_data );

    if ( mqtt_logic_layer_user_data != NULL )
    {
        /* printf("_close next layer handlers_for_topics = %p, size = %d\n",
            mqtt_logic_layer_user_data->handlers_for_topics,
            mqtt_logic_layer_user_data->handlers_for_topics->elem_no); */

        const xi_vector_t* handlers_for_topics =
            mqtt_logic_layer_user_data->handlers_for_topics;

        check_expected( handlers_for_topics );
        check_expected( handlers_for_topics->elem_no );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state );
}

static void xi_itest_clean_session_arrange( int inject_subscribe_handlers )
{
    xi_itest_inject_wraps( xi_context, XI_LAYER_TYPE_MQTT_CODEC, NULL, NULL,
                           xi_mockfunction__layerfunction_close, NULL,
                           xi_mockfunction__layerfunction_init, NULL );

    if ( inject_subscribe_handlers > 0 )
    {
        xi_inject_subscribe_handler(
            &xi_context->context_data.copy_of_handlers_for_topics, handlers_b );
    }
}

void clean_session_on_connection_state_changed( xi_context_handle_t in_context_handle,
                                                void* data,
                                                xi_state_t state )
{
    XI_UNUSED( in_context_handle );
    XI_UNUSED( data );
    XI_UNUSED( state );
}

static void xi_itest_clean_session_act( enum xi_session_type_e session_type )
{
    xi_connect( xi_context_handle, "test", "test", 10, 20, session_type,
                &clean_session_on_connection_state_changed );
    xi_evtd_step( xi_context->context_data.evtd_instance,
                  xi_bsp_time_getcurrenttime_seconds() + 1 );

    XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( &xi_context->layer_chain.bottom, NULL,
                                               XI_STATE_OK );

    xi_evtd_step( xi_context->context_data.evtd_instance,
                  xi_bsp_time_getcurrenttime_seconds() + 1 );

    return;
}

// test cases
void xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_clean_session(
    void** state )
{
    XI_UNUSED( state );

    xi_itest_clean_session_arrange( 0 );

    expect_value( xi_mockfunction__layerfunction_init, in_out_state, XI_STATE_OK );
    expect_not_value( xi_mockfunction__layerfunction_init, mqtt_logic_layer_user_data,
                      NULL );
    expect_not_value( xi_mockfunction__layerfunction_init, handlers_for_topics, NULL );
    expect_value( xi_mockfunction__layerfunction_init, handlers_for_topics->elem_no, 0 );
    will_return( xi_mockfunction__layerfunction_init, PROC_TYPE_DONT );

    xi_itest_clean_session_act( XI_SESSION_CLEAN );
}

void xi_itest_test_valid_flow__handlers_vector_should_be_empty__init_with_continue_session(
    void** state )
{
    XI_UNUSED( state );

    xi_itest_clean_session_arrange( 0 );

    expect_value( xi_mockfunction__layerfunction_init, in_out_state, XI_STATE_OK );
    expect_not_value( xi_mockfunction__layerfunction_init, mqtt_logic_layer_user_data,
                      NULL );
    expect_not_value( xi_mockfunction__layerfunction_init, handlers_for_topics, NULL );
    expect_value( xi_mockfunction__layerfunction_init, handlers_for_topics->elem_no, 0 );
    will_return( xi_mockfunction__layerfunction_init, PROC_TYPE_DONT );

    xi_itest_clean_session_act( XI_SESSION_CONTINUE );
}

void xi_itest_test_valid_flow__handlers_should_be_copied( void** state )
{
    XI_UNUSED( state );

    xi_itest_clean_session_arrange( 1 );

    expect_value( xi_mockfunction__layerfunction_init, in_out_state, XI_STATE_OK );
    expect_not_value( xi_mockfunction__layerfunction_init, mqtt_logic_layer_user_data,
                      NULL );
    expect_not_value( xi_mockfunction__layerfunction_init, handlers_for_topics, NULL );
    expect_value( xi_mockfunction__layerfunction_init, handlers_for_topics->elem_no, 2 );
    will_return( xi_mockfunction__layerfunction_init, PROC_TYPE_DONT );

    xi_itest_clean_session_act( XI_SESSION_CONTINUE );
}

void xi_itest_test_valid_flow__handlers_should_be_prereserved_across_initialization_deinitinialization(
    void** state )
{
    XI_UNUSED( state );

    xi_itest_clean_session_arrange( 1 );

    xi_vector_t* handlers_for_topics =
        xi_context->context_data.copy_of_handlers_for_topics;

    expect_value( xi_mockfunction__layerfunction_init, in_out_state, XI_STATE_OK );
    expect_not_value( xi_mockfunction__layerfunction_init, mqtt_logic_layer_user_data,
                      NULL );
    expect_value( xi_mockfunction__layerfunction_init, handlers_for_topics,
                  xi_context->context_data.copy_of_handlers_for_topics );
    expect_value( xi_mockfunction__layerfunction_init, handlers_for_topics->elem_no, 2 );

    expect_value( xi_mockfunction__layerfunction_close, in_out_state, XI_STATE_TIMEOUT );
    expect_not_value( xi_mockfunction__layerfunction_close, mqtt_logic_layer_user_data,
                      NULL );
    expect_value( xi_mockfunction__layerfunction_close, handlers_for_topics,
                  xi_context->context_data.copy_of_handlers_for_topics );
    expect_value( xi_mockfunction__layerfunction_close, handlers_for_topics->elem_no, 2 );

    will_return( xi_mockfunction__layerfunction_init, PROC_TYPE_DO );
    will_return( xi_mockfunction__layerfunction_init, NULL );
    will_return( xi_mockfunction__layerfunction_init, XI_STATE_TIMEOUT );

    xi_itest_clean_session_act( XI_SESSION_CONTINUE );

    assert_ptr_equal( xi_context->context_data.copy_of_handlers_for_topics,
                      handlers_for_topics );
}
