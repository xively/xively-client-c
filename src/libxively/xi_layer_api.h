/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAYER_API_H__
#define __XI_LAYER_API_H__

#include "xi_config.h"
#include "xi_layer.h"
#include "xi_event_dispatcher_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XI_THIS_LAYER( context ) ( ( xi_layer_connectivity_t* )context )->self
#define XI_THIS_LAYER_STATE( context ) XI_THIS_LAYER( context )->layer_state
#define XI_THIS_LAYER_NOT_OPERATIONAL( context )                                         \
    ( XI_THIS_LAYER_STATE( context ) == XI_LAYER_STATE_CLOSING ||                        \
      XI_THIS_LAYER_STATE( context ) == XI_LAYER_STATE_CLOSED )
#define XI_CONTEXT_DATA( context ) XI_THIS_LAYER( context )->context_data
#define XI_THIS_LAYER_STATE_UPDATE( context, state )                                     \
    XI_THIS_LAYER_STATE( context ) = state

#if XI_DEBUG_EXTRA_INFO
#define XI_LAYERS_CONNECT( lp_i, ln_i )                                                  \
    ln_i->layer_connection.prev         = lp_i;                                          \
    lp_i->layer_connection.next         = ln_i;                                          \
    lp_i->debug_info.debug_line_connect = __LINE__;                                      \
    lp_i->debug_info.debug_file_connect = __FILE__;                                      \
    ln_i->debug_info.debug_line_connect = __LINE__;                                      \
    ln_i->debug_info.debug_file_connect = __FILE__;
#else
#define XI_LAYERS_CONNECT( lp_i, ln_i )                                                  \
    ln_i->layer_connection.prev = lp_i;                                                  \
    lp_i->layer_connection.next = ln_i
#endif

#if XI_DEBUG_EXTRA_INFO
#define XI_LAYERS_DISCONNECT( lp_i, ln_i )                                               \
    ln_i->layer_connection.prev         = 0;                                             \
    lp_i->layer_connection.next         = 0;                                             \
    lp_i->debug_info.debug_line_connect = __LINE__;                                      \
    lp_i->debug_info.debug_file_connect = __FILE__;                                      \
    ln_i->debug_info.debug_line_connect = __LINE__;                                      \
    ln_i->debug_info.debug_file_connect = __FILE__;
#else
#define XI_LAYERS_DISCONNECT( lp_i, ln_i )                                               \
    ln_i->layer_connection.prev = 0;                                                     \
    lp_i->layer_connection.next = 0
#endif

#if XI_DEBUG_EXTRA_INFO
extern xi_state_t xi_layer_continue_with_impl( xi_layer_func_t* f,
                                               xi_layer_connectivity_t* from_context,
                                               xi_layer_connectivity_t* context,
                                               void* data,
                                               xi_state_t state,
                                               const char* file_name,
                                               const int line_no );

#define XI_LAYER_CONTINUE_WITH( layer, target, context, data, state )                    \
    xi_layer_continue_with_impl(                                                         \
        ( ( NULL == context->layer_connection.layer )                                    \
              ? ( NULL )                                                                 \
              : ( context->layer_connection.layer->layer_funcs->target ) ),              \
        &context->layer_connection,                                                      \
        ( ( NULL == context->layer_connection.layer )                                    \
              ? ( NULL )                                                                 \
              : ( &context->layer_connection.layer->layer_connection ) ),                \
        data, state, __FILE__, __LINE__ )
#else /* XI_DEBUG_EXTRA_INFO */
extern xi_state_t xi_layer_continue_with_impl( xi_layer_func_t* f,
                                               xi_layer_connectivity_t* from_context,
                                               xi_layer_connectivity_t* context,
                                               void* data,
                                               xi_state_t state );

#define XI_LAYER_CONTINUE_WITH( layer, target, context, data, state )                    \
    xi_layer_continue_with_impl(                                                         \
        ( ( NULL == context->layer_connection.layer )                                    \
              ? ( NULL )                                                                 \
              : ( context->layer_connection.layer->layer_funcs->target ) ),              \
        &context->layer_connection,                                                      \
        ( ( NULL == context->layer_connection.layer )                                    \
              ? ( NULL )                                                                 \
              : ( &context->layer_connection.layer->layer_connection ) ),                \
        data, state )

#endif /* XI_DEBUG_EXTRA_INFO */

/* ON_DEMAND */
#define XI_PROCESS_PUSH_ON_THIS_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( self, push, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_PUSH_ON_NEXT_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( next, push, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( prev, push, XI_THIS_LAYER( context ), data, state )

/* ON_PUSHING */
#define XI_PROCESS_PULL_ON_THIS_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( self, pull, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_PULL_ON_NEXT_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( next, pull, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_PULL_ON_PREV_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( prev, pull, XI_THIS_LAYER( context ), data, state )

/* CLOSING */
#define XI_PROCESS_CLOSE_ON_THIS_LAYER( context, data, state )                           \
    XI_LAYER_CONTINUE_WITH( self, close, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_CLOSE_ON_NEXT_LAYER( context, data, state )                           \
    XI_LAYER_CONTINUE_WITH( next, close, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, state )                           \
    XI_LAYER_CONTINUE_WITH( prev, close, XI_THIS_LAYER( context ), data, state )

/* CLOSING_EXTERNALLY */
#define XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, state )                \
    XI_LAYER_CONTINUE_WITH( self, close_externally, XI_THIS_LAYER( context ), data,      \
                            state )

#define XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, state )                \
    XI_LAYER_CONTINUE_WITH( next, close_externally, XI_THIS_LAYER( context ), data,      \
                            state )

#define XI_PROCESS_CLOSE_EXTERNALLY_ON_PREV_LAYER( context, data, state )                \
    XI_LAYER_CONTINUE_WITH( prev, close_externally, XI_THIS_LAYER( context ), data,      \
                            state )

/* INIT */
#define XI_PROCESS_INIT_ON_THIS_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( self, init, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_INIT_ON_NEXT_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( next, init, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_INIT_ON_PREV_LAYER( context, data, state )                            \
    XI_LAYER_CONTINUE_WITH( prev, init, XI_THIS_LAYER( context ), data, state )

/* CONNECT */
#define XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, state )                         \
    XI_LAYER_CONTINUE_WITH( self, connect, XI_THIS_LAYER( context ), data, state );

#define XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, state )                         \
    XI_LAYER_CONTINUE_WITH( next, connect, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_CONNECT_ON_PREV_LAYER( context, data, state )                         \
    XI_LAYER_CONTINUE_WITH( prev, connect, XI_THIS_LAYER( context ), data, state )

/* POST-CONNECT */
#define XI_PROCESS_POST_CONNECT_ON_THIS_LAYER( context, data, state )                    \
    XI_LAYER_CONTINUE_WITH( self, post_connect, XI_THIS_LAYER( context ), data, state );

#define XI_PROCESS_POST_CONNECT_ON_NEXT_LAYER( context, data, state )                    \
    XI_LAYER_CONTINUE_WITH( next, post_connect, XI_THIS_LAYER( context ), data, state )

#define XI_PROCESS_POST_CONNECT_ON_PREV_LAYER( context, data, state )                    \
    XI_LAYER_CONTINUE_WITH( prev, post_connect, XI_THIS_LAYER( context ), data, state )

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAYER_API_H__ */
