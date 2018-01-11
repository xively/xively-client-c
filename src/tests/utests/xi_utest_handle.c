/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_memory_checks.h"

#include "xi_handle.h"
#include "xi_vector.h"
#include "xi_types.h"
#include "xively.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif

XI_TT_TESTGROUP_BEGIN( utest_handle )

XI_TT_TESTCASE( utest__xi_object_for_handle__object_found, {
    xi_vector_t* vector = xi_vector_create();

    void* object = ( void* )( intptr_t )333;
    xi_register_handle_for_object( vector, 5, object );
    xi_handle_t handle;
    xi_find_handle_for_object( vector, object, &handle );

    void* object2 = xi_object_for_handle( vector, handle );

    tt_want_ptr_op( object, ==, object2 );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;

} )

XI_TT_TESTCASE( utest__xi_object_for_handle__object_not_found, {
    xi_vector_t* vector = xi_vector_create();

    void* object = ( void* )( intptr_t )333;
    xi_register_handle_for_object( vector, 5, object );
    xi_handle_t handle;
    xi_find_handle_for_object( vector, object, &handle );
    handle += 1;

    xi_context_t* object2 = xi_object_for_handle( vector, handle );

    tt_want_ptr_op( object, !=, object2 );
    tt_want_ptr_op( object2, ==, NULL );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_find_handle_for_object__handle_found, {
    xi_vector_t* vector = xi_vector_create();

    void* object = ( void* )( intptr_t )333;
    xi_register_handle_for_object( vector, 5, object );
    xi_handle_t handle = XI_INVALID_CONTEXT_HANDLE;
    xi_state_t state   = xi_find_handle_for_object( vector, object, &handle );

    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( handle, >, XI_INVALID_CONTEXT_HANDLE );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_find_handle_for_object__handle_not_found, {
    xi_vector_t* vector = xi_vector_create();

    void* object = ( void* )( intptr_t )333;
    xi_register_handle_for_object( vector, 5, object );
    xi_context_handle_t handle = XI_INVALID_CONTEXT_HANDLE;
    object           = ( intptr_t* )object + 1; // cast required to keep IAR happy
    xi_state_t state = xi_find_handle_for_object( vector, object, &handle );

    tt_want_int_op( state, ==, XI_ELEMENT_NOT_FOUND );
    tt_want_int_op( handle, ==, XI_INVALID_CONTEXT_HANDLE );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_delete_handle_for_object__delete_success_handle_not_found, {
    xi_vector_t* vector = xi_vector_create();

    void* object = ( void* )( intptr_t )333;
    xi_register_handle_for_object( vector, 5, object );
    xi_state_t state           = XI_STATE_OK;
    xi_context_handle_t handle = XI_INVALID_CONTEXT_HANDLE;

    state = xi_find_handle_for_object( vector, object, &handle );
    tt_want_int_op( state, ==, XI_STATE_OK );
    tt_want_int_op( handle, >, XI_INVALID_CONTEXT_HANDLE );

    xi_state_t delete_state = xi_delete_handle_for_object( vector, object );
    tt_want_int_op( delete_state, ==, XI_STATE_OK );

    state = xi_find_handle_for_object( vector, object, &handle );
    tt_want_int_op( state, ==, XI_ELEMENT_NOT_FOUND );
    tt_want_int_op( handle, ==, XI_INVALID_CONTEXT_HANDLE );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_delete_handle_for_object__unsuccessful_delete, {
    xi_vector_t* vector = xi_vector_create();

    void* unregistered_object = ( void* )( intptr_t )444;
    xi_state_t delete_state = xi_delete_handle_for_object( vector, unregistered_object );

    tt_want_int_op( delete_state, ==, XI_ELEMENT_NOT_FOUND );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_register_handle_for_object__successful_registration, {
    xi_vector_t* vector = xi_vector_create();

    int32_t max_num_contexts = 10;
    int i;
    for ( i = 0; i < max_num_contexts; ++i )
    {
        void* object = ( void* )( intptr_t )222;
        xi_state_t state =
            xi_register_handle_for_object( vector, max_num_contexts, object );

        tt_want_int_op( state, ==, XI_STATE_OK );
    }

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( utest__xi_register_handle_for_object__unsuccessful_registration, {
    xi_vector_t* vector = xi_vector_create();

    int32_t max_num_contexts = 10;
    int i;
    for ( i = 0; i < max_num_contexts; ++i )
    {
        void* object = ( void* )( intptr_t )222;
        xi_register_handle_for_object( vector, max_num_contexts, object );
    }

    void* object     = ( void* )( intptr_t )333;
    xi_state_t state = xi_register_handle_for_object( vector, max_num_contexts, object );
    tt_want_int_op( state, ==, XI_NO_MORE_RESOURCE_AVAILABLE );

    xi_vector_destroy( vector );

    tt_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
