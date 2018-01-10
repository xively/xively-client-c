/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_fs_header.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

XI_TT_TESTGROUP_BEGIN( utest_fs_dummy )

XI_TT_TESTCASE( utest__xi_fs_dummy_stat__values_not_important, {
    xi_fs_stat_t stat;
    xi_state_t ret = xi_fs_stat( NULL, XI_FS_CONFIG_DATA, "test_data", &stat );

    tt_int_op( ret, ==, XI_FS_RESOURCE_NOT_AVAILABLE );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_dummy_open__values_not_important, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    xi_state_t ret = xi_fs_open( NULL, XI_FS_CONFIG_DATA, "test_data", XI_FS_OPEN_READ,
                                 &resource_handle );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_dummy_read__values_not_important, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    const uint8_t* buffer                   = NULL;
    size_t buffer_size                      = 0;

    xi_state_t ret = xi_fs_read( NULL, resource_handle, 0, &buffer, &buffer_size );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_dummy_write__values_not_important, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    uint8_t buffer[1024]                    = {'a'};
    size_t buffer_size                      = 1024;
    size_t bytes_written                    = 0;

    xi_state_t ret =
        xi_fs_write( NULL, resource_handle, buffer, buffer_size, 0, &bytes_written );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_dummy_close__values_not_important, {
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    xi_state_t ret                          = xi_fs_close( NULL, resource_handle );

    tt_int_op( ret, ==, XI_FS_ERROR );
end:;
} )

XI_TT_TESTCASE( utest__xi_fs_dummy_remove__values_not_important, {
    xi_state_t ret = xi_fs_remove( NULL, XI_FS_CONFIG_DATA, "test_name" );

    tt_int_op( ret, ==, XI_FS_RESOURCE_NOT_AVAILABLE );
end:;
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
