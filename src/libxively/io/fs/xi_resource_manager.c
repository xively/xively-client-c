/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_resource_manager.h"
#include "xi_allocator.h"
#include "xi_internals.h"
#include "xi_coroutine.h"
#include "xi_event_dispatcher_api.h"
#include "xi_globals.h"

/* counter for unique filedescriptors required before calling open on resource */
static volatile xi_fd_t xi_resource_manager_counter = -1;

static xi_state_t
xi_resource_manager_get_callback_state( xi_event_handle_t* event_handle )
{
    switch ( event_handle->handle_type )
    {
        case XI_EVENT_HANDLE_ARGC3:
            return event_handle->handlers.h3.a3;
        case XI_EVENT_HANDLE_ARGC4:
            return event_handle->handlers.h4.a3;
        default:
            return XI_INVALID_PARAMETER;
    }
}

static xi_state_t xi_resource_manager_set_callback_state( xi_event_handle_t* event_handle,
                                                          xi_state_t state )
{
    /* get already set state */
    const xi_state_t callback_state =
        xi_resource_manager_get_callback_state( event_handle );

    /* calculate the real ret_state */
    const xi_state_t ret_state =
        ( XI_STATE_OK == callback_state ) ? state : callback_state;

    switch ( event_handle->handle_type )
    {
        case XI_EVENT_HANDLE_ARGC3:
            event_handle->handlers.h3.a3 = ret_state;
            break;
        case XI_EVENT_HANDLE_ARGC4:
            event_handle->handlers.h4.a3 = ret_state;
            break;
        default:
            return XI_INVALID_PARAMETER;
    }

    return XI_STATE_OK;
}


/* to remove code repetition */
#define XI_RESOURCE_MANAGER_YIELD_FROM( cs, evtd_instance, resource_handle, event_type,  \
                                        until_state, callback, state, post_invocation,   \
                                        f, ... )                                         \
    {                                                                                    \
        int8_t res = xi_evtd_register_file_fd( evtd_instance, event_type,                \
                                               resource_handle, callback );              \
                                                                                         \
        XI_CHECK_CND( 0 == res, XI_OUT_OF_MEMORY, state );                               \
    }                                                                                    \
                                                                                         \
    for ( ;; )                                                                           \
    {                                                                                    \
        assert( NULL != f );                                                             \
        state = f( __VA_ARGS__ );                                                        \
                                                                                         \
        post_invocation;                                                                 \
                                                                                         \
        XI_CR_YIELD_UNTIL( cs, state == until_state, until_state );                      \
                                                                                         \
        xi_evtd_unregister_file_fd( evtd_instance, resource_handle );                    \
                                                                                         \
        break;                                                                           \
    }                                                                                    \
                                                                                         \
    XI_CHECK_STATE( state )

/* to remove code repetition */
#define XI_RESOURCE_MANGER_INVOKE_CALLBACK()                                             \
    XI_CHECK_STATE(                                                                      \
        xi_resource_manager_set_callback_state( &ctx->callback, ret_state ) );           \
                                                                                         \
    /* register callback invocation */                                                   \
    XI_CHECK_MEMORY( xi_evtd_execute( xi_globals.evtd_instance, ctx->callback ),         \
                     ret_state );                                                        \
                                                                                         \
    /* dispose the callback handle */                                                    \
    xi_dispose_handle( &ctx->callback )


/* helper function that decrease the counter and keeps it < 0 */
static xi_fd_t xi_resource_manager_get_invalid_unique_fd( void )
{
    xi_fd_t ret = xi_resource_manager_counter;

    xi_resource_manager_counter = xi_resource_manager_counter - 1;

    if ( xi_resource_manager_counter >= 0 )
    {
        xi_resource_manager_counter = -1;
    }

    return ret;
}

xi_state_t
xi_resource_manager_make_context( xi_data_desc_t* data_buffer,
                                  xi_resource_manager_context_t** const context )
{
    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( NULL != *context )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    XI_ALLOC_AT( xi_resource_manager_context_t, *context, state );

    if ( NULL == data_buffer )
    {
        ( *context )->data_buffer =
            NULL; /* read suppose to allocate memory cause it does the stat operation */
        ( *context )->memory_type = XI_MEMORY_TYPE_MANAGED;
    }
    else
    {
        ( *context )->data_buffer = data_buffer;
        ( *context )->memory_type = XI_MEMORY_TYPE_UNMANAGED;
    }

    ( *context )->resource_handle = xi_resource_manager_get_invalid_unique_fd();

    return state;

err_handling:
    XI_SAFE_FREE( *context );
    return state;
}

xi_state_t
xi_resource_manager_free_context( xi_resource_manager_context_t** const context )
{
    if ( NULL != context && NULL != *context )
    {
        /* PRE-CONDITION */
        assert( XI_MEMORY_TYPE_UNKNOWN != ( *context )->memory_type );

        if ( XI_MEMORY_TYPE_MANAGED == ( *context )->memory_type )
        {
            xi_free_desc( &( *context )->data_buffer );
        }

        XI_SAFE_FREE( ( *context ) );
    }

    return XI_STATE_OK;
}

static xi_state_t xi_resource_manager_open_coroutine( void* context,
                                                      void* res_type,
                                                      xi_state_t state,
                                                      void* res_name )
{
    /* PRE-CONDITIONS */
    assert( NULL != context );

    if ( NULL == res_name )
    {
        return XI_INVALID_PARAMETER;
    }

    /* local variables */
    xi_state_t ret_state                    = XI_STATE_OK;
    xi_fs_resource_handle_t resource_handle = xi_fs_init_resource_handle();
    xi_event_handle_t local_handle = xi_make_handle( &xi_resource_manager_open_coroutine,
                                                     context, res_type, state, res_name );

    /* local variables <- parameters */
    xi_resource_manager_context_t* const ctx = context;
    const xi_fs_resource_type_t resource_type =
        ( xi_fs_resource_type_t )( intptr_t )res_type;
    const char* const resource_name = ( char* )res_name;

    XI_CR_START( ctx->cs );

    /* yield from stat_resource */
    XI_RESOURCE_MANAGER_YIELD_FROM(
        ctx->cs, xi_globals.evtd_instance, ctx->resource_handle, XI_EVENT_WANT_READ,
        XI_STATE_WANT_READ, local_handle, ret_state, {/* empty block of code */},
        xi_internals.fs_functions.stat_resource, NULL, resource_type, resource_name,
        &ctx->resource_stat );

    /* yield from open_resource */
    XI_RESOURCE_MANAGER_YIELD_FROM(
        ctx->cs, xi_globals.evtd_instance, ctx->resource_handle, XI_EVENT_WANT_READ,
        XI_STATE_WANT_READ, local_handle, ret_state, {/* empty block of code */},
        xi_internals.fs_functions.open_resource, NULL, resource_type, resource_name,
        ctx->open_flags, &resource_handle );

    /* remember the valid resource handle */
    ctx->resource_handle = resource_handle;

    /* invoke the callback */
    XI_RESOURCE_MANGER_INVOKE_CALLBACK();

    /* end of coroutine */
    XI_CR_EXIT( ctx->cs, ret_state );
    XI_CR_END();

err_handling:;
    XI_CR_RESET( ctx->cs );
    xi_event_handle_t event_handle = ctx->callback;
    xi_dispose_handle( &ctx->callback );
    xi_resource_manager_set_callback_state( &event_handle, ret_state );
    xi_evtd_execute_handle( &event_handle );

    return ret_state;
}

xi_state_t xi_resource_manager_read_coroutine( void* context )
{
    /* PRE-CONDITIONS */
    assert( NULL != context );

    /* local variables */
    xi_state_t ret_state = XI_STATE_OK;
    xi_event_handle_t local_handle =
        xi_make_handle( &xi_resource_manager_read_coroutine, context );
    const uint8_t* buffer = NULL;
    size_t buffer_size    = 0;

    /* local variables <- parameters */
    xi_resource_manager_context_t* const ctx = ( xi_resource_manager_context_t* )context;

    XI_CR_START( ctx->cs );

    /* always reset the offset before further processing */
    ctx->data_offset = 0;

    XI_RESOURCE_MANAGER_YIELD_FROM(
        ctx->cs, xi_globals.evtd_instance, ctx->resource_handle, XI_EVENT_WANT_READ,
        XI_STATE_WANT_READ, local_handle, ret_state,
        { /* this section will be called after each read_resource function invocation */
          ctx->data_offset += buffer_size;

          /* if not the whole file has been read */
          if ( XI_STATE_OK == ret_state &&
               ctx->data_offset != ctx->resource_stat.resource_size )
          {
              ret_state = XI_STATE_WANT_READ;
          }

          if ( NULL == ctx->data_buffer ) /* if buffer was not allocated */
          {
              if ( XI_STATE_OK == ret_state ) /* if all read at once */
              {
                  ctx->data_buffer =
                      xi_make_desc_from_buffer_share( ( uint8_t* )buffer, buffer_size );

                  XI_CHECK_MEMORY( ctx->data_buffer, ret_state );
              }
              else if ( XI_STATE_WANT_READ == ret_state )
              {
                  ctx->data_buffer =
                      xi_make_empty_desc_alloc( ctx->resource_stat.resource_size );

                  XI_CHECK_MEMORY( ctx->data_buffer, ret_state );

                  /* append first chunk */
                  XI_CHECK_STATE( xi_data_desc_append_bytes( ctx->data_buffer, buffer,
                                                             buffer_size ) );
              }
          }
          else if ( XI_STATE_WANT_READ == ret_state ||
                    XI_STATE_OK == ret_state ) /* if continuation */
          {
              /* accumulate */
              XI_CHECK_STATE( xi_data_desc_append_data_resize(
                  ctx->data_buffer, ( char* )buffer, buffer_size ) );
          }

          /* if it's not the whole file than let's keep reading */
          if ( ctx->data_offset < ctx->resource_stat.resource_size )
          {
              ret_state = XI_STATE_WANT_READ;
          }
        },
        xi_internals.fs_functions.read_resource, NULL, ctx->resource_handle,
        ctx->data_offset, &buffer, &buffer_size );

    /* POST-CONDITION */
    assert( NULL != ctx->data_buffer );

    XI_RESOURCE_MANGER_INVOKE_CALLBACK();

    /* end of coroutine */
    XI_CR_EXIT( ctx->cs, ret_state );
    XI_CR_END();

err_handling:;
    xi_event_handle_t event_handle = ctx->callback;
    xi_dispose_handle( &ctx->callback );
    xi_resource_manager_set_callback_state( &event_handle, ret_state );
    xi_evtd_execute_handle( &event_handle );

    return ret_state;
}

xi_state_t xi_resource_manager_close_coroutine( void* context )
{
    /* PRE-CONDITIONS */
    assert( NULL != context );

    /* local variables */
    xi_state_t ret_state = XI_STATE_OK;
    xi_event_handle_t local_handle =
        xi_make_handle( &xi_resource_manager_close_coroutine, context );

    /* local variables <- parameters */
    xi_resource_manager_context_t* const ctx = ( xi_resource_manager_context_t* )context;

    XI_CR_START( ctx->cs );

    XI_RESOURCE_MANAGER_YIELD_FROM(
        ctx->cs, xi_globals.evtd_instance, ctx->resource_handle, XI_EVENT_WANT_READ,
        XI_STATE_WANT_READ, local_handle, ret_state, {/* empty block of code */},
        xi_internals.fs_functions.close_resource, NULL, ctx->resource_handle );

    /* POST-CONDITION */
    assert( NULL != ctx->data_buffer );

    ctx->resource_handle = xi_fs_init_resource_handle();

    XI_RESOURCE_MANGER_INVOKE_CALLBACK();

    /* end of coroutine */
    XI_CR_EXIT( ctx->cs, ret_state );
    XI_CR_END();

err_handling:;
    xi_event_handle_t event_handle = ctx->callback;
    xi_dispose_handle( &ctx->callback );
    xi_resource_manager_set_callback_state( &event_handle, ret_state );
    xi_evtd_execute_handle( &event_handle );

    return ret_state;
}

xi_state_t xi_resource_manager_open( xi_resource_manager_context_t* const context,
                                     xi_event_handle_t callback,
                                     const xi_fs_resource_type_t resource_type,
                                     const char* const resource_name,
                                     const xi_fs_open_flags_t open_flags,
                                     void* fs_context )
{
    XI_UNUSED( fs_context );

    if ( NULL == resource_name || NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( 1 != xi_handle_disposed( &context->callback ) || context->resource_handle >= 0 )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    context->open_flags = open_flags;
    context->callback   = callback;

    XI_CHECK_MEMORY(
        xi_evtd_execute( xi_globals.evtd_instance,
                         xi_make_handle( &xi_resource_manager_open_coroutine,
                                         ( void* )context,
                                         ( void* )( intptr_t )resource_type, XI_STATE_OK,
                                         ( void* )resource_name ) ),
        state );
    return state;

err_handling:
    return state;
}

xi_state_t xi_resource_manager_read( xi_resource_manager_context_t* const context,
                                     xi_event_handle_t callback,
                                     void* fs_context )
{
    XI_UNUSED( fs_context );

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( 1 != xi_handle_disposed( &context->callback ) || context->resource_handle < 0 )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    context->callback = callback;

    XI_CHECK_MEMORY( xi_evtd_execute( xi_globals.evtd_instance,
                                      xi_make_handle( &xi_resource_manager_read_coroutine,
                                                      ( void* )context ) ),
                     state );

    return state;

err_handling:
    return state;
}

xi_state_t xi_resource_manager_close( xi_resource_manager_context_t* const context,
                                      xi_event_handle_t callback,
                                      void* fs_context )
{
    XI_UNUSED( fs_context );

    if ( NULL == context )
    {
        return XI_INVALID_PARAMETER;
    }

    if ( 1 != xi_handle_disposed( &context->callback ) || context->resource_handle < 0 )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t state = XI_STATE_OK;

    context->callback = callback;

    XI_CHECK_MEMORY(
        xi_evtd_execute(
            xi_globals.evtd_instance,
            xi_make_handle( &xi_resource_manager_close_coroutine, ( void* )context ) ),
        state );

    return state;

err_handling:
    return state;
}
