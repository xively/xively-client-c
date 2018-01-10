/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef XI_ITEST_HELPERS_H
#define XI_ITEST_HELPERS_H

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>

#include <xi_mqtt_logic_layer.h>
#include <xi_mqtt_logic_layer_data.h>
#include <xi_event_handle.h>
#include <xi_layers_ids.h>
#include <xi_macros.h>
#include <xively.h>
#include <xi_types.h>
#include <cmocka.h> // IAR EWARM limitation: comocka.h must come after all headers that may reference stdlib.h


struct CMGroupTest
{
    struct CMUnitTest* tests;
    const char* name;
    int len;
    CMFixtureFunction group_setup;
    CMFixtureFunction group_teardown;
};

#define cmocka_test_group( group )                                                       \
    {                                                                                    \
        group, #group, XI_ARRAYSIZE( group ), NULL, NULL                                 \
    }

#define cmocka_test_group_end                                                            \
    {                                                                                    \
        NULL, NULL, 0, NULL, NULL                                                        \
    }

/**
 * @brief The CMP_TYPE enum
 * Type of the comparation
 */
enum CMP_TYPE
{
    CMP_TYPE_EQUAL,
    CMP_TYPE_NOT_EQUAL
};

enum PROC_TYPE
{
    PROC_TYPE_DONT,
    PROC_TYPE_DO
};

/**
 * @brief The xi_mock_cmp struct
 *
 * Helper struct for passing the conditions to check, this
 * way it's possible to reuse the same wrapper in a multiple situations.
 *
 */
typedef struct xi_mock_cmp_s
{
    void* lvalue;
    enum CMP_TYPE cmp_type;
    void* rvalue;
} xi_mock_cmp_t;

/**
 * type that represents function that will be passed to test various of states
 * of layers and contexts
 * will make the code to be reusable and easily extendable
 */
typedef void( test_state_fun_t )( void* context, void* data, xi_state_t in_out_state );

/**
 * macro for safe evaluation of test state function
 */
#define test_state_evaluate( fun, context, data, in_out_state )                          \
    if ( ( fun ) )                                                                       \
    {                                                                                    \
        ( *( fun ) )( context, data, in_out_state );                                     \
    }

/**
 * @brief cmocka_run_test_groups
 *
 * helper function that enables running series of test groups by cmocka
 */
extern int cmocka_run_test_groups( const struct CMGroupTest* const tgroups );

/**
 * @brief xi_itest_ptr_cmp
 * @param cmp
 *
 * Checks if the passed condition is fulfield
 */
extern void xi_itest_ptr_cmp( xi_mock_cmp_t* cmp );

/**
 * @brief xi_itest_find_layer
 *
 * helper function that allows to find a pointer to a layer on the layer's
 * stack so that it can be used to exchange pointers etc.
 *
 * @param xi_context
 * @param layer_id
 * @return
 */
extern xi_layer_t* xi_itest_find_layer( xi_context_t* xi_context, int layer_id );

/**
 * @brief xi_itest_inject_wraps
 *
 * helper that allows to inject wrappers into the layer system so it's possible
 * to inject all functions or only one parameters may be equall to NULL which
 *means
 * do not change that pointer.
 *
 */
extern void xi_itest_inject_wraps( xi_context_t* xi_context,
                                   int layer_id,
                                   xi_layer_func_t* push,
                                   xi_layer_func_t* pull,
                                   xi_layer_func_t* close,
                                   xi_layer_func_t* close_externally,
                                   xi_layer_func_t* init,
                                   xi_layer_func_t* connect );

/**
 * @brief xi_create_context_with_custom_layers
 *
 * hidden API function that allows us to initialize context with custom
 * layer setup
 *
 * @param context
 * @param layer_config
 * @param layer_chain
 * @param layer_chain_size
 * @return
 */
extern xi_state_t xi_create_context_with_custom_layers( xi_context_t** context,
                                                        xi_layer_type_t layer_config[],
                                                        xi_layer_type_id_t layer_chain[],
                                                        size_t layer_chain_size );

/**
 * @brief xi_delete_context_with_custom_layers
 *
 * hidden API function that allows us to delete context with custom
 * layer setup
 *
 * @param context
 * @param layer_config
 * @return
 */
extern xi_state_t xi_delete_context_with_custom_layers( xi_context_t** context,
                                                        xi_layer_type_t layer_config[],
                                                        size_t layer_chain_size );

/**
 * macros for extracting next, and prev layer user data - this will be replaced
 * by the lookup function
 */
#define XI_PREV_LAYER( context ) ( ( xi_layer_connectivity_t* )context )->prev
#define XI_NEXT_LAYER( context ) ( ( xi_layer_connectivity_t* )context )->next

#endif // XI_ITEST_HELPERS_H
