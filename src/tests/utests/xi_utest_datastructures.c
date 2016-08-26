// Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library.

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xi_heap.h"
#include "xi_static_vector.h"
#include "xi_vector.h"

#include "xi_memory_checks.h"
#include "xi_rng.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

void test_heap( xi_heap_t* heap, xi_heap_index_type_t index )
{
    if ( index >= heap->first_free )
    {
        return;
    }

    xi_heap_element_t* e = heap->elements[ index ];

    xi_heap_index_type_t li = LEFT( index );
    xi_heap_index_type_t ri = RIGHT( index );

    li = li >= heap->first_free ? index : li;
    ri = ri >= heap->first_free ? index : ri;

    if ( li == index || ri == index ) return;

    xi_heap_element_t* le = heap->elements[ li ];
    xi_heap_element_t* re = heap->elements[ ri ];

    tt_assert( e->key <= le->key );
    tt_assert( e->key <= re->key );

    test_heap( heap, li );
    test_heap( heap, ri );

end:
    ;
}

int8_t utest_datastructures_cmp_static_vector(
    const union xi_static_vector_selector_u* e0,
    const union xi_static_vector_selector_u* e1 )
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

int8_t static_vector_is_odd( union xi_static_vector_selector_u* e0 )
{
    if( ( e0->i32_value & 1 ) > 0 )
    {
        return 1;
    }

    return 0;
}

int8_t vector_is_odd( union xi_vector_selector_u* e0 )
{
    if( ( e0->i32_value & 1 ) > 0 )
    {
        return 1;
    }

    return 0;
}

int8_t utest_datastructures_cmp_vector_i32(
    const union xi_vector_selector_u* e0,
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

int8_t utest_datastructures_cmp_vector_ptr(
    const union xi_vector_selector_u* e0,
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

int8_t utest_datastructures_cmp_vector_ui32(
    const union xi_vector_selector_u* e0,
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

void utest_static_vector_for_each_inc( union xi_static_vector_selector_u* e )
{
    e->i32_value += 1;
}

#endif

/*-----------------------------------------------------------------------*/
// HEAP TESTS
/*-----------------------------------------------------------------------*/
XI_TT_TESTGROUP_BEGIN( utest_datastructures )

XI_TT_TESTCASE( test_heap_index_calculus,
                {
                    xi_heap_index_type_t index = 0;

                    tt_assert( RIGHT( index ) == 2 );
                    tt_assert( LEFT( index ) == 1 );
                    tt_assert( PARENT( RIGHT( index ) ) == 0 );
                    tt_assert( PARENT( LEFT( index ) ) == 0 );

                end:
                    ;
                } )

XI_TT_TESTCASE( test_heap_create,
                {
                    xi_heap_t* heap = xi_heap_create();

                    tt_assert( heap != 0 );
                    tt_assert( heap->capacity == 2 );
                    tt_assert( heap->first_free == 0 );

                    int i = 0;
                    for ( ; i < heap->capacity; ++i )
                    {
                        tt_assert( heap->elements[ i ] == 0 );
                    }

                /* Every test-case function needs to finish with an "end:"
                   label and (optionally) code to clean up local variables. */
                end:
                    xi_heap_destroy( heap );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_heap_add,
                {
                    xi_heap_t* heap = xi_heap_create();

                    xi_heap_element_add_void( heap, 12, ( void* )12 );

                    tt_assert( heap->first_free == 1 );

                    tt_assert( ( heap->elements[ 0 ] )->index == 0 );
                    tt_assert( ( heap->elements[ 0 ] )->key == 12 );
                    tt_assert( ( heap->elements[ 0 ] )->heap_value.void_value
                               == ( void* )12 );

                    xi_heap_element_add_void( heap, 13, ( void* )13 );

                    tt_assert( heap->first_free == 2 );

                    tt_assert( ( heap->elements[ 1 ] )->index == 1 );
                    tt_assert( ( heap->elements[ 1 ] )->key == 13 );
                    tt_assert( ( heap->elements[ 1 ] )->heap_value.void_value
                               == ( void* )13 );

                    xi_heap_element_add_void( heap, 1, ( void* )1 );

                    tt_assert( ( heap->elements[ 0 ] )->index == 0 );
                    tt_assert( ( heap->elements[ 0 ] )->key == 1 );
                    tt_assert( ( heap->elements[ 0 ] )->heap_value.void_value
                               == ( void* )1 );

                    tt_assert( ( heap->elements[ 2 ] )->index == 2 );
                    tt_assert( ( heap->elements[ 2 ] )->key == 12 );
                    tt_assert( ( heap->elements[ 2 ] )->heap_value.void_value
                               == ( void* )12 );

                end:
                    xi_heap_destroy( heap );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_heap_sequencial_add,
                {
                    xi_heap_t* heap = xi_heap_create();

                    long i = 0;
                    for ( ; i < 16; ++i )
                    {
                        xi_heap_element_add_void( heap, i, ( void* )i );
                    }

                    for ( i = 0; i < 16; ++i )
                    {
                        tt_assert( heap->elements[ i ]->index == i );
                        tt_assert( heap->elements[ i ]->heap_value.void_value
                                   == ( void* )i );
                        tt_assert( heap->elements[ i ]->key == i );
                    }

                end:
                    xi_heap_destroy( heap );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE_WITH_SETUP( test_heap_random_add,
                           xi_utest_setup_basic, xi_utest_teardown_basic, NULL,
                {
                    xi_heap_t* heap = xi_heap_create();
                    srand( time( 0 ) );

                    size_t i = 0;
                    for ( ; i < 64; ++i )
                    {
                        xi_heap_index_type_t index = xi_rand() & 63;
                        xi_heap_element_add_void( heap, index, ( void* )i );
                    }

                    test_heap( heap, 0 );

                    xi_heap_destroy( heap );
                } )

XI_TT_TESTCASE_WITH_SETUP( test_heap_random_remove,
                           xi_utest_setup_basic, xi_utest_teardown_basic, NULL,
                {
                    xi_heap_t* heap = xi_heap_create();
                    srand( time( 0 ) );

                    size_t i = 0;
                    for ( ; i < 64; ++i )
                    {
                        xi_heap_index_type_t index = xi_rand() & 63;
                        xi_heap_element_add_void( heap, index, ( void* )i );
                    }

                    xi_heap_key_type_t key = 0;

                    for ( i = 0; i < 64; ++i )
                    {
                        const xi_heap_element_t* e = xi_heap_get_top( heap );
                        tt_assert( key <= e->key );
                        key = e->key;
                    }

                end:
                    xi_heap_destroy( heap );
                } )

XI_TT_TESTCASE( test_static_vector_create,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 16 );

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );
                    tt_assert( sv->capacity == 16 );
                    tt_assert( sv->elem_no == 0 );
                    tt_assert( XI_MEMORY_TYPE_MANAGED == sv->memory_type  );


                    xi_static_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );

                end:
                    ;
                } )

XI_TT_TESTCASE(
    xi_utest__xi_static_vector_create_from__valid_static_data__return_vector,
    {
        xi_static_vector_elem_t v_data[]
            = {XI_SVEC_ELEM( XI_SVEC_VALUE_UI32( 1 ) ), XI_SVEC_ELEM( XI_SVEC_VALUE_UI32( 3 ) ),
               XI_SVEC_ELEM( XI_SVEC_VALUE_UI32( 0xFFFFFFFF ) )};
        xi_static_vector_t* sv = xi_static_vector_create_from(
            ( xi_static_vector_elem_t* )v_data, 3, XI_MEMORY_TYPE_UNMANAGED );

        tt_assert( sv != 0 );
        tt_assert( sv->array != 0 );
        tt_assert( sv->capacity == 3 );
        tt_assert( sv->elem_no == 3 );
        tt_assert( XI_MEMORY_TYPE_UNMANAGED == sv->memory_type );

        size_t i = 0;
        for ( ; i < 3; ++i )
        {
            tt_want_uint_op( sv->array[ i ].selector_t.ui32_value, ==,
                             v_data[ i ].selector_t.ui32_value );
        }

        xi_static_vector_destroy( sv );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    end:
        ;
    } )

XI_TT_TESTCASE(
    xi_utest__xi_static_vector_create_from__valid_dynamic_data__return_vector,
    {
        xi_state_t local_state = XI_STATE_OK;

        const int data_size = 16;
        const int capacity = sizeof( xi_static_vector_elem_t ) * data_size;

        XI_ALLOC_BUFFER( xi_static_vector_elem_t, v_data, capacity,
                         local_state );

        int i = 0;
        for ( ; i < data_size; ++i )
        {
            v_data[ i ].selector_t.i32_value = i;
        }

        xi_static_vector_t* sv
            = xi_static_vector_create_from( ( xi_static_vector_elem_t* )v_data,
                                            data_size, XI_MEMORY_TYPE_MANAGED );

        tt_assert( sv != 0 );
        tt_assert( sv->array != 0 );
        tt_assert( sv->capacity == data_size );
        tt_assert( sv->elem_no == data_size );
        tt_assert( XI_MEMORY_TYPE_MANAGED == sv->memory_type );

        i = 0;
        for ( ; i < data_size; ++i )
        {
            tt_want_uint_op( sv->array[ i ].selector_t.ui32_value, ==,
                             v_data[ i ].selector_t.ui32_value );
        }

        xi_static_vector_destroy( sv );
        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    err_handling:
    end:
        ;
    } )

XI_TT_TESTCASE( test_static_vector_push,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 4 );

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                    xi_static_vector_push( sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_PTR( ( void* ) 15 ) ) );

                    tt_assert( sv->array[ 0 ].selector_t.ptr_value
                               == ( void* )15 );
                    tt_assert( sv->elem_no == 1 );

                end:
                    ;
                    xi_static_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_static_vector_push_all,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 4 );

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                    int32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        const xi_static_vector_elem_t* e
                            = xi_static_vector_push( sv,
                                                     XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_PTR( ( void* )( intptr_t )i ) ) );
                        tt_assert( &sv->array[ i ] == e );
                    }

                    for ( i = 0; i < 4; ++i )
                    {
                        tt_assert( sv->array[ i ].selector_t.ptr_value
                                   == ( void* )( intptr_t )i );
                    }

                    tt_assert( sv->elem_no == 4 );

                    tt_assert( 0 == xi_static_vector_push(
                                        sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_UI32( 123 ) ) ) );

                    tt_assert( sv->elem_no == 4 );
                end:
                    ;
                    xi_static_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_static_vector_del,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 4 );

                    int32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        xi_static_vector_push( sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_I32( i ) ) );
                    }

                    xi_static_vector_del( sv, 0 );

                    tt_assert( sv->array[ 0 ].selector_t.i32_value == 3 );
                    tt_assert( sv->array[ 1 ].selector_t.i32_value == 1 );
                    tt_assert( sv->array[ 2 ].selector_t.i32_value == 2 );
                    tt_assert( sv->elem_no == 3 );

                    xi_static_vector_del( sv, 1 );

                    tt_assert( sv->array[ 0 ].selector_t.i32_value == 3 );
                    tt_assert( sv->array[ 1 ].selector_t.i32_value == 2 );
                    tt_assert( sv->elem_no == 2 );

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                end:
                    ;
                    xi_static_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( utest__test_static_vector_for_each__valid_data__inc,
{
    xi_static_vector_t* sv = xi_static_vector_create( 4 );

    int32_t i = 0;
    for ( ; i < 4; ++i )
    {
        xi_static_vector_push( sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_I32( i ) ) );
    }

    xi_static_vector_for_each( sv, &utest_static_vector_for_each_inc );


    for ( ; i < 4; ++i )
    {
        tt_int_op( sv->array[ i ].selector_t.i32_value, ==, i + 1 );
    }

end:
    ;
    xi_static_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
})

XI_TT_TESTCASE( test_static_vector_remove_if,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 10 );

                    int32_t i = 0;
                    for ( ; i < 10; ++i )
                    {
                        xi_static_vector_push( sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_I32( i ) ) );
                    }

                    xi_static_vector_remove_if( sv, &static_vector_is_odd );

                    tt_want_int_op( sv->elem_no, ==, 5 );

                    for( i = 0 ; i < sv->elem_no; ++i )
                    {
                        tt_want_int_op( ( sv->array[ i ].selector_t.i32_value & 1 ), ==, 0 );
                    }

                    xi_static_vector_destroy( sv );

                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                })

XI_TT_TESTCASE( test_static_vector_find,
                {
                    xi_static_vector_t* sv = xi_static_vector_create( 4 );

                    int32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        xi_static_vector_push( sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_I32( i ) ) );
                    }

                    for ( i = 3; i >= 0; --i )
                    {
                        tt_assert( i == xi_static_vector_find(
                                            sv, XI_SVEC_CONST_VALUE_PARAM( XI_SVEC_VALUE_UI32( i ) ),
                                            &utest_datastructures_cmp_static_vector ) );
                    }

                end:
                    ;
                    xi_static_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_create,
                {
                    xi_vector_t* sv = xi_vector_create();

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );
                    tt_assert( sv->capacity == 2 );
                    tt_assert( sv->elem_no == 0 );

                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                end:
                    ;
                } )

XI_TT_TESTCASE( test_vector_assign, {
    xi_vector_t* sv = xi_vector_create();

    uint32_t rnd_value = rand();

    int8_t result =
        xi_vector_assign( sv, 13, XI_VEC_VALUE_PARAM( XI_VEC_VALUE_UI32( rnd_value ) ) );

    tt_assert( result == 1 );
    tt_assert( sv->capacity == 13 );
    tt_assert( sv->elem_no == 13 );

    xi_vector_index_type_t i = 0;
    for ( ; i < 13; ++i )
    {
        tt_assert( sv->array[i].selector_t.ui32_value == rnd_value )
    }

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
end:;
} )

XI_TT_TESTCASE( test_vector_reserve_upsize, {
    xi_vector_t* sv = xi_vector_create();

    xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (void *)15 ) ) );

    int8_t result =
        xi_vector_reserve( sv, 13 );

    tt_assert( result == 1 );
    tt_assert( sv->capacity == 13 );
    tt_assert( sv->elem_no == 1 );
    tt_assert( sv->array[0].selector_t.i32_value == 15 );

    xi_vector_destroy( sv );
    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
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

XI_TT_TESTCASE( test_vector_push,
                {
                    xi_vector_t* sv = xi_vector_create();

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                    xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (void *)15 ) ) );

                    tt_assert( sv->array[ 0 ].selector_t.ptr_value == ( void* )15 );
                    tt_assert( sv->elem_no == 1 );

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_push_all,
                {
                    xi_vector_t* sv = xi_vector_create();

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                    int32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        const xi_vector_elem_t* e
                            = xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
                        tt_assert( &sv->array[ i ] == e );
                    }

                    tt_assert( sv->capacity == 4 );

                    for ( i = 0; i < 4; ++i )
                    {
                        tt_assert( sv->array[ i ].selector_t.i32_value
                                   == i );
                    }

                    tt_assert( sv->elem_no == 4 );

                    tt_assert( 0 != xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (void *)123 ) ) ) );

                    tt_assert( sv->elem_no == 5 );
                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_del,
                {
                    xi_vector_t* sv = xi_vector_create();

                    int32_t i = 0;
                    for ( ; i < 16; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
                    }

                    xi_vector_del( sv, 0 );

                    tt_assert( sv->array[ 0 ].selector_t.i32_value == 15 );
                    tt_assert( sv->array[ 1 ].selector_t.i32_value == 1 );
                    tt_assert( sv->array[ 2 ].selector_t.i32_value == 2 );
                    tt_assert( sv->elem_no == 15 );

                    xi_vector_del( sv, 1 );

                    tt_assert( sv->array[ 0 ].selector_t.i32_value == 15 );
                    tt_assert( sv->array[ 1 ].selector_t.i32_value == 14 );
                    tt_assert( sv->elem_no == 14 );

                    tt_assert( sv != 0 );
                    tt_assert( sv->array != 0 );

                    for ( i = 0; i < 14; ++i )
                    {
                        xi_vector_del( sv, 0 );
                    }

                    tt_assert( sv->capacity == 1 );

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_find_I32,
                {
                    xi_vector_t* sv = xi_vector_create();

                    int32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
                    }

                    for ( i = 3; i >= 0; --i )
                    {
                        tt_assert(
                            i == xi_vector_find( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ),
                                                 &utest_datastructures_cmp_vector_i32 ) );
                    }

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_find_UI32,
                {
                    xi_vector_t* sv = xi_vector_create();

                    uint32_t i = 0;
                    for ( ; i < 4; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( i ) ) );
                    }

                    int8_t vi;
                    for ( vi = 3; vi >= 0; --vi )
                    {
                        i = (uint32_t) vi;
                        tt_assert(
                            vi == xi_vector_find( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( i ) ),
                                                 &utest_datastructures_cmp_vector_ui32 ) );
                    }

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_find_PTR,
                {
                    xi_vector_t* sv = xi_vector_create();

                    unsigned long i = 0;
                    for ( ; i < 4; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (char *) i ) ) );
                    }

                    int8_t vi;
                    for ( vi = 3; vi >= 0; --vi )
                    {
                        i = vi;
                        tt_assert(
                            vi == xi_vector_find( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (char *) i ) ),
                                                 &utest_datastructures_cmp_vector_ptr ) );
                    }

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_get,
                {
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

                end:
                    ;
                    xi_vector_destroy( sv );
                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                } )

XI_TT_TESTCASE( test_vector_remove_if_I32,
                {
                    xi_vector_t* sv = xi_vector_create( );

                    int32_t i = 0;
                    for ( ; i < 10; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_I32( i ) ) );
                    }

                    xi_vector_remove_if( sv, &vector_is_odd );

                    tt_want_int_op( sv->elem_no, ==, 5 );

                    for( i = 0 ; i < sv->elem_no; ++i )
                    {
                        tt_want_int_op( ( sv->array[ i ].selector_t.i32_value & 1 ), ==, 0 );
                    }

                    xi_vector_destroy( sv );

                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                })

XI_TT_TESTCASE( test_vector_remove_if_UI32,
                {
                    xi_vector_t* sv = xi_vector_create( );

                    int32_t i = 0;
                    for ( ; i < 10; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_UI32( (uint32_t)i ) ) );
                    }

                    xi_vector_remove_if( sv, &vector_is_odd );

                    tt_want_int_op( sv->elem_no, ==, 5 );

                    for( i = 0 ; i < sv->elem_no; ++i )
                    {
                        tt_want_int_op( ( sv->array[ i ].selector_t.ui32_value & 1 ), ==, 0 );
                    }

                    xi_vector_destroy( sv );

                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                })

XI_TT_TESTCASE( test_vector_remove_if_PTR,
                {
                    xi_vector_t* sv = xi_vector_create( );

                    unsigned long i = 0;
                    for ( ; i < 10; ++i )
                    {
                        xi_vector_push( sv, XI_VEC_CONST_VALUE_PARAM( XI_VEC_VALUE_PTR( (void *)i ) ) );
                    }

                    xi_vector_remove_if( sv, &vector_is_odd );

                    tt_want_int_op( sv->elem_no, ==, 5 );

                    for( i = 0 ; i < (unsigned long) sv->elem_no; ++i )
                    {
                        tt_want_int_op( ( (unsigned long) sv->array[ i ].selector_t.ptr_value & 1 ), ==, 0 );
                    }

                    xi_vector_destroy( sv );

                    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
                })

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
