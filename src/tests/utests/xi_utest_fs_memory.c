/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_fs_header.h"
#include "xi_fs_filenames.h"
#include "xi_fs_api.h"
#include "xi_macros.h"

#ifndef XI_NO_TLS_LAYER
#include "xi_RootCA_list.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

XI_TT_TESTGROUP_BEGIN( utest_fs_memory )

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_stat__valid_data__stat_returned, {
    xi_fs_stat_t stat = {.resource_size = 0};
    xi_state_t ret =
        xi_fs_stat( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, &stat );

    tt_int_op( ret, ==, XI_STATE_OK );
    tt_int_op( stat.resource_size, ==, sizeof( xi_RootCA_list ) );

end:;
} )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_stat__invalid_stat__invalid_parameter_returned, {
    xi_state_t ret =
        xi_fs_stat( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, NULL );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
end:;
} )
#endif


#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE(
    utest__xi_fs_memory_open__valid_parameters__valid_resource_handle_returned, {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

        xi_state_t ret =
            xi_fs_open( NULL, XI_FS_CONFIG_DATA, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                        XI_FS_OPEN_READ, &resource_handle );

        tt_int_op( ret, ==, XI_STATE_OK );
        tt_int_op( resource_handle, !=, XI_FS_INVALID_RESOURCE_HANDLE );
    end:
        xi_fs_close( NULL, resource_handle );
    } )
#endif


#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE(
    utest__xi_fs_memory_read__valid_existing_resource__pointer_to_the_memory_block_returned,
    {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

        const uint8_t* buffer = NULL;
        size_t buffer_size    = 0;

        xi_state_t ret =
            xi_fs_open( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                        XI_FS_OPEN_READ, &resource_handle );

        tt_int_op( ret, ==, XI_STATE_OK );

        ret = xi_fs_read( NULL, resource_handle, 0, &buffer, &buffer_size );

        tt_int_op( ret, ==, XI_STATE_OK );
        tt_ptr_op( xi_RootCA_list, ==, buffer );
        tt_int_op( XI_MIN( sizeof( xi_RootCA_list ), xi_fs_buffer_size ), ==,
                   buffer_size );
        tt_int_op( memcmp( xi_RootCA_list, buffer,
                           XI_MIN( sizeof( xi_RootCA_list ), xi_fs_buffer_size ) ),
                   ==, 0 );

    end:
        xi_fs_close( NULL, resource_handle );
    } )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE(
    utest__xi_fs_memory_read__valid_existing_resource_with_valid_offset__pointer_to_the_memory_block_with_given_offset_returned,
    {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

        const uint8_t* buffer = NULL;
        const size_t offset   = sizeof( xi_RootCA_list ) / 2;
        const size_t size_left =
            XI_MIN( sizeof( xi_RootCA_list ) - offset, xi_fs_buffer_size );
        size_t buffer_size = 0;

        xi_state_t ret =
            xi_fs_open( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                        XI_FS_OPEN_READ, &resource_handle );

        tt_int_op( ret, ==, XI_STATE_OK );

        ret = xi_fs_read( NULL, resource_handle, offset, &buffer, &buffer_size );

        tt_int_op( ret, ==, XI_STATE_OK );
        tt_ptr_op( xi_RootCA_list + offset, ==, buffer );
        tt_int_op( size_left, ==, buffer_size );
        tt_int_op( memcmp( xi_RootCA_list + offset, buffer, size_left ), ==, 0 );

    end:;
        xi_fs_close( NULL, resource_handle );
    } )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE(
    utest__xi_fs_memory_read__valid_existing_resource_with_invalid_offset__pointer_to_the_memory_block_with_clamped_offset_returned,
    {
        xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

        const uint8_t* buffer  = NULL;
        const size_t offset    = sizeof( xi_RootCA_list ) + 1024;
        const size_t size_left = 0;
        size_t buffer_size     = 0;

        xi_state_t ret =
            xi_fs_open( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                        XI_FS_OPEN_READ, &resource_handle );

        tt_int_op( ret, ==, XI_STATE_OK );

        ret = xi_fs_read( NULL, resource_handle, offset, &buffer, &buffer_size );

        tt_int_op( ret, ==, XI_STATE_OK );
        tt_ptr_op( xi_RootCA_list + sizeof( xi_RootCA_list ), ==, buffer );
        tt_int_op( size_left, ==, buffer_size );
    end:
        xi_fs_close( NULL, resource_handle );
    } )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_read__invalid_buffer__invalid_parameter_returned, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

    size_t buffer_size = 0;

    xi_state_t ret = xi_fs_open( NULL, XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                                 XI_FS_OPEN_READ, &resource_handle );

    tt_int_op( ret, ==, XI_STATE_OK );

    ret = xi_fs_read( NULL, resource_handle, 0, NULL, &buffer_size );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
    tt_int_op( 0, ==, buffer_size );

end:
    xi_fs_close( NULL, resource_handle );
} )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_read__invalid_buffer_size__invalid_parameter_returned,
                {
                    xi_fs_resource_handle_t resource_handle =
                        xi_fs_init_resource_handle();

                    const uint8_t* buffer = NULL;

                    xi_state_t ret = xi_fs_open( NULL, XI_FS_CERTIFICATE,
                                                 XI_GLOBAL_CERTIFICATE_FILE_NAME,
                                                 XI_FS_OPEN_READ, &resource_handle );

                    tt_int_op( ret, ==, XI_STATE_OK );

                    ret = xi_fs_read( NULL, resource_handle, 0, &buffer, NULL );

                    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
                    tt_int_op( NULL, ==, buffer );

                end:
                    xi_fs_close( NULL, resource_handle );
                } )
#endif

XI_TT_TESTCASE( utest__xi_fs_memory_write__correct_parameters, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    uint8_t buffer[1024]                    = {'a'};
    size_t buffer_size                      = 1024;
    size_t bytes_written                    = 0;

    xi_state_t ret =
        xi_fs_write( NULL, resource_handle, buffer, buffer_size, 0, &bytes_written );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_close__valid_resource_handle, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

    xi_state_t ret = xi_fs_open( NULL, XI_FS_CONFIG_DATA, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                                 XI_FS_OPEN_READ, &resource_handle );

    tt_int_op( ret, ==, XI_STATE_OK );
    tt_int_op( resource_handle, !=, XI_FS_INVALID_RESOURCE_HANDLE );

    ret = xi_fs_close( NULL, resource_handle );

    tt_int_op( ret, ==, XI_STATE_OK );
end:;
} )
#endif

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_open_twice_close_twice__valid_resource_handle, {
    xi_fs_resource_handle_t resource_handle1 = xi_fs_init_resource_handle();

    xi_state_t ret = XI_STATE_OK;

    ret = xi_fs_open( NULL, XI_FS_CONFIG_DATA, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                      XI_FS_OPEN_READ, &resource_handle1 );

    tt_int_op( ret, ==, XI_STATE_OK );
    tt_int_op( resource_handle1, !=, XI_FS_INVALID_RESOURCE_HANDLE );

    xi_fs_resource_handle_t resource_handle2 = xi_fs_init_resource_handle();

    ret = xi_fs_open( NULL, XI_FS_CONFIG_DATA, XI_GLOBAL_CERTIFICATE_FILE_NAME,
                      XI_FS_OPEN_READ, &resource_handle2 );

    tt_int_op( ret, ==, XI_STATE_OK );
    tt_int_op( resource_handle2, !=, XI_FS_INVALID_RESOURCE_HANDLE );

    ret = xi_fs_close( NULL, resource_handle1 );

    tt_int_op( ret, ==, XI_STATE_OK );

    ret = xi_fs_close( NULL, resource_handle2 );

    tt_int_op( ret, ==, XI_STATE_OK );
end:;
} )
#endif

XI_TT_TESTCASE( utest__xi_fs_memory_remove__incorrect_parameters, {
    xi_state_t ret = xi_fs_remove( NULL, XI_FS_CONFIG_DATA, "test_name" );

    tt_int_op( ret, ==, XI_FS_RESOURCE_NOT_AVAILABLE );
end:;
} )

#ifndef XI_NO_TLS_LAYER
XI_TT_TESTCASE( utest__xi_fs_memory_remove__correct_parameters, {
    xi_state_t ret =
        xi_fs_remove( NULL, XI_FS_CONFIG_DATA, XI_GLOBAL_CERTIFICATE_FILE_NAME );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )
#endif

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
