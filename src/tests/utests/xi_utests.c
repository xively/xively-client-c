/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"

/* This block of code below must not be clang-formatted */
// clang-format off

/* Define a single bit for each test group. */
#define XI_TT_CORE                              ( 1 )
#define XI_TT_CONNECT                           ( XI_TT_CORE << 1 )
#define XI_TT_EVENT_DISPATCHER                  ( XI_TT_CONNECT << 1 )
#define XI_TT_EVENT_DISPATCHER_TIMED            ( XI_TT_EVENT_DISPATCHER << 1 )
#define XI_TT_DATASTRUCTURES                    ( XI_TT_EVENT_DISPATCHER_TIMED << 1 )
#define XI_TT_DATA_DESC                         ( XI_TT_DATASTRUCTURES << 1 )
#define XI_TT_BACKOFF                           ( XI_TT_DATA_DESC << 1 )
#define XI_TT_MQTT_CTORS_DTORS                  ( XI_TT_BACKOFF << 1 )
#define XI_TT_MQTT_PARSER                       ( XI_TT_MQTT_CTORS_DTORS << 1 )
#define XI_TT_MQTT_LOGIC_LAYER_SUBSCRIBE        ( XI_TT_MQTT_PARSER << 1 )
#define XI_TT_CONTROL_TOPIC                     ( XI_TT_MQTT_LOGIC_LAYER_SUBSCRIBE << 1 )
#define XI_TT_MQTT_SERIALIZER                   ( XI_TT_CONTROL_TOPIC << 1 )
#define XI_TT_MEMORY_LIMITER                    ( XI_TT_MQTT_SERIALIZER << 1 )
#define XI_TT_SENML                             ( XI_TT_MEMORY_LIMITER << 1 )
#define XI_TT_SENML_SERIALIZER                  ( XI_TT_SENML << 1 )
#define XI_TT_THREAD                            ( XI_TT_SENML_SERIALIZER << 1 )
#define XI_TT_THREAD_WORKERTHREAD               ( XI_TT_THREAD << 1 )
#define XI_TT_THREAD_THREADPOOL                 ( XI_TT_THREAD_WORKERTHREAD << 1 )
#define XI_TT_HELPERS                           ( XI_TT_THREAD_THREADPOOL << 1 )
#define XI_TT_MQTT_CODEC_LAYER_DATA             ( XI_TT_HELPERS << 1 )
#define XI_TT_PUBLISH                           ( XI_TT_MQTT_CODEC_LAYER_DATA << 1 )
#define XI_TT_FS                                ( XI_TT_PUBLISH << 1 )
#define XI_TT_RESOURCE_MANAGER                  ( XI_TT_FS << 1 )
#define XI_TT_IO_LAYER                          ( XI_TT_RESOURCE_MANAGER << 1 )
#define XI_TT_TIME_EVENT                        ( XI_TT_IO_LAYER << 1 )

// clang-format on

/* If no test set is defined, run all tests */
#ifndef XI_TT_TEST_SET
#define XI_TT_TEST_SET ( ~0 )
#endif

#define XI_TT_TESTCASE_PREDECLARATION( testgroupname )                                   \
    extern struct testcase_t testgroupname[]

// test groups
XI_TT_TESTCASE_PREDECLARATION( utest_core );
XI_TT_TESTCASE_PREDECLARATION( utest_connect );
XI_TT_TESTCASE_PREDECLARATION( utest_event_dispatcher );
XI_TT_TESTCASE_PREDECLARATION( utest_event_dispatcher_timed );
XI_TT_TESTCASE_PREDECLARATION( utest_datastructures );
XI_TT_TESTCASE_PREDECLARATION( utest_list );
XI_TT_TESTCASE_PREDECLARATION( utest_data_desc );
XI_TT_TESTCASE_PREDECLARATION( utest_backoff );
XI_TT_TESTCASE_PREDECLARATION( utest_mqtt_ctors_dtors );
XI_TT_TESTCASE_PREDECLARATION( utest_mqtt_parser );
XI_TT_TESTCASE_PREDECLARATION( utest_mqtt_logic_layer_subscribe );
XI_TT_TESTCASE_PREDECLARATION( utest_mqtt_codec_layer_data );
XI_TT_TESTCASE_PREDECLARATION( utest_publish );
XI_TT_TESTCASE_PREDECLARATION( utest_fwu_checksum );
XI_TT_TESTCASE_PREDECLARATION( utest_cbor_codec_ct_encode );
XI_TT_TESTCASE_PREDECLARATION( utest_cbor_codec_ct_decode );

#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
XI_TT_TESTCASE_PREDECLARATION( utest_control_message_sft );
XI_TT_TESTCASE_PREDECLARATION( utest_sft_logic );
XI_TT_TESTCASE_PREDECLARATION( utest_sft_logic_internal_methods );
#endif

#ifdef XI_CONTROL_TOPIC_ENABLED
XI_TT_TESTCASE_PREDECLARATION( utest_control_topic );
XI_TT_TESTCASE_PREDECLARATION( utest_protobuf_endianess );
XI_TT_TESTCASE_PREDECLARATION( utest_protobuf_engine );
#endif
XI_TT_TESTCASE_PREDECLARATION( utest_helpers );
XI_TT_TESTCASE_PREDECLARATION( utest_helper_functions );
XI_TT_TESTCASE_PREDECLARATION( utest_mqtt_serializer );
XI_TT_TESTCASE_PREDECLARATION( utest_handle );
XI_TT_TESTCASE_PREDECLARATION( utest_timed_task );

#ifdef XI_MEMORY_LIMITER_ENABLED
XI_TT_TESTCASE_PREDECLARATION( utest_memory_limiter );
#endif

#ifdef XI_SENML_ENABLED
XI_TT_TESTCASE_PREDECLARATION( utest_senml );
XI_TT_TESTCASE_PREDECLARATION( utest_senml_serialization );
#endif

XI_TT_TESTCASE_PREDECLARATION( utest_rng );

#ifdef XI_MODULE_THREAD_ENABLED
#include "xi_utest_thread.h"
#include "xi_utest_thread_workerthread.h"
#include "xi_utest_thread_threadpool.h"
#endif

XI_TT_TESTCASE_PREDECLARATION( utest_resource_manager );

XI_TT_TESTCASE_PREDECLARATION( utest_fs );

#ifdef XI_FS_MEMORY
XI_TT_TESTCASE_PREDECLARATION( utest_fs_memory );
#endif

#ifdef XI_FS_DUMMY
XI_TT_TESTCASE_PREDECLARATION( utest_fs_dummy );
#endif

#ifdef XI_FS_POSIX
XI_TT_TESTCASE_PREDECLARATION( utest_fs_posix );
#endif

XI_TT_TESTCASE_PREDECLARATION( utest_time_event );

#include "xi_test_utils.h"
#include "xi_lamp_communication.h"

/* Make an array of testgroups. This is mandatory. Unlike more
   heavy-duty testing frameworks, groups can't nest. */
struct testgroup_t groups[] = {
/* Every group has a 'prefix', and an array of tests.  That's it. */

#if ( XI_TT_TEST_SET & XI_TT_CORE )
    {"utest_core - ", utest_core},
#endif

#if ( XI_TT_TEST_SET & XI_TT_CONNECT )
    {"utest_connect - ", utest_connect},
#endif

#if ( XI_TT_TEST_SET & XI_TT_DATASTRUCTURES )
    {"utest_datastructures - ", utest_datastructures},
    {"utest_list - ", utest_list},
#endif

#if ( XI_TT_TEST_SET & XI_TT_EVENT_DISPATCHER )
    {"utest_event_dispatcher - ", utest_event_dispatcher},
#endif

#if ( XI_TT_TEST_SET & XI_TT_EVENT_DISPATCHER_TIMED )
    {"utest_event_dispatcher_timed - ", utest_event_dispatcher_timed},
#endif

#if ( XI_TT_TEST_SET & XI_TT_IO_FILE )
    {"utest_io_file - ", utest_io_file},
#endif

#if ( XI_TT_TEST_SET & XI_TT_DEVICE_CREDENTIALS )
    {"utest_device_credentials - ", utest_device_credentials},
#endif

#if ( XI_TT_TEST_SET & XI_TT_DATA_DESC )
    {"utest_data_desc - ", utest_data_desc},
#endif

#if ( XI_TT_TEST_SET & XI_TT_BACKOFF )
    {"utest_backoff - ", utest_backoff},
#endif

#if ( XI_TT_TEST_SET & XI_TT_MQTT_CTORS_DTORS )
    {"utest_mqtt_ctors_dtors - ", utest_mqtt_ctors_dtors},
#endif

#if ( XI_TT_TEST_SET & XI_TT_MQTT_PARSER )
    {"utest_mqtt_parser - ", utest_mqtt_parser},
#endif

#if ( XI_TT_TEST_SET & XI_TT_MQTT_LOGIC_LAYER_SUBSCRIBE )
    {"utest_mqtt_logic_layer_subscribe - ", utest_mqtt_logic_layer_subscribe},
#endif

#if ( XI_TT_TEST_SET & XI_TT_PUBLISH )
    {"utest_publish - ", utest_publish},
#endif

#ifdef XI_CONTROL_TOPIC_ENABLED
#if ( XI_TT_TEST_SET & XI_TT_CONTROL_TOPIC )
    {"utest_control_topic - ", utest_control_topic},
    {"utest_protobuf_endianess", utest_protobuf_endianess},
    {"utest_protobuf_engine", utest_protobuf_engine},
#endif
#endif

#if ( XI_TT_TEST_SET & XI_TT_MQTT_SERIALIZER )
    {"utest_mqtt_serializer - ", utest_mqtt_serializer},
#endif

#ifdef XI_MEMORY_LIMITER_ENABLED
#if ( XI_TT_TEST_SET & XI_TT_MEMORY_LIMITER )
    {"utest_memory_limiter - ", utest_memory_limiter},
#endif

#endif

#ifdef XI_SENML_ENABLED
#if ( XI_TT_TEST_SET & XI_TT_SENML )
    {"utest_senml - ", utest_senml},
#endif

#if ( XI_TT_TEST_SET & XI_TT_SENML_SERIALIZER )
    {"utest_senml_serialization - ", utest_senml_serialization},
#endif
#endif

#ifdef XI_MODULE_THREAD_ENABLED
#if ( XI_TT_TEST_SET & XI_TT_THREAD )
    {"utest_thread - ", utest_thread},
#endif

#if ( XI_TT_TEST_SET & XI_TT_THREAD_WORKERTHREAD )
    {"utest_thread_workerthread - ", utest_thread_workerthread},
#endif

#if ( XI_TT_TEST_SET & XI_TT_THREAD_THREADPOOL )
    {"utest_thread_threadpool - ", utest_thread_threadpool},
#endif
#endif

#if ( XI_TT_TEST_SET & XI_TT_HELPERS )
    {"utest_helpers - ", utest_helpers},
    {"utest_helper_functions - ", utest_helper_functions},
#endif

    {"utest_handle - ", utest_handle},

    {"utest_timed_task - ", utest_timed_task},

#if ( XI_TT_TEST_SET & XI_TT_MQTT_CODEC_LAYER_DATA )
    {"utest_mqtt_codec_layer_data - ", utest_mqtt_codec_layer_data},
#endif

#if ( XI_TT_TEST_SET & XI_TT_FS )
    {"utest_fs - ", utest_fs},
#ifdef XI_FS_DUMMY
    {"utest_fs_dummy - ", utest_fs_dummy},
#endif
#ifdef XI_FS_MEMORY
    {"utest_fs_memory - ", utest_fs_memory},
#endif
#ifdef XI_FS_POSIX
    {"utest_fs_posix - ", utest_fs_posix},
#endif
#endif

#if ( XI_TT_TEST_SET & XI_TT_RESOURCE_MANAGER )
    {"utest_resource_manager - ", utest_resource_manager},
#endif

#if ( XI_TT_TEST_SET & XI_TT_TIME_EVENT )
    {"utest_time_event - ", utest_time_event},
#endif

    {"utest_rng - ", utest_rng},
    {"utest_fwu_checksum - ", utest_fwu_checksum},
#ifdef XI_SECURE_FILE_TRANSFER_ENABLED
    {"utest_cbor_codec_ct_encode - ", utest_cbor_codec_ct_encode},
    {"utest_cbor_codec_ct_decode - ", utest_cbor_codec_ct_decode},
    {"utest_control_message_sft - ", utest_control_message_sft},
    {"utest_sft_logic - ", utest_sft_logic},
    {"utest_sft_logic_internal_methods - ", utest_sft_logic_internal_methods},
#endif

    END_OF_GROUPS};


#ifndef XI_EMBEDDED_TESTS
int main( int argc, char const* argv[] )
#else
int xi_utests_main( int argc, char const* argv[] )
#endif
{
    xi_test_init( argc, argv );

    // report test start
    xi_test_report_result( xi_test_load_level ? "xi_utest_id_l1" : "xi_utest_id_l0",
                           xi_test_load_level ? "xu1" : "xu", 1, 0 );

    // run all tests
    const int number_of_failures = tinytest_main( argc, argv, groups );
    // printf( "tinttest_main return value = %i\n", number_of_failures );

    // report test finish + result
    xi_test_report_result( xi_test_load_level ? "xi_utest_id_l1" : "xi_utest_id_l0",
                           xi_test_load_level ? "xu1" : "xu", 0, number_of_failures );

    return number_of_failures;
}
