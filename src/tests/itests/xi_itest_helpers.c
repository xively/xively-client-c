/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_helpers.h"

int cmocka_run_test_groups( const struct CMGroupTest* const tgroups )
{
    int ret = 0;

    int i = 0;
    while ( tgroups[i].tests )
    {
        ret = _cmocka_run_group_tests( tgroups[i].name, tgroups[i].tests, tgroups[i].len,
                                       NULL, NULL );

        if ( ret != 0 )
        {
            return ret;
        }

        ++i;
    }

    return ret;
}

void xi_itest_ptr_cmp( xi_mock_cmp_t* cmp )
{
    switch ( cmp->cmp_type )
    {
        case CMP_TYPE_EQUAL:
            assert_ptr_equal( cmp->lvalue, cmp->rvalue );
            break;
        case CMP_TYPE_NOT_EQUAL:
            assert_ptr_not_equal( cmp->lvalue, cmp->rvalue );
            break;
    }
}

xi_layer_t* xi_itest_find_layer( xi_context_t* xi_context, int layer_id )
{
    xi_layer_t* root = xi_context->layer_chain.top;

    while ( root )
    {
        if ( root->layer_type_id == layer_id )
        {
            return root;
        }

        root = root->layer_connection.prev;
    }

    return root;
}

#define xi_itest_inject_wrap( layer, wrap_name, wrap )                                   \
    if ( wrap )                                                                          \
    {                                                                                    \
        layer->layer_funcs->wrap_name = wrap;                                            \
    }

void xi_itest_inject_wraps( xi_context_t* xi_context,
                            int layer_id,
                            xi_layer_func_t* push,
                            xi_layer_func_t* pull,
                            xi_layer_func_t* close,
                            xi_layer_func_t* close_externally,
                            xi_layer_func_t* init,
                            xi_layer_func_t* connect )
{
    // first find proper layer
    xi_layer_t* layer = xi_itest_find_layer( xi_context, layer_id );

    if ( layer )
    {
        xi_itest_inject_wrap( layer, push, push );
        xi_itest_inject_wrap( layer, pull, pull );
        xi_itest_inject_wrap( layer, close, close );
        xi_itest_inject_wrap( layer, close_externally, close_externally );
        xi_itest_inject_wrap( layer, init, init );
        xi_itest_inject_wrap( layer, connect, connect );
    }
    else
    {
        fail_msg( "Could not find layer type: %d\n", layer_id );
    }
}
