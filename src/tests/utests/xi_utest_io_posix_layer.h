
#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"
#include "xi_utest_basic_testcase_frame.h"

#include "xi_layer_default_functions.h"
#include "xi_io_posix_layer.h"
#include "xi_io_posix_layer_state.h"
#include "xi_layer_macros.h"
#include "xi_handle.h"
#include "xi_memory_checks.h"
#include "xively.h"
#include "xi_globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
/* this value will be incremented via the next_connect function
   it's value will be used in order to test if the connect has been
    called with proper parameters */
volatile int xi_utest_io_posix_test_value = 0;

/* this value is set via the next_connect function in order to test received xi_state */
volatile xi_state_t xi_test_io_posix_next_conn_state = XI_STATE_OK;

/* let's put the declaration of the functions to gain access to them */
extern xi_state_t xi_create_context_with_custom_layers( xi_context_t** xi_context,
                                                        xi_layer_type_t layer_config[],
                                                        xi_layer_type_id_t layer_chain[],
                                                        size_t layer_chain_size );

extern xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                        xi_layer_type_t layer_config[],
                                                        size_t layer_chain_size );

/* mocked version of the next layer connect */
xi_state_t
xi_mock_layer_io_posix_next_connect( void* context, void* data, xi_state_t state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( state );

    xi_utest_io_posix_test_value += 3;
    xi_test_io_posix_next_conn_state = state;

    return state;
}

enum xi_utest_io_posix_layer_stack_order_e
{
    XI_LAYER_TYPE_SUT_IO = 0,
    XI_LAYER_TYPE_MOCK_IO_NEXT
};

#define XI_UTEST_POSIX_LAYER_CHAIN_DEFAULT                                               \
    XI_LAYER_TYPE_SUT_IO                                                                 \
    , XI_LAYER_TYPE_MOCK_IO_NEXT

XI_DECLARE_LAYER_CHAIN_SCHEME( XI_UTEST_IO_POSIX_LAYER_CHAIN,
                               XI_UTEST_POSIX_LAYER_CHAIN_DEFAULT );

XI_DECLARE_LAYER_TYPES_BEGIN( utest_io_posix_layer_types )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_SUT_IO,
                    &xi_io_posix_layer_push,
                    &xi_io_posix_layer_pull,
                    &xi_io_posix_layer_close,
                    &xi_io_posix_layer_close_externally,
                    &xi_io_posix_layer_init,
                    &xi_io_posix_layer_connect,
                    &xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_IO_NEXT,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      &xi_mock_layer_io_posix_next_connect,
                      NULL)
XI_DECLARE_LAYER_TYPES_END()

#endif

XI_TT_TESTGROUP_BEGIN( utest_io_posix_layer )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_io_connect__error_layer_state,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t local_state = XI_STATE_OK;

        /* create context */
        xi_context_t* xi_context                   = NULL;
        xi_context_handle_t xi_context_handle      = XI_INVALID_CONTEXT_HANDLE;
        int err_state                              = 1;
        xi_io_posix_layer_state_t posix_layer_data = {-1, 0, 0};
        xi_layer_t* layer                          = NULL;
        xi_layer_connectivity_t* layer_context     = NULL;

        local_state = xi_create_context_with_custom_layers(
            &xi_context, utest_io_posix_layer_types, XI_UTEST_IO_POSIX_LAYER_CHAIN,
            XI_UTEST_IO_POSIX_LAYER_CHAINSIZE_SUFFIX );

        XI_CHECK_STATE( local_state );

        assert( xi_context != NULL );

        local_state = xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                 xi_context, &xi_context_handle );

        XI_CHECK_STATE( local_state );

        assert( xi_context_handle != XI_INVALID_CONTEXT_HANDLE );

        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        /* before the test begin we need to init the data layer*/
        layer            = xi_context->layer_chain.bottom;
        layer->user_data = &posix_layer_data;

        /* let's extract our layer context */
        layer_context = &layer->layer_connection;

        for ( ; err_state < XI_ERROR_COUNT; ++err_state )
        {
            /* let's set the test value to 0 */
            xi_utest_io_posix_test_value = 0;

            /* let's run the connect function with the wrong parameters */
            xi_io_posix_layer_connect( layer_context, NULL, ( xi_state_t )err_state );

            tt_int_op( xi_utest_io_posix_test_value, ==, 0 );
            xi_evtd_single_step( xi_globals.evtd_instance, 20 );
            tt_int_op( xi_utest_io_posix_test_value, ==, 3 );
            tt_int_op( xi_test_io_posix_next_conn_state, ==, ( xi_state_t )err_state );
        }

    end:
    err_handling:
        if ( local_state != XI_STATE_OK )
        {
            xi_debug_printf( "local state is not ok!" );
        }

        /* free context */
        if ( xi_context_handle != XI_INVALID_CONTEXT_HANDLE )
        {
            xi_delete_context_with_custom_layers(
                &xi_context, utest_io_posix_layer_types,
                XI_UTEST_IO_POSIX_LAYER_CHAINSIZE_SUFFIX );
        }
    } )

XI_TT_TESTCASE_WITH_SETUP(
    utest__xi_io_connect__layer_data_null,
    xi_utest_setup_basic,
    xi_utest_teardown_basic,
    NULL,
    {
        xi_state_t local_state = XI_STATE_OK;

        /* create context */
        xi_context_t* xi_context               = NULL;
        xi_context_handle_t xi_context_handle  = XI_INVALID_CONTEXT_HANDLE;
        xi_layer_t* layer                      = NULL;
        xi_layer_connectivity_t* layer_context = NULL;

        local_state = xi_create_context_with_custom_layers(
            &xi_context, utest_io_posix_layer_types, XI_UTEST_IO_POSIX_LAYER_CHAIN,
            XI_UTEST_IO_POSIX_LAYER_CHAINSIZE_SUFFIX );

        XI_CHECK_STATE( local_state );

        assert( xi_context != NULL );

        local_state = xi_find_handle_for_object( xi_globals.context_handles_vector,
                                                 xi_context, &xi_context_handle );

        XI_CHECK_STATE( local_state );

        assert( xi_context_handle != XI_INVALID_CONTEXT_HANDLE );

        if ( XI_INVALID_CONTEXT_HANDLE >= xi_context_handle )
        {
            tt_fail_msg( "Failed to create default context!" );
            return;
        }

        /* before the test begin we need to init the data layer*/
        layer            = xi_context->layer_chain.bottom;
        layer->user_data = NULL;

        /* let's extract our layer context */
        layer_context = &layer->layer_connection;

        /* let's set the test value to 0 */
        xi_utest_io_posix_test_value = 0;

        /* let's run the connect function with the right arguments */
        xi_io_posix_layer_connect( layer_context, NULL, XI_STATE_OK );

        tt_int_op( xi_utest_io_posix_test_value, ==, 0 );
        xi_evtd_single_step( xi_globals.evtd_instance, 20 );
        tt_int_op( xi_utest_io_posix_test_value, ==, 3 );
        tt_int_op( xi_test_io_posix_next_conn_state, ==, XI_INTERNAL_ERROR );

    end:
    err_handling:
        if ( local_state != XI_STATE_OK )
        {
            xi_debug_printf( "local state is not ok!" );
        }

        /* free context */
        if ( xi_context_handle != XI_INVALID_CONTEXT_HANDLE )
        {
            xi_delete_context_with_custom_layers(
                &xi_context, utest_io_posix_layer_types,
                XI_UTEST_IO_POSIX_LAYER_CHAINSIZE_SUFFIX );
        }
    } )

XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
