/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xi_vector.h"
#include "xi_memory_checks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

int8_t vector_is_odd( union xi_vector_selector_u* e0 )
{
    if ( ( e0->i32_value & 1 ) > 0 )
    {
        return 1;
    }

    return 0;
}

int8_t utest_datastructures_cmp_vector_i32( const union xi_vector_selector_u* e0,
                                            const union xi_vector_selector_u* e1 )
{
    if ( e0->i32_value < e1->i32_value )
    {
        return -1;
    }
    else if ( e0->i32_value > e1->i32_value )
    {
        return 1;
    }

    return 0;
}

int8_t utest_datastructures_cmp_vector_ptr( const union xi_vector_selector_u* e0,
                                            const union xi_vector_selector_u* e1 )
{
    if ( e0->ptr_value < e1->ptr_value )
    {
        return -1;
    }
    else if ( e0->ptr_value > e1->ptr_value )
    {
        return 1;
    }

    return 0;
}

int8_t utest_datastructures_cmp_vector_ui32( const union xi_vector_selector_u* e0,
                                             const union xi_vector_selector_u* e1 )
{
    if ( e0->ui32_value < e1->ui32_value )
    {
        return -1;
    }
    else if ( e0->ui32_value > e1->ui32_value )
    {
        return 1;
    }

    return 0;
}
#endif

/*-----------------------------------------------------------------------*/
// HEAP TESTS
/*-----------------------------------------------------------------------*/
XI_TT_TESTGROUP_BEGIN( utest_datastructures )

XI_TT_TESTCASE( test_vector_create, {
    xi_vector_t* sv = xi_vector_create();

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );
    tt_assert( sv->capacity == 2 );
    tt_assert( sv->elem_no == 0 );
    tt_assert( sv->memory_type == XI_MEMORY_TYPE_MANAGED );

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( test_vector_reserve_upsize, {
    xi_vector_t* sv = xi_vector_create();

    xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )15 ) ) );

    int8_t result = xi_vector_reserve( sv, 13 );

    tt_assert( result == 1 );
    tt_assert( sv->capacity == 13 );
    tt_assert( sv->elem_no == 1 );
    tt_assert( sv->array[0].selector_t.i32_value == 15 );

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( xi_utest__vector_create_from__valid_data__return_vector, {
    xi_vector_elem_t v_data[] = {XI_VEC_ELEM( XI_VEC_VALUE_UI32( 1 ) ),
                                 XI_VEC_ELEM( XI_VEC_VALUE_UI32( 3 ) ),
                                 XI_VEC_ELEM( XI_VEC_VALUE_UI32( 0xFFFFFFFF ) )};
    xi_vector_t* sv =
        xi_vector_create_from( ( xi_vector_elem_t* )v_data, 3, XI_MEMORY_TYPE_UNMANAGED );

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );
    tt_assert( sv->capacity == 3 );
    tt_assert( sv->elem_no == 3 );
    tt_assert( XI_MEMORY_TYPE_UNMANAGED == sv->memory_type );

    size_t i = 0;
    for ( ; i < 3; ++i )
    {
        tt_want_uint_op( sv->array[i].selector_t.ui32_value, ==,
                         v_data[i].selector_t.ui32_value );
    }

    xi_vector_destroy( sv );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( xi_utest__xi_vector_create_from__valid_dynamic_data__return_vector, {
    xi_state_t local_state = XI_STATE_OK;

    const int data_size = 16;
    const int capacity  = sizeof( xi_vector_elem_t ) * data_size;

    XI_ALLOC_BUFFER( xi_vector_elem_t, v_data, capacity, local_state );

    int i = 0;
    for ( ; i < data_size; ++i )
    {
        v_data[i].selector_t.i32_value = i;
    }

    xi_vector_t* sv = xi_vector_create_from( ( xi_vector_elem_t* )v_data, data_size,
                                             XI_MEMORY_TYPE_MANAGED );

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );
    tt_assert( sv->capacity == data_size );
    tt_assert( sv->elem_no == data_size );
    tt_assert( XI_MEMORY_TYPE_MANAGED == sv->memory_type );

    i = 0;
    for ( ; i < data_size; ++i )
    {
        tt_want_uint_op( sv->array[i].selector_t.ui32_value, ==,
                         v_data[i].selector_t.ui32_value );
    }

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
err_handling:
end:;
} )


XI_TT_TESTCASE( test_vector_reserve_downsize, {
    xi_vector_t* sv = xi_vector_create();

    const xi_vector_index_type_t test_no_elements        = 13;
    const xi_vector_index_type_t test_no_elements_plus_5 = test_no_elements + 5;

    xi_vector_index_type_t i = 0;
    for ( ; i < test_no_elements_plus_5; ++i )
    {
        tt_assert( NULL != xi_vector_push(
                               sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( 15 ) ) ) );
    }

    int8_t result = xi_vector_reserve( sv, test_no_elements );

    tt_assert( result == 1 );
    tt_assert( sv->capacity == test_no_elements );
    tt_assert( sv->elem_no == test_no_elements );

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( test_vector_push, {
    xi_vector_t* sv = xi_vector_create();

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );

    xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )15 ) ) );

    tt_assert( sv->array[0].selector_t.ptr_value == ( void* )15 );
    tt_assert( sv->elem_no == 1 );

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_push_all, {
    xi_vector_t* sv = xi_vector_create();

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );

    int32_t i = 0;
    for ( ; i < 4; ++i )
    {
        const xi_vector_elem_t* e =
            xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
        tt_assert( &sv->array[i] == e );
    }

    tt_assert( sv->capacity == 4 );

    for ( i = 0; i < 4; ++i )
    {
        tt_assert( sv->array[i].selector_t.i32_value == i );
    }

    tt_assert( sv->elem_no == 4 );

    tt_assert( 0 != xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM(
                                            XI_VEC_VALUE_PTR( ( void* )123 ) ) ) );

    tt_assert( sv->elem_no == 5 );
end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_del, {
    xi_vector_t* sv = xi_vector_create();

    int32_t i = 0;
    for ( ; i < 16; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
    }

    xi_vector_del( sv, 0 );

    tt_assert( sv->array[0].selector_t.i32_value == 15 );
    tt_assert( sv->array[1].selector_t.i32_value == 1 );
    tt_assert( sv->array[2].selector_t.i32_value == 2 );
    tt_assert( sv->elem_no == 15 );

    xi_vector_del( sv, 1 );

    tt_assert( sv->array[0].selector_t.i32_value == 15 );
    tt_assert( sv->array[1].selector_t.i32_value == 14 );
    tt_assert( sv->elem_no == 14 );

    tt_assert( sv != 0 );
    tt_assert( sv->array != 0 );

    for ( i = 0; i < 14; ++i )
    {
        xi_vector_del( sv, 0 );
    }

    tt_assert( sv->capacity == 16 );

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_find_I32, {
    xi_vector_t* sv = xi_vector_create();

    int32_t i = 0;
    for ( ; i < 4; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
    }

    for ( i = 3; i >= 0; --i )
    {
        tt_assert( i == xi_vector_find( sv,
                                        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ),
                                        &utest_datastructures_cmp_vector_i32 ) );
    }

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_find_UI32, {
    xi_vector_t* sv = xi_vector_create();

    uint32_t i = 0;
    for ( ; i < 4; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( i ) ) );
    }

    int8_t vi;
    for ( vi = 3; vi >= 0; --vi )
    {
        i = ( uint32_t )vi;
        tt_assert( vi ==
                   xi_vector_find( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( i ) ),
                                   &utest_datastructures_cmp_vector_ui32 ) );
    }

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_find_PTR, {
    xi_vector_t* sv = xi_vector_create();

    unsigned long i = 0;
    for ( ; i < 4; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( char* )i ) ) );
    }

    int8_t vi;
    for ( vi = 3; vi >= 0; --vi )
    {
        i = vi;
        tt_assert( vi == xi_vector_find( sv, XI_VEC_CONST_VALUE_PARAM(
                                                 XI_VEC_VALUE_PTR( ( char* )i ) ),
                                         &utest_datastructures_cmp_vector_ptr ) );
    }

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_get, {
    xi_vector_t* sv = xi_vector_create();

    int32_t i = 0;
    for ( ; i < 32; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
    }

    // Test with existing elements
    for ( ; i < 32; ++i )
    {
        void* elem = xi_vector_get( sv, i );
        tt_assert( elem == ( void* )( intptr_t )i );
    }

    // Test overindexing
    void* elem = xi_vector_get( sv, 32 );
    tt_assert( NULL == elem );

end:;
    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_remove_if_I32, {
    xi_vector_t* sv = xi_vector_create();

    int32_t i = 0;
    for ( ; i < 10; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
    }

    xi_vector_remove_if( sv, &vector_is_odd );

    tt_want_int_op( sv->elem_no, ==, 5 );

    for ( i = 0; i < sv->elem_no; ++i )
    {
        tt_want_int_op( ( sv->array[i].selector_t.i32_value & 1 ), ==, 0 );
    }

    xi_vector_destroy( sv );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_remove_if_UI32, {
    xi_vector_t* sv = xi_vector_create();

    int32_t i = 0;
    for ( ; i < 10; ++i )
    {
        xi_vector_push( sv,
                        XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( ( uint32_t )i ) ) );
    }

    xi_vector_remove_if( sv, &vector_is_odd );

    tt_want_int_op( sv->elem_no, ==, 5 );

    for ( i = 0; i < sv->elem_no; ++i )
    {
        tt_want_int_op( ( sv->array[i].selector_t.ui32_value & 1 ), ==, 0 );
    }

    xi_vector_destroy( sv );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTCASE( test_vector_remove_if_PTR, {
    xi_vector_t* sv = xi_vector_create();

    unsigned long i = 0;
    for ( ; i < 10; ++i )
    {
        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( ( void* )i ) ) );
    }

    xi_vector_remove_if( sv, &vector_is_odd );

    tt_want_int_op( sv->elem_no, ==, 5 );

    for ( i = 0; i < ( unsigned long )sv->elem_no; ++i )
    {
        tt_want_int_op( ( ( unsigned long )sv->array[i].selector_t.ptr_value & 1 ), ==,
                        0 );
    }

    xi_vector_destroy( sv );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
