/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xively.h"
#include "xi_version.h"
#include "xi_err.h"
#include "xi_helpers.h"
#include "xi_memory_checks.h"
#include "xi_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

xi_state_t h0( void )
{
    tt_assert( 0 == 0 );
end:
    return 0;
}

xi_state_t h1( xi_event_handle_arg1_t a1 )
{
    tt_assert( a1 == 0 );
end:
    return 0;
}

xi_state_t h2( xi_event_handle_arg1_t a1, xi_event_handle_arg2_t a2 )
{
    tt_assert( a1 == 0 );
    tt_assert( *( ( int* )a2 ) == 22 );
end:
    return 0;
}

xi_state_t
h3( xi_event_handle_arg1_t a1, xi_event_handle_arg2_t a2, xi_event_handle_arg3_t a3 )
{
    tt_assert( a1 == 0 );
    tt_assert( *( ( int* )a2 ) == 222 );
    tt_assert( a3 == 0 );
end:
    return 0;
}

xi_state_t h4( xi_event_handle_arg1_t a1,
               xi_event_handle_arg2_t a2,
               xi_event_handle_arg3_t a3,
               xi_event_handle_arg4_t a4 )
{
    tt_assert( a1 == 0 );
    tt_assert( *( ( int* )a2 ) == 2222 );
    tt_assert( a3 == 0 );
    tt_assert( *( ( int* )a4 ) == 4444 );
end:
    return 0;
}

xi_state_t h5( xi_event_handle_arg1_t a1,
               xi_event_handle_arg2_t a2,
               xi_event_handle_arg3_t a3,
               xi_event_handle_arg4_t a4,
               xi_event_handle_arg5_t a5 )
{
    tt_assert( a1 == 0 );
    tt_assert( *( ( int* )a2 ) == 22222 );
    tt_assert( a3 == 0 );
    tt_assert( *( ( int* )a4 ) == 44444 );
    tt_assert( *( ( int* )a5 ) == 55555 );
end:
    return 0;
}

xi_state_t h6( xi_event_handle_arg1_t a1,
               xi_event_handle_arg2_t a2,
               xi_event_handle_arg3_t a3,
               xi_event_handle_arg4_t a4,
               xi_event_handle_arg5_t a5,
               xi_event_handle_arg6_t a6 )
{
    tt_assert( a1 == 0 );
    tt_assert( *( ( int* )a2 ) == 222222 );
    tt_assert( a3 == 0 );
    tt_assert( *( ( int* )a4 ) == 444444 );
    tt_assert( *( ( int* )a5 ) == 555555 );
    tt_assert( *( ( int* )a6 ) == 666666 );
end:
    return 0;
}


extern void xi_default_client_callback( xi_context_handle_t in_context_handle,
                                        void* data,
                                        xi_state_t state );
#endif


XI_TT_TESTGROUP_BEGIN( utest_core )

XI_TT_TESTCASE( test_initialize_shutdown_free_up_all_memory, {
    xi_initialize( "test-acc" );
    xi_shutdown();

    xi_initialize( "test-acc" );
    xi_shutdown();

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE_WITH_SETUP(
    test_create_and_delete_context, xi_utest_setup_basic, xi_utest_teardown_basic, NULL, {
        xi_context_handle_t xi_context_handle = xi_create_context();
        tt_assert( xi_context_handle > XI_INVALID_CONTEXT_HANDLE );

        xi_delete_context( xi_context_handle );

    end:;
    } )

XI_TT_TESTCASE( test_make_handles, {
    xi_event_handle_t eh;

    int val_a2                = 22;
    xi_event_handle_arg2_t a2 = &val_a2;
    int val_a4                = 4444;
    xi_event_handle_arg4_t a4 = &val_a4;
    int val_a5                = 55555;
    xi_event_handle_arg5_t a5 = &val_a5;
    int val_a6                = 666666;
    xi_event_handle_arg6_t a6 = &val_a6;

    eh = xi_make_handle( h0 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC0 );
    xi_evtd_execute_handle( &eh );

    eh = xi_make_handle( h1, 0 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC1 );
    xi_evtd_execute_handle( &eh );

    eh = xi_make_handle( h2, 0, a2 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC2 );
    xi_evtd_execute_handle( &eh );

    val_a2 += 200;

    eh = xi_make_handle( h3, 0, a2, 0 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC3 );
    xi_evtd_execute_handle( &eh );

    val_a2 += 2000;

    eh = xi_make_handle( h4, 0, a2, 0, a4 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC4 );
    xi_evtd_execute_handle( &eh );

    val_a2 += 20000;
    val_a4 += 40000;

    eh = xi_make_handle( h5, 0, a2, 0, a4, a5 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC5 );
    xi_evtd_execute_handle( &eh );

    val_a2 += 200000;
    val_a4 += 400000;
    val_a5 += 500000;

    eh = xi_make_handle( h6, 0, a2, 0, a4, a5, a6 );
    tt_assert( eh.handle_type == XI_EVENT_HANDLE_ARGC6 );
    xi_evtd_execute_handle( &eh );

end:;
} )

XI_TT_TESTCASE( test_version_major, {
    tt_assert( XI_MAJOR == xi_major );
    tt_assert( 0 != xi_major );
end:;
} )

XI_TT_TESTCASE( test_version_minor, {
    tt_assert( XI_MINOR == xi_minor );
end:;
} )

XI_TT_TESTCASE( test_version_revision, {
    tt_assert( XI_REVISION == xi_revision );
end:;
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
