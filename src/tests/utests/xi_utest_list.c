/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdlib.h>

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xi_tt_testcase_management.h"
#include "xi_macros.h"
#include "xi_list.h"

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

/**
 * @brief structure used for testing purposes
 */
typedef struct xi_utest_list_s
{
    int value;
    struct xi_utest_list_s* __next;
} xi_utest_list_t;

/* list utest helpers */
static void xi_utest_destroy_list( xi_utest_list_t* list );
static xi_utest_list_t* xi_utest_make_test_list( size_t elem_no );

static xi_utest_list_t* xi_utest_make_test_list( size_t elem_no )
{
    xi_state_t state     = XI_STATE_OK;
    xi_utest_list_t* out = NULL;

    size_t i = 0;
    for ( ; i < elem_no; ++i )
    {
        XI_ALLOC( xi_utest_list_t, elem, state );
        elem->value = ( int )i;
        XI_LIST_PUSH_BACK( xi_utest_list_t, out, elem );
    }

    return out;

err_handling:
    xi_utest_destroy_list( out );
    return ( xi_utest_list_t* )NULL;
}

static void xi_utest_destroy_list( xi_utest_list_t* list )
{
    while ( list )
    {
        xi_utest_list_t* next = list->__next;

        XI_SAFE_FREE( list );

        list = next;
    }
}

static int xi_utest_list_verificator( xi_utest_list_t* list,
                                      int ( *pred )( void*, xi_utest_list_t*, int ) )
{
    size_t i = 0;

    while ( list )
    {
        if ( pred( NULL, list, i++ ) != 1 )
        {
            return 0;
        }
        list = list->__next;
    }

    return i;
}

static int xi_utest_list_number_odd_predicate( void* arg, xi_utest_list_t* elem, int pos )
{
    XI_UNUSED( arg );
    XI_UNUSED( pos );

    return ( elem->value % 2 ) != 0;
}

static int
xi_utest_list_number_even_predicate( void* arg, xi_utest_list_t* elem, int pos )
{
    XI_UNUSED( arg );
    XI_UNUSED( pos );

    return !xi_utest_list_number_odd_predicate( arg, elem, pos );
}

static int
xi_utest_list_number_less_than_10_predicate( void* arg, xi_utest_list_t* elem, int pos )
{
    XI_UNUSED( arg );
    XI_UNUSED( pos );

    return ( elem->value < 10 );
}

static int
xi_utest_list_number_more_than_10_predicate( void* arg, xi_utest_list_t* elem, int pos )
{
    XI_UNUSED( arg );
    XI_UNUSED( pos );

    return ( elem->value >= 10 );
}

#endif

XI_TT_TESTGROUP_BEGIN( utest_list )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_list_split_list_even_and_odd_test,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_utest_list_t* test_list = xi_utest_make_test_list( 10 );
        xi_utest_list_t* odd_list  = NULL;

        tt_ptr_op( test_list, !=, NULL );


        XI_LIST_SPLIT_I( xi_utest_list_t, test_list, xi_utest_list_number_odd_predicate,
                         NULL, odd_list );


        tt_ptr_op( test_list, !=, NULL );
        tt_ptr_op( odd_list, !=, NULL );

        tt_int_op(
            xi_utest_list_verificator( odd_list, &xi_utest_list_number_odd_predicate ),
            ==, 5 );

        tt_int_op(
            xi_utest_list_verificator( test_list, &xi_utest_list_number_even_predicate ),
            ==, 5 );

    end:
        xi_utest_destroy_list( test_list );
        xi_utest_destroy_list( odd_list );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_list_split_list_less_than_10_test_on_10_elements,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_utest_list_t* test_list    = xi_utest_make_test_list( 10 );
        xi_utest_list_t* less_10_list = NULL;

        tt_ptr_op( test_list, !=, NULL );

        XI_LIST_SPLIT_I( xi_utest_list_t, test_list,
                         xi_utest_list_number_less_than_10_predicate, NULL,
                         less_10_list );

        tt_ptr_op( test_list, ==, NULL );
        tt_ptr_op( less_10_list, !=, NULL );

        tt_int_op( xi_utest_list_verificator(
                       less_10_list, &xi_utest_list_number_less_than_10_predicate ),
                   ==, 10 );

    end:
        xi_utest_destroy_list( test_list );
        xi_utest_destroy_list( less_10_list );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_list_split_list_less_than_10_test_on_13_elements,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_utest_list_t* test_list    = xi_utest_make_test_list( 13 );
        xi_utest_list_t* less_10_list = NULL;

        tt_ptr_op( test_list, !=, NULL );

        XI_LIST_SPLIT_I( xi_utest_list_t, test_list,
                         xi_utest_list_number_less_than_10_predicate, NULL,
                         less_10_list );

        tt_ptr_op( test_list, !=, NULL );
        tt_ptr_op( less_10_list, !=, NULL );

        tt_int_op( xi_utest_list_verificator(
                       less_10_list, &xi_utest_list_number_less_than_10_predicate ),
                   ==, 10 );

        tt_int_op( xi_utest_list_verificator(
                       test_list, &xi_utest_list_number_more_than_10_predicate ),
                   ==, 3 );

    end:
        xi_utest_destroy_list( test_list );
        xi_utest_destroy_list( less_10_list );
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_list_split_list_more_than_10_test,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_utest_list_t* test_list    = xi_utest_make_test_list( 13 );
        xi_utest_list_t* more_10_list = NULL;

        tt_ptr_op( test_list, !=, NULL );

        XI_LIST_SPLIT_I( xi_utest_list_t, test_list,
                         xi_utest_list_number_more_than_10_predicate, NULL,
                         more_10_list );

        tt_ptr_op( test_list, !=, NULL );
        tt_ptr_op( more_10_list, !=, NULL );

        tt_int_op( xi_utest_list_verificator(
                       test_list, &xi_utest_list_number_less_than_10_predicate ),
                   ==, 10 );

        tt_int_op( xi_utest_list_verificator(
                       more_10_list, &xi_utest_list_number_more_than_10_predicate ),
                   ==, 3 );

    end:
        xi_utest_destroy_list( test_list );
        xi_utest_destroy_list( more_10_list );
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
