/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_fs_api.h"
#include "xi_fs_header.h"
#include "xi_internals.h"
#include "xively.h"
#include "xi_macros.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

xi_state_t xi_fs_utest_stat( const void* context,
                             const xi_fs_resource_type_t resource_type,
                             const char* const resource_name,
                             xi_fs_stat_t* resource_stat )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );
    XI_UNUSED( resource_stat );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

xi_state_t xi_fs_utest_open( const void* context,
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

xi_state_t xi_fs_utest_read( const void* context,
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

xi_state_t xi_fs_utest_write( const void* context,
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
xi_fs_utest_close( const void* context, const xi_fs_resource_handle_t resource_handle )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_handle );

    return XI_FS_ERROR;
}

xi_state_t xi_fs_utest_remove( const void* context,
                               const xi_fs_resource_type_t resource_type,
                               const char* const resource_name )
{
    XI_UNUSED( context );
    XI_UNUSED( resource_type );
    XI_UNUSED( resource_name );

    return XI_FS_RESOURCE_NOT_AVAILABLE;
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_fs )

#ifdef XI_EXPOSE_FS
XI_TT_TESTCASE( utest__xi_set_fs_functions__correct_values, {
    const xi_fs_functions_t fs_functions = {
        sizeof( xi_fs_functions_t ), &xi_fs_utest_stat,  &xi_fs_utest_open,
        &xi_fs_utest_read,           &xi_fs_utest_write, &xi_fs_utest_close,
        &xi_fs_utest_remove};

    xi_state_t ret = xi_set_fs_functions( fs_functions );

    tt_int_op( ret, ==, XI_STATE_OK );

    tt_ptr_op( xi_internals.fs_functions.stat_resource, ==, &xi_fs_utest_stat );
    tt_ptr_op( xi_internals.fs_functions.open_resource, ==, &xi_fs_utest_open );
    tt_ptr_op( xi_internals.fs_functions.read_resource, ==, &xi_fs_utest_read );
    tt_ptr_op( xi_internals.fs_functions.write_resource, ==, &xi_fs_utest_write );
    tt_ptr_op( xi_internals.fs_functions.close_resource, ==, &xi_fs_utest_close );
    tt_ptr_op( xi_internals.fs_functions.remove_resource, ==, &xi_fs_utest_remove );
end:;
} )

XI_TT_TESTCASE( utest__xi_set_fs_functions__incorrect_size, {
    const xi_fs_functions_t fs_functions = {sizeof( xi_fs_functions_t ) - 2,
                                            &xi_fs_utest_stat,
                                            &xi_fs_utest_open,
                                            &xi_fs_utest_read,
                                            &xi_fs_utest_write,
                                            &xi_fs_utest_close,
                                            &xi_fs_utest_remove};

    xi_state_t ret = xi_set_fs_functions( fs_functions );

    tt_int_op( ret, ==, XI_INTERNAL_ERROR );
end:;
} )
#endif

/* This is the shared tests part, this behaviour should be common for all fs
 * implementations except dummy */
#if defined( XI_FS_MEMORY ) || defined( XI_FS_POSIX )
XI_TT_TESTCASE( utest__xi_fs_stat__invalid_resource_name__invalid_parameter_returned, {
    xi_fs_stat_t stat = {.resource_size = 0};
    xi_state_t ret = xi_fs_stat( NULL, XI_FS_CERTIFICATE, NULL, &stat );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_stat__invalid_stat__invalid_parameter_returned, {
    xi_state_t ret = xi_fs_stat( NULL, XI_FS_CERTIFICATE,
                                 "file_name_that_does_not_exist.xively_rulez!", NULL );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
end:;
} )

XI_TT_TESTCASE(
    utest__xi_fs_stat__invalid_file_name_parameters__resource_not_availible_returned, {
        xi_fs_stat_t stat = {.resource_size = 0};
        xi_state_t ret =
            xi_fs_stat( NULL, XI_FS_CERTIFICATE,
                        "file_name_that_does_not_exist.xively_rulez!", &stat );

        tt_int_op( ret, ==, XI_FS_RESOURCE_NOT_AVAILABLE );
        tt_int_op( stat.resource_size, ==, 0 );
    end:;
    } )

XI_TT_TESTCASE( utest__xi_fs_open__invalid_file_name__resource_not_availible_returned, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

    xi_state_t ret = xi_fs_open( NULL, XI_FS_CONFIG_DATA,
                                 "file_name_that_does_not_exist.xively_rulez!",
                                 XI_FS_OPEN_READ, &resource_handle );

    tt_int_op( ret, ==, XI_FS_RESOURCE_NOT_AVAILABLE );
    tt_int_op( resource_handle, ==, XI_FS_INVALID_RESOURCE_HANDLE );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_read__invalid_resource_handle__invalid_parameter_returned, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();

    const uint8_t* buffer = NULL;
    size_t buffer_size    = 0;

    xi_state_t ret = xi_fs_read( NULL, resource_handle, 0, &buffer, &buffer_size );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
    tt_ptr_op( NULL, ==, buffer );
    tt_int_op( 0, ==, buffer_size );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_memory_close__invalid_resource_handle, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    xi_state_t ret                          = xi_fs_close( NULL, resource_handle );

    tt_int_op( ret, ==, XI_INVALID_PARAMETER );
end:;
} )
#endif

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
