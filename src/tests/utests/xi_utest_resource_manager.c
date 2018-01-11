/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_resource_manager.h"
#include "xi_internals.h"
#include "xively.h"
#include "xi_fs_filenames.h"

#include "xi_helpers.h"
#include "xi_memory_checks.h"
#include "xi_globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

#include "xi_fs_header.h"
#include "xi_macros.h"

typedef enum xi_utest_rm_fs_function_type_e {
    XI_UTEST_RM_FS_FUNCTION_STAT = 0,
    XI_UTEST_RM_FS_FUNCTION_OPEN,
    XI_UTEST_RM_FS_FUNCTION_READ,
    XI_UTEST_RM_FS_FUNCTION_WRITE,
    XI_UTEST_RM_FS_FUNCTION_CLOSE,
    XI_UTEST_RM_FS_FUNCTION_REMOVE,
    XI_UTEST_RM_FS_FUNCITON_COUNT
} xi_utest_rm_fs_function_type_e;

/* test buffer, contains 20 characters, the test is using 4 chunks each 5 characters */
static const uint8_t xi_utest_resource_manager_test_buffer[] = {
    'c', 'h', 'u', 'n', 'k', 't', 'e', 's', 't', '1',
    't', 'e', 's', 't', '2', 't', 'e', 's', 't', '3'};

static uint8_t xi_utest_resource_manager_fs_counters[XI_UTEST_RM_FS_FUNCITON_COUNT] = {
    0, 0, 0, 0, 0, 0};

static uint8_t xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCITON_COUNT] = {0};

static const uint8_t
    xi_utest_resource_manager_fs_limit_test_cases[][XI_UTEST_RM_FS_FUNCITON_COUNT] = {
        {1 /* STAT */, 1 /* OPEN */, 1 /* READ */, 1 /* WRITE */, 1 /* CLOSE */,
         1 /* REMOVE */},
        {4 /* STAT */, 4 /* OPEN */, 4 /* READ */, 4 /* WRITE */, 4 /* CLOSE */,
         4 /* REMOVE */}};

void xi_utest_resource_manager_fs_set_limit_test_case( const size_t no )
{
    /* PRE-CONDITION */
    assert( no < XI_ARRAYSIZE( xi_utest_resource_manager_fs_limit_test_cases ) );
    memcpy( ( void* )xi_utest_resource_manager_fs_limits,
            ( const void* )xi_utest_resource_manager_fs_limit_test_cases[no],
            sizeof( xi_utest_resource_manager_fs_limits ) );
}

void xi_utest_resource_manager_fs_counter_reset()
{
    memset( &xi_utest_resource_manager_fs_counters, 0,
            sizeof( xi_utest_resource_manager_fs_counters ) );
}

#define XI_UTEST_RM_FS_FUNCTION_BODY( function_type, ret_state, common, chunk_action,    \
                                      limit_reached_action )                             \
    common;                                                                              \
                                                                                         \
    xi_utest_resource_manager_fs_counters[function_type] += 1;                           \
                                                                                         \
    if ( xi_utest_resource_manager_fs_counters[function_type] ==                         \
         xi_utest_resource_manager_fs_limits[function_type] )                            \
    {                                                                                    \
        limit_reached_action;                                                            \
        return XI_STATE_OK;                                                              \
    }                                                                                    \
                                                                                         \
                                                                                         \
    chunk_action;                                                                        \
                                                                                         \
    return ret_state

/* happy versions of file system operations */

xi_state_t xi_utest_resource_manager_fs_stat( const void* context,
                                              const xi_fs_resource_type_t resource_type,
                                              const char* const resource_name,
                                              xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( resource_stat );

    XI_UTEST_RM_FS_FUNCTION_BODY( XI_UTEST_RM_FS_FUNCTION_STAT, XI_STATE_WANT_READ, {},
                                  {}, {
                                      resource_stat->resource_size =
                                          sizeof( xi_utest_resource_manager_test_buffer );
                                  } );
}

xi_state_t xi_utest_resource_manager_fs_open( const void* context,
                                              const xi_fs_resource_type_t resource_type,
                                              const char* const resource_name,
                                              const xi_fs_open_flags_t open_flags,
                                              xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( open_flags );
    XI_UNUSED( resource_handle );

    XI_UTEST_RM_FS_FUNCTION_BODY( XI_UTEST_RM_FS_FUNCTION_OPEN, XI_STATE_WANT_READ, {},
                                  {}, { *resource_handle = 0; } );
}

xi_state_t
xi_utest_resource_manager_fs_read( const void* context,
                                   const xi_fs_resource_handle_t resource_handle,
                                   const size_t offset,
                                   const uint8_t** buffer,
                                   size_t* const buffer_size )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( offset );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );

    XI_UTEST_RM_FS_FUNCTION_BODY(
        XI_UTEST_RM_FS_FUNCTION_READ, XI_STATE_WANT_READ,
        const uint8_t limit_value =
            xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCTION_READ];

        const size_t data_size    = sizeof( xi_utest_resource_manager_test_buffer );
        const size_t chunk_size   = data_size / limit_value;
        const size_t chunk_offset = offset;
        ,
        {

            *buffer = xi_utest_resource_manager_test_buffer + ( intptr_t )chunk_offset;
            *buffer_size = chunk_size;
        },
        {
            *buffer = xi_utest_resource_manager_test_buffer + ( intptr_t )chunk_offset;
            *buffer_size = chunk_size;
        } );
}

xi_state_t
xi_utest_resource_manager_fs_write( const void* context,
                                    const xi_fs_resource_handle_t resource_handle,
                                    const uint8_t* const buffer,
                                    const size_t buffer_size,
                                    const size_t offset,
                                    size_t* const bytes_written )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );
    XI_UNUSED( offset );
    XI_UNUSED( bytes_written );

    return XI_FS_ERROR;
}

xi_state_t
xi_utest_resource_manager_fs_close( const void* context,
                                    const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );

    XI_UTEST_RM_FS_FUNCTION_BODY( XI_UTEST_RM_FS_FUNCTION_CLOSE, XI_STATE_WANT_READ, {},
                                  {}, {} );
}

xi_state_t xi_utest_resource_manager_fs_remove( const void* context,
                                                const xi_fs_resource_type_t resource_type,
                                                const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

/* Broken version of filesystem operations */

xi_state_t
xi_utest_resource_manager_fs_failing_stat( const void* context,
                                           const xi_fs_resource_type_t resource_type,
                                           const char* const resource_name,
                                           xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( resource_stat );

    return XI_FS_ERROR;
}

xi_state_t
xi_utest_resource_manager_fs_failing_open( const void* context,
                                           const xi_fs_resource_type_t resource_type,
                                           const char* const resource_name,
                                           const xi_fs_open_flags_t open_flags,
                                           xi_fs_resource_handle_t* resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( open_flags );
    XI_UNUSED( resource_handle );

    return XI_FS_ERROR;
}

xi_state_t
xi_utest_resource_manager_fs_failing_read( const void* context,
                                           const xi_fs_resource_handle_t resource_handle,
                                           const size_t offset,
                                           const uint8_t** buffer,
                                           size_t* const buffer_size )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( offset );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );

    return XI_FS_ERROR;
}

xi_state_t
xi_utest_resource_manager_fs_failing_write( const void* context,
                                            const xi_fs_resource_handle_t resource_handle,
                                            const uint8_t* const buffer,
                                            const size_t buffer_size,
                                            const size_t offset,
                                            size_t* const bytes_written )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );
    XI_UNUSED( buffer );
    XI_UNUSED( buffer_size );
    XI_UNUSED( offset );
    XI_UNUSED( bytes_written );

    return XI_FS_ERROR;
}


xi_state_t xi_utest_resource_manager_fs_failing_close(
    const void* context, const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );

    return XI_FS_ERROR;
}

xi_state_t
xi_utest_resource_manager_fs_failing_remove( const void* context,
                                             const xi_fs_resource_type_t resource_type,
                                             const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

/* local filesystem functions that we are going to inject in order to test
    the resource manager */
static xi_fs_functions_t xi_resource_manager_fs_functions = {
    sizeof( xi_fs_functions_t ),         &xi_utest_resource_manager_fs_stat,
    &xi_utest_resource_manager_fs_open,  &xi_utest_resource_manager_fs_read,
    &xi_utest_resource_manager_fs_write, &xi_utest_resource_manager_fs_close,
    &xi_utest_resource_manager_fs_remove};

/* local filesystem functions that fails we are going to inject in order to test
    the resource manager */
static xi_fs_functions_t xi_resource_manager_fs_failing_functions = {
    sizeof( xi_fs_functions_t ),
    &xi_utest_resource_manager_fs_failing_stat,
    &xi_utest_resource_manager_fs_failing_open,
    &xi_utest_resource_manager_fs_failing_read,
    &xi_utest_resource_manager_fs_failing_write,
    &xi_utest_resource_manager_fs_failing_close,
    &xi_utest_resource_manager_fs_failing_remove};

/* simulates the layer callback, it increases the int value passed through void* data
 * parameter */
xi_state_t
xi_utest_resource_manager_open_callback( void* data, void* data1, xi_state_t state )
{
    XI_UNUSED( data1 );
    XI_UNUSED( state );

    int* p = ( int* )data;

    if ( state == XI_STATE_OK )
    {
        *p += 1;
    }
    else
    {
        *p += 1000;
    }

    return XI_STATE_OK;
}

/* simulates the layer callback, it increases the int value passed through void* data
 * parameter */
xi_state_t
xi_utest_resource_manager_read_callback( void* data, void* data1, xi_state_t state )
{
    XI_UNUSED( data1 );
    XI_UNUSED( state );

    int* p = ( int* )data;
    if ( state == XI_STATE_OK )
    {
        *p += 1;
    }
    else
    {
        *p += 1000;
    }

    return XI_STATE_OK;
}

/* simulates the layer callback, it increases the int value passed through void* data
 * parameter */
xi_state_t
xi_utest_resource_manager_close_callback( void* data, void* data1, xi_state_t state )
{
    XI_UNUSED( data1 );
    XI_UNUSED( state );

    int* p = ( int* )data;
    if ( state == XI_STATE_OK )
    {
        *p += 1;
    }
    else
    {
        *p += 1000;
    }

    return XI_STATE_OK;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_resource_manager )

XI_TT_TESTCASE(
    utest__xi_resource_manager_create_context__valid_data_empty_buffer__return_XI_STATE_OK,
    {
        xi_resource_manager_context_t* context = NULL;
        xi_state_t res                         = XI_STATE_OK;

        res = xi_resource_manager_make_context( NULL, &context );
        tt_int_op( XI_STATE_OK, ==, res );

        tt_int_op( NULL, !=, context );
        tt_ptr_op( NULL, ==, context->data_buffer );
        tt_int_op( 0, ==, context->cs );
        tt_int_op( 0, >, context->resource_handle );
        tt_int_op( 0, ==, context->open_flags );
        tt_int_op( XI_MEMORY_TYPE_MANAGED, ==, context->memory_type );

        res = xi_resource_manager_free_context( &context );
        tt_int_op( XI_STATE_OK, ==, res );

        tt_ptr_op( NULL, ==, context );

        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;
    end:
        xi_resource_manager_free_context( &context );
    } )

XI_TT_TESTCASE(
    utest__xi_resource_manager_create_context__valid_data_filled_buffer__return_XI_STATE_OK,
    {
        xi_resource_manager_context_t* context = NULL;
        xi_data_desc_t* data_desc              = xi_make_empty_desc_alloc( 128 );
        xi_state_t res                         = XI_STATE_OK;

        tt_ptr_op( NULL, !=, data_desc );

        res = xi_resource_manager_make_context( data_desc, &context );
        tt_int_op( XI_STATE_OK, ==, res );

        tt_int_op( NULL, !=, context );
        tt_ptr_op( context->data_buffer, ==, data_desc );
        tt_int_op( 0, ==, context->cs );
        tt_int_op( 0, >, context->resource_handle );
        tt_int_op( 0, ==, context->open_flags );
        tt_int_op( XI_MEMORY_TYPE_UNMANAGED, ==, context->memory_type );

        res = xi_resource_manager_free_context( &context );
        tt_int_op( XI_STATE_OK, ==, res );
        tt_ptr_op( NULL, ==, context );

        xi_free_desc( &data_desc );

        tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

        return;
    end:
        xi_free_desc( &data_desc );
        xi_resource_manager_free_context( &context );
    } )

XI_TT_TESTCASE(
    utest__xi_resource_manager_open_resource__valid_data__return_XI_STATE_OK, {
        xi_fs_functions_t fs_functions_copy = xi_internals.fs_functions;
        xi_internals.fs_functions           = xi_resource_manager_fs_functions;
        xi_state_t res                      = XI_STATE_OK;

        xi_resource_manager_context_t* context = NULL;

        size_t test_no = 0;
        for ( ; test_no < XI_ARRAYSIZE( xi_utest_resource_manager_fs_limit_test_cases );
              ++test_no )
        {
            int test_value = 0;
            xi_utest_resource_manager_fs_set_limit_test_case( test_no );

            xi_globals.evtd_instance = xi_evtd_create_instance();
            tt_ptr_op( NULL, !=, xi_globals.evtd_instance );

            res = xi_resource_manager_make_context( NULL, &context );
            tt_int_op( XI_STATE_OK, ==, res );

            res = xi_resource_manager_open(
                context, xi_make_handle( &xi_utest_resource_manager_open_callback,
                                         &test_value, NULL, XI_STATE_OK ),
                XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, XI_FS_OPEN_READ,
                NULL );

            tt_int_op( XI_STATE_OK, ==, res );

            /* invocation of coroutine */
            xi_evtd_single_step( xi_globals.evtd_instance, 1 );

            {
                const uint8_t least_required_num_of_evtd_steps =
                    xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCTION_OPEN] +
                    xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCTION_STAT];
                uint8_t steps_already_taken =
                    2; /* because two times the coroutine get's activated automaticaly */

                for ( ; steps_already_taken < least_required_num_of_evtd_steps;
                      ++steps_already_taken )
                {
                    tt_int_op( 1, ==, xi_evtd_update_file_fd_events(
                                          xi_globals.evtd_instance ) );
                    tt_int_op( 0, ==, test_value );
                }
            }

            tt_int_op( 0, ==, test_value );

            /* invocation of callback */
            xi_evtd_single_step( xi_globals.evtd_instance, 1 );

            tt_int_op( 1, ==, test_value );
            tt_int_op( 0, <=, context->resource_handle );

            xi_evtd_destroy_instance( xi_globals.evtd_instance );

            res = xi_resource_manager_free_context( &context );
            tt_int_op( XI_STATE_OK, ==, res );
            tt_ptr_op( NULL, ==, context );

            tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

            xi_utest_resource_manager_fs_counter_reset();
        }

        xi_internals.fs_functions = fs_functions_copy;

        return;

    end:
        xi_evtd_destroy_instance( xi_globals.evtd_instance );
        xi_resource_manager_free_context( &context );
        xi_internals.fs_functions = fs_functions_copy;
        xi_utest_resource_manager_fs_counter_reset();
    } )

XI_TT_TESTCASE(
    utest__xi_resource_manager_read_resource__valid_data__return_XI_STATE_OK_and_buffer_filled_with_data,
    {
        xi_resource_manager_context_t* context = NULL;
        xi_data_desc_t* local_buffer           = NULL;

        xi_fs_functions_t fs_functions_copy = xi_internals.fs_functions;
        xi_internals.fs_functions           = xi_resource_manager_fs_functions;

        xi_state_t res = XI_STATE_OK;

        size_t test_no = 0;
        for ( ; test_no < XI_ARRAYSIZE( xi_utest_resource_manager_fs_limit_test_cases );
              ++test_no )
        {
            int test_type = 0;
            for ( ; test_type < 1; ++test_type )
            {
                int test_value = 0;
                xi_utest_resource_manager_fs_set_limit_test_case( test_no );

                xi_globals.evtd_instance = xi_evtd_create_instance();
                tt_ptr_op( NULL, !=, xi_globals.evtd_instance );

                if ( test_type == 0 )
                {
                    local_buffer = xi_make_empty_desc_alloc( 1 );
                    res = xi_resource_manager_make_context( local_buffer, &context );
                    tt_int_op( XI_STATE_OK, ==, res );
                }
                else
                {
                    res = xi_resource_manager_make_context( NULL, &context );
                    tt_int_op( XI_STATE_OK, ==, res );
                }


                xi_state_t res = xi_resource_manager_open(
                    context, xi_make_handle( &xi_utest_resource_manager_open_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, XI_FS_OPEN_READ,
                    NULL );

                tt_int_op( XI_STATE_OK, ==, res );

                /* invocation of open coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                /* open partial reading */
                {
                    const uint8_t least_required_num_of_evtd_steps =
                        xi_utest_resource_manager_fs_limits
                            [XI_UTEST_RM_FS_FUNCTION_OPEN] +
                        xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCTION_STAT];
                    uint8_t steps_already_taken =
                        2; /* two times the coroutine gets activated automaticaly */

                    for ( ; steps_already_taken < least_required_num_of_evtd_steps;
                          ++steps_already_taken )
                    {
                        tt_int_op( 1, ==, xi_evtd_update_file_fd_events(
                                              xi_globals.evtd_instance ) );
                        tt_int_op( 0, ==, test_value );
                    }
                }

                /* invocation of open callback */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                tt_int_op( 1, ==, test_value );

                /* the resource is open and ready to read */
                res = xi_resource_manager_read(
                    context, xi_make_handle( &xi_utest_resource_manager_read_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    NULL );

                tt_int_op( XI_STATE_OK, ==, res );

                /* invocation of read coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                /* call each part of read */
                {
                    const uint8_t least_required_num_of_evtd_steps =
                        xi_utest_resource_manager_fs_limits[XI_UTEST_RM_FS_FUNCTION_READ];
                    uint8_t steps_already_taken =
                        1; /* one time the coroutine gets activated automaticaly */

                    for ( ; steps_already_taken < least_required_num_of_evtd_steps;
                          ++steps_already_taken )
                    {
                        tt_int_op( 1, ==, xi_evtd_update_file_fd_events(
                                              xi_globals.evtd_instance ) );
                        tt_int_op( 1, ==, test_value );
                    }
                }

                tt_int_op( 1, ==, test_value );

                /* invocation of read callback function */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                /* here the whole buffer should be read */
                tt_int_op( 2, ==, test_value );
                tt_int_op( context->data_buffer->length, ==,
                           sizeof( xi_utest_resource_manager_test_buffer ) );
                tt_int_op( 0, ==,
                           memcmp( context->data_buffer->data_ptr,
                                   xi_utest_resource_manager_test_buffer,
                                   sizeof( xi_utest_resource_manager_test_buffer ) ) );

                /* the resource is ready to be closed */
                res = xi_resource_manager_close(
                    context, xi_make_handle( &xi_utest_resource_manager_close_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    NULL );

                /* invocation of close coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                /* call each part of close */
                {
                    const uint8_t least_required_num_of_evtd_steps =
                        xi_utest_resource_manager_fs_limits
                            [XI_UTEST_RM_FS_FUNCTION_CLOSE];
                    uint8_t steps_already_taken =
                        1; /* one time coroutine gets activated automaticaly */

                    for ( ; steps_already_taken < least_required_num_of_evtd_steps;
                          ++steps_already_taken )
                    {
                        tt_int_op( 1, ==, xi_evtd_update_file_fd_events(
                                              xi_globals.evtd_instance ) );
                        tt_int_op( 2, ==, test_value );
                    }
                }

                tt_int_op( 2, ==, test_value );

                /* invocation of close callback function */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                tt_int_op( 3, ==, test_value );
                tt_int_op( 0, >, context->resource_handle );

                /* the resource is closed */
                xi_evtd_destroy_instance( xi_globals.evtd_instance );

                res = xi_resource_manager_free_context( &context );
                tt_int_op( XI_STATE_OK, ==, res );
                tt_ptr_op( NULL, ==, context );

                if ( test_type == 0 )
                {
                    xi_free_desc( &local_buffer );
                }

                tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

                xi_utest_resource_manager_fs_counter_reset();
            }
        }

        xi_internals.fs_functions = fs_functions_copy;

        return;

    end:
        xi_evtd_destroy_instance( xi_globals.evtd_instance );
        xi_resource_manager_free_context( &context );
        xi_internals.fs_functions = fs_functions_copy;
        xi_utest_resource_manager_fs_counter_reset();
        xi_free_desc( &local_buffer );
    } )

XI_TT_TESTCASE(
    utest__xi_resource_manager_read_resource__valid_data__fs_functions_failing, {
        xi_resource_manager_context_t* context = NULL;
        xi_data_desc_t* local_buffer           = NULL;

        xi_fs_functions_t fs_functions_copy = xi_internals.fs_functions;
        xi_internals.fs_functions           = xi_resource_manager_fs_failing_functions;
        xi_state_t res                      = XI_STATE_OK;

        size_t test_no = 0;
        for ( ; test_no < XI_ARRAYSIZE( xi_utest_resource_manager_fs_limit_test_cases );
              ++test_no )
        {
            int test_type = 0;
            for ( ; test_type < 1; ++test_type )
            {
                int test_value = 0;
                xi_utest_resource_manager_fs_set_limit_test_case( test_no );

                xi_globals.evtd_instance = xi_evtd_create_instance();
                tt_ptr_op( NULL, !=, xi_globals.evtd_instance );

                if ( test_type == 0 )
                {
                    local_buffer = xi_make_empty_desc_alloc( 1 );
                    res = xi_resource_manager_make_context( local_buffer, &context );
                    tt_int_op( XI_STATE_OK, ==, res );
                }
                else
                {
                    res = xi_resource_manager_make_context( NULL, &context );
                    tt_int_op( XI_STATE_OK, ==, res );
                }

                xi_state_t res = xi_resource_manager_open(
                    context, xi_make_handle( &xi_utest_resource_manager_open_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, XI_FS_OPEN_READ,
                    NULL );

                tt_int_op( XI_STATE_OK, ==, res );

                /* invocation of open coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                tt_int_op( 1000, ==, test_value );

                /* let's pretend that we have valid handle */
                context->resource_handle = 1;

                /* the resource is open and ready to read */
                res = xi_resource_manager_read(
                    context, xi_make_handle( &xi_utest_resource_manager_read_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    NULL );

                tt_int_op( XI_STATE_OK, ==, res );

                /* invocation of read coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                /* here the whole buffer should be read */
                tt_int_op( 2000, ==, test_value );

                if ( test_type == 1 )
                {
                    tt_ptr_op( NULL, ==, context->data_buffer );
                }

                /* the resource is ready to be closed */
                res = xi_resource_manager_close(
                    context, xi_make_handle( &xi_utest_resource_manager_close_callback,
                                             &test_value, NULL, XI_STATE_OK ),
                    NULL );

                /* invocation of close coroutine */
                xi_evtd_single_step( xi_globals.evtd_instance, 1 );

                tt_int_op( 3000, ==, test_value );

                xi_evtd_destroy_instance( xi_globals.evtd_instance );

                res = xi_resource_manager_free_context( &context );
                tt_int_op( XI_STATE_OK, ==, res );
                tt_ptr_op( NULL, ==, context );

                if ( test_type == 0 )
                {
                    xi_free_desc( &local_buffer );
                }

                tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );

                xi_utest_resource_manager_fs_counter_reset();
            }
        }

        xi_internals.fs_functions = fs_functions_copy;

        return;

    end:
        xi_evtd_destroy_instance( xi_globals.evtd_instance );
        xi_resource_manager_free_context( &context );
        xi_internals.fs_functions = fs_functions_copy;
        xi_utest_resource_manager_fs_counter_reset();
        xi_free_desc( &local_buffer );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
