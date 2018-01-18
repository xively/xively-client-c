/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"
#include "xi_memory_checks.h"

#include "xi_fs_header.h"
#include "xi_fs_filenames.h"
#include "xi_macros.h"
#include "xi_bsp_rng.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
static const char* xi_utest_fs_posix_existing_file_name =
    "file_that_does_exist.utest_file";
static const char* xi_utest_fs_posix_not_existing_file_name =
    "file_that_does_not_exist.utest_file";
static const size_t xi_utest_fs_posix_file_test_size_small = 1024;
static const size_t xi_utest_fs_posix_file_test_size_big   = 4096;

/* this has to be global at least for now until we gain the ability to pass env variable
 * to tests */
char* xi_utest_fs_posix_memory_block = NULL;

void* xi_utest_fs_posix_create_test_random_file( const char* const file_name,
                                                 size_t file_size )
{
    XI_UNUSED( file_name );
    XI_UNUSED( file_size );

    unsigned i            = 0u;
    size_t blocks_written = 0u;
    char* memory_buffer   = NULL;

    FILE* fd = fopen( file_name, "wb" );

    if ( NULL == fd )
    {
        tt_abort();
    }

    memory_buffer = ( char* )malloc( file_size );

    if ( NULL == memory_buffer )
    {
        goto err_handling;
    }

    for ( ; i < file_size; ++i )
    {
        memory_buffer[i] = ( char )( xi_bsp_rng_get() % 255u );
    }

    blocks_written = fwrite( memory_buffer, 1, file_size, fd );

    if ( file_size != blocks_written )
    {
        tt_abort();
    }

end:
    fclose( fd );
    return memory_buffer;

err_handling:
    free( xi_utest_fs_posix_memory_block );
    tt_abort();

    return NULL;
}

int xi_utest_fs_posix_unlink_file_free_memory( const char* const file_name,
                                               char* memory_block )
{
    free( memory_block );

    int ret = unlink( file_name );

    if ( 0 != ret ) /* if error return error */
    {
        return 0;
    }

    /* otherwise return sucess */
    return 1;
}

void* utest__xi_fs_posix__setup_small( const struct testcase_t* t )
{
    XI_UNUSED( t );

    xi_utest_setup_basic( t );

    xi_utest_fs_posix_memory_block = ( char* )xi_utest_fs_posix_create_test_random_file(
        xi_utest_fs_posix_existing_file_name, xi_utest_fs_posix_file_test_size_small );

    tt_ptr_op( NULL, !=, xi_utest_fs_posix_memory_block );

    return xi_utest_fs_posix_memory_block;

end:
    return NULL;
}

void* utest__xi_fs_posix__setup_big( const struct testcase_t* t )
{
    XI_UNUSED( t );

    xi_utest_setup_basic( t );

    xi_utest_fs_posix_memory_block = ( char* )xi_utest_fs_posix_create_test_random_file(
        xi_utest_fs_posix_existing_file_name, xi_utest_fs_posix_file_test_size_big );

    tt_ptr_op( NULL, !=, xi_utest_fs_posix_memory_block );

    return xi_utest_fs_posix_memory_block;

end:
    return NULL;
}

int utest__xi_fs_posix__clean( const struct testcase_t* t, void* data )
{
    XI_UNUSED( t );

    int ret = xi_utest_fs_posix_unlink_file_free_memory(
        xi_utest_fs_posix_existing_file_name, ( char* )data );

    tt_int_op( 1, ==, ret );

    xi_utest_teardown_basic( t, data );

end:
    return ret;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_fs_posix )

XI_TT_TESTCASE_WITH_SETUP( utest__xi_fs_stat_posix__valid_data__stat_returned,
                           utest__xi_fs_posix__setup_small,
                           utest__xi_fs_posix__clean,
                           NULL,
                           {
                               xi_fs_stat_t resource_stat = {.resource_size = 0};
                               xi_state_t res = xi_fs_stat(
                                   NULL, XI_FS_CERTIFICATE,
                                   xi_utest_fs_posix_existing_file_name, &resource_stat );

                               tt_int_op( XI_STATE_OK, ==, res );
                               tt_int_op( resource_stat.resource_size, ==,
                                          xi_utest_fs_posix_file_test_size_small );

                           end:;
                           } )

XI_TT_TESTCASE( utest__xi_fs_stat_posix__valid_data__file_does_not_exist, {
    xi_fs_stat_t resource_stat = {.resource_size = 0};
    xi_state_t res =
        xi_fs_stat( NULL, XI_FS_CERTIFICATE, xi_utest_fs_posix_not_existing_file_name,
                    &resource_stat );

    tt_int_op( XI_FS_RESOURCE_NOT_AVAILABLE, ==, res );
    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE_WITH_SETUP( utest__xi_fs_posix__valid_data__open_success,
                           utest__xi_fs_posix__setup_small,
                           utest__xi_fs_posix__clean,
                           NULL,
                           {
                               xi_fs_resource_handle_t resource_handle =
                                   xi_fs_init_resource_handle();
                               xi_state_t res =
                                   xi_fs_open( NULL, XI_FS_CERTIFICATE,
                                               xi_utest_fs_posix_existing_file_name,
                                               XI_FS_OPEN_READ, &resource_handle );

                               tt_int_op( XI_STATE_OK, ==, res );
                               tt_int_op( resource_handle, !=,
                                          XI_FS_INVALID_RESOURCE_HANDLE );

                               xi_fs_close( NULL, resource_handle );

                               return;

                           end:
                               xi_fs_close( NULL, resource_handle );
                           } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_fs_posix__valid_data__read_success,
    utest__xi_fs_posix__setup_small,
    utest__xi_fs_posix__clean,
    NULL,
    {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
        xi_state_t res                          = XI_STATE_OK;

        res = xi_fs_open( NULL, XI_FS_CERTIFICATE, xi_utest_fs_posix_existing_file_name,
                          XI_FS_OPEN_READ, &resource_handle );

        const uint8_t* buffer = NULL;
        size_t buffer_size    = 0u;
        int memcmp_res        = 0;

        res = xi_fs_read( NULL, resource_handle, 0, &buffer, &buffer_size );

        tt_int_op( XI_STATE_OK, ==, res );
        tt_int_op( buffer_size, ==, xi_utest_fs_posix_file_test_size_small );

        memcmp_res = memcmp( buffer, xi_utest_fs_posix_memory_block,
                             xi_utest_fs_posix_file_test_size_small );
        tt_int_op( memcmp_res, ==, 0 );
        xi_fs_close( NULL, resource_handle );

        return;
    end:
        xi_fs_close( NULL, resource_handle );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_fs_posix__valid_data__read_in_chunks_success,
    utest__xi_fs_posix__setup_big,
    utest__xi_fs_posix__clean,
    NULL,
    {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
        xi_state_t res                          = XI_STATE_OK;

        res = xi_fs_open( NULL, XI_FS_CERTIFICATE, xi_utest_fs_posix_existing_file_name,
                          XI_FS_OPEN_READ, &resource_handle );

        uint8_t* accumulator = NULL;
        size_t bytes_read    = 0u;
        int memcmp_res       = 0;

        XI_ALLOC_BUFFER_AT( uint8_t, accumulator, xi_utest_fs_posix_file_test_size_big,
                            res );

        do
        {
            const uint8_t* buffer = NULL;
            size_t buffer_size    = 0u;

            res = xi_fs_read( NULL, resource_handle, bytes_read, &buffer, &buffer_size );
            XI_CHECK_STATE( res );
            memcpy( accumulator + bytes_read, buffer, buffer_size );
            bytes_read += buffer_size;
        } while ( bytes_read != xi_utest_fs_posix_file_test_size_big );

        tt_int_op( XI_STATE_OK, ==, res );
        tt_int_op( bytes_read, ==, xi_utest_fs_posix_file_test_size_big );

        memcmp_res = memcmp( accumulator, xi_utest_fs_posix_memory_block,
                             xi_utest_fs_posix_file_test_size_small );

        tt_int_op( memcmp_res, ==, 0 );
        xi_fs_close( NULL, resource_handle );
        XI_SAFE_FREE( accumulator );

        return;

    err_handling:
        tt_abort();
    end:
        XI_SAFE_FREE( accumulator );
        xi_fs_close( NULL, resource_handle );
    } )

XI_TT_TESTCASE_WITH_SETUP( utest__xi_fs_posix__valid_data__close_success,
                           utest__xi_fs_posix__setup_small,
                           utest__xi_fs_posix__clean,
                           NULL,
                           {
                               xi_fs_resource_handle_t resource_handle =
                                   xi_fs_init_resource_handle();
                               xi_state_t res = XI_STATE_OK;

                               res = xi_fs_open( NULL, XI_FS_CERTIFICATE,
                                                 xi_utest_fs_posix_existing_file_name,
                                                 XI_FS_OPEN_READ, &resource_handle );

                               res = xi_fs_close( NULL, resource_handle );
                               tt_int_op( XI_STATE_OK, ==, res );
                           end:;
                           } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
