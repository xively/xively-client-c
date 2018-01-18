/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_fs_filenames.h"
#include "xi_layer_api.h"
#include "xi_resource_manager.h"
#include <xi_bsp_tls.h>
#include <xi_connection_data.h>
#include <xi_coroutine.h>
#include <xi_debug.h>
#include <xi_list.h>
#include <xi_macros.h>
#include <xi_tls_layer.h>
#include <xi_tls_layer_state.h>

/* Forward declarations */
static xi_state_t send_handler( void* context, void* data, xi_state_t state );
static xi_state_t recv_handler( void* context, void* data, xi_state_t state );

xi_bsp_tls_state_t
xi_bsp_tls_recv_callback( char* buf, int sz, void* context, int* bytes_read )
{
    assert( NULL != buf );
    assert( NULL != context );
    assert( NULL != bytes_read );

    // xi_debug_format( "Entering: %s", __FUNCTION__ );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* recvd = layer_data->raw_buffer;

    /* if there is no buffer in the queue just leave with WANT_READ state */
    if ( NULL != recvd )
    {
        /* calculate how much data left in the buffer and copy as much as it's possible */
        const int recvd_data_length_available = recvd->length - recvd->curr_pos;
        assert( recvd_data_length_available > 0 );

        const int bytes_to_copy = XI_MIN( sz, recvd_data_length_available );
        memcpy( buf, ( recvd->data_ptr + recvd->curr_pos ), bytes_to_copy );
        recvd->curr_pos += bytes_to_copy;

        /* if we'ver emptied the buffer let it go */
        if ( recvd->curr_pos == recvd->length )
        {
            xi_data_desc_t* tmp = NULL;
            XI_LIST_POP( xi_data_desc_t, layer_data->raw_buffer, tmp );
            xi_free_desc( &tmp );
        }

        /* set the return argument value */
        *bytes_read = bytes_to_copy;

        /* success */
        return XI_BSP_TLS_STATE_OK;
    }

    /* may happen if the buffer is not yet received
     * in that case cyassl will have to wait till data is
     * there */
    return XI_BSP_TLS_STATE_WANT_READ;
}

xi_bsp_tls_state_t
xi_bsp_tls_send_callback( char* buf, int sz, void* context, int* bytes_sent )
{
    assert( NULL != buf );
    assert( NULL != context );
    assert( NULL != bytes_sent );

    xi_debug_format( "Entering %s", __FUNCTION__ );

    xi_state_t state = XI_STATE_OK;

    xi_layer_connectivity_t* ctx = ( xi_layer_connectivity_t* )context;

    XI_UNUSED( ctx );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer_desc = NULL;

    /* begin the coroutine scope */
    XI_CR_START( layer_data->tls_lib_handler_sending_cs );

    /* sanity checks on the outside state vs the coroutine state */
    assert( layer_data->tls_layer_write_state == XI_TLS_LAYER_DATA_NONE );
    layer_data->tls_layer_write_state = XI_TLS_LAYER_DATA_WRITING;

    /* create new descriptor, but share the data */
    buffer_desc = xi_make_desc_from_buffer_share( ( unsigned char* )buf, sz );
    XI_CHECK_MEMORY( buffer_desc, state );

    /* call the previous layer push function in order to send the data */
    XI_PROCESS_PUSH_ON_PREV_LAYER( context, buffer_desc, XI_STATE_OK );

    /* keep yielding until the data is sent */
    do
    {
        assert( XI_TLS_LAYER_DATA_NONE != layer_data->tls_layer_write_state );

        XI_CR_YIELD_UNTIL( layer_data->tls_lib_handler_sending_cs,
                           layer_data->tls_layer_write_state != XI_TLS_LAYER_DATA_WRITTEN,
                           XI_BSP_TLS_STATE_WANT_WRITE );
    } while ( layer_data->tls_layer_write_state != XI_TLS_LAYER_DATA_WRITTEN );

    /* sanity check on the outside state */
    assert( layer_data->tls_layer_write_state == XI_TLS_LAYER_DATA_WRITTEN );
    layer_data->tls_layer_write_state = XI_TLS_LAYER_DATA_NONE;

    /* set the return argument value */
    *bytes_sent = sz;

    /* exit the coroutine scope */
    XI_CR_EXIT( layer_data->tls_lib_handler_sending_cs, XI_BSP_TLS_STATE_OK );

    XI_CR_END();

err_handling:
    /* this line will only deallocate the descriptor not the data - since the descriptor
     * is sharing the data */
    xi_free_desc( &buffer_desc );
    return XI_BSP_TLS_STATE_WRITE_ERROR;
}

static xi_state_t connect_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_bsp_tls_state_t bsp_tls_state = XI_BSP_TLS_STATE_CONNECT_ERROR;

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_format( "connect handler is not operational, layer_data = %p",
                         layer_data );
        return XI_STATE_OK;
    }

    /* begin coroutine scope */
    XI_CR_START( layer_data->tls_layer_conn_cs );

    do
    {
        bsp_tls_state = xi_bsp_tls_connect( layer_data->tls_context );

        XI_CR_YIELD_UNTIL( layer_data->tls_layer_conn_cs,
                           ( bsp_tls_state == XI_BSP_TLS_STATE_WANT_READ ||
                             bsp_tls_state == XI_BSP_TLS_STATE_WANT_WRITE ),
                           ( ( bsp_tls_state == XI_BSP_TLS_STATE_WANT_READ )
                                 ? XI_STATE_WANT_READ
                                 : XI_STATE_WANT_WRITE ) );

        if ( XI_BSP_TLS_STATE_OK != bsp_tls_state )
        {
            in_out_state = XI_BSP_TLS_STATE_CERT_ERROR == bsp_tls_state
                               ? XI_TLS_FAILED_CERT_ERROR
                               : XI_TLS_CONNECT_ERROR;
            goto err_handling;
        }
    } while ( bsp_tls_state != XI_BSP_TLS_STATE_OK );

    /* connection done we can restore the logic handlers */
    layer_data->tls_layer_logic_recv_handler = &recv_handler;
    layer_data->tls_layer_logic_send_handler = &send_handler;

    if ( NULL != layer_data->raw_buffer ||
         xi_bsp_tls_pending( layer_data->tls_context ) > 0 )
    {
        xi_debug_logger( "XI_PROCESS_PULL_ON_THIS_LAYER" );
        XI_PROCESS_PULL_ON_THIS_LAYER( context, NULL, XI_STATE_WANT_READ );
    }

    XI_CR_EXIT( layer_data->tls_layer_conn_cs,
                XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, XI_STATE_OK ) );

    /* end of coroutine scope */
    XI_CR_END();

err_handling:
    XI_CR_RESET( layer_data->tls_layer_conn_cs );
    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

static xi_state_t send_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_UNUSED( data );
    XI_UNUSED( in_out_state );

    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    /* PRECONDITIONS */
    assert( NULL != context );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );
        return XI_STATE_OK;
    }

    /* cook temporary data before entering the coroutine scope */
    xi_bsp_tls_state_t ret = XI_BSP_TLS_STATE_WRITE_ERROR;
    int bytes_written      = 0;
    const size_t offset    = layer_data->to_write_buffer->curr_pos;
    const size_t size_left =
        layer_data->to_write_buffer->capacity - layer_data->to_write_buffer->curr_pos;

    /* coroutine scope begins */
    XI_CR_START( layer_data->tls_layer_send_cs );

    /* this loop can be break only by success, error state is being checked inside */
    do
    {
        /* passes data and a size to bsp tls write function */
        ret = xi_bsp_tls_write( layer_data->tls_context,
                                layer_data->to_write_buffer->data_ptr + offset, size_left,
                                &bytes_written );

        if ( bytes_written > 0 )
        {
            layer_data->to_write_buffer->curr_pos += bytes_written;
        }

        /* while bsp tls is unable to read let's exit the coroutine */
        XI_CR_YIELD_UNTIL( layer_data->tls_layer_send_cs,
                           ret == XI_BSP_TLS_STATE_WANT_WRITE, XI_STATE_WANT_WRITE );

        /* here we check for an error */
        if ( XI_BSP_TLS_STATE_OK != ret )
        {
            goto err_handling;
        }

    } while ( ret != XI_BSP_TLS_STATE_OK );

    /* free the memory */
    xi_free_desc( &layer_data->to_write_buffer );

    /* exit the coroutine scope */
    XI_CR_EXIT( layer_data->tls_layer_send_cs,
                XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, XI_STATE_WRITTEN ) );

    XI_CR_END();

err_handling:
    /* coroutine reset */
    XI_CR_RESET( layer_data->tls_layer_send_cs );
    /* free the memory */
    xi_free_desc( &layer_data->to_write_buffer );
    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, NULL, XI_STATE_FAILED_WRITING );
}

static xi_state_t recv_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    ( void )context;
    ( void )data;

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    int size_left          = 0;
    xi_bsp_tls_state_t ret = XI_BSP_TLS_STATE_READ_ERROR;
    int bytes_read         = 0;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );
        return XI_STATE_OK;
    }

    /* if recv buffer is empty than create one */
    if ( NULL == layer_data->decoded_buffer )
    {
        layer_data->decoded_buffer = xi_make_empty_desc_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( layer_data->decoded_buffer, in_out_state );
    }

    /* coroutine scope begin */
    XI_CR_START( layer_data->tls_layer_recv_cs );

    do
    {
        size_left =
            layer_data->decoded_buffer->capacity - layer_data->decoded_buffer->length;

        assert( size_left > 0 );

        bytes_read = 0;
        ret        = xi_bsp_tls_read( layer_data->tls_context,
                               layer_data->decoded_buffer->data_ptr +
                                   layer_data->decoded_buffer->length,
                               size_left, &bytes_read );

        if ( bytes_read > 0 )
        {
            layer_data->decoded_buffer->length += bytes_read;
        }

        XI_CR_YIELD_UNTIL( layer_data->tls_layer_recv_cs,
                           ( ret == XI_BSP_TLS_STATE_WANT_READ ), XI_STATE_WANT_READ );

        if ( ret != XI_BSP_TLS_STATE_OK )
        {
            in_out_state = XI_TLS_READ_ERROR;
            goto err_handling;
        }

    } while ( ret != XI_BSP_TLS_STATE_OK );

#if 0 /* leave it for future use */
    xi_debug_data_logger( "recved", buffer_desc );
#endif

    xi_data_desc_t* ret_buffer = layer_data->decoded_buffer;
    layer_data->decoded_buffer = NULL;

    if ( NULL != layer_data->raw_buffer ||
         xi_bsp_tls_pending( layer_data->tls_context ) > 0 )
    {
        XI_PROCESS_PULL_ON_THIS_LAYER( context, NULL, XI_STATE_WANT_READ );
    }

    XI_CR_EXIT( layer_data->tls_layer_recv_cs,
                XI_PROCESS_PULL_ON_NEXT_LAYER( context, ret_buffer, XI_STATE_OK ) );

    /* coroutine scope ends */
    XI_CR_END();

err_handling:
    XI_CR_RESET( layer_data->tls_layer_recv_cs );

    if ( layer_data && layer_data->decoded_buffer )
    {
        xi_free_desc( &layer_data->decoded_buffer );
    }

    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

xi_state_t xi_tls_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( context );
    XI_UNUSED( data );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer = ( xi_data_desc_t* )data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );

        /* cleaning of not finished requests */
        xi_free_desc( &buffer );

        return XI_STATE_OK;
    }

    if ( in_out_state == XI_STATE_WRITTEN )
    {
        xi_debug_logger( "data written" );

        assert( XI_TLS_LAYER_DATA_WRITING == layer_data->tls_layer_write_state );
        layer_data->tls_layer_write_state = XI_TLS_LAYER_DATA_WRITTEN;

        assert( NULL != layer_data->tls_layer_logic_send_handler );
        return layer_data->tls_layer_logic_send_handler( context, NULL, XI_STATE_OK );
    }

    if ( in_out_state == XI_STATE_OK || in_out_state == XI_STATE_WANT_WRITE )
    {
        xi_debug_logger( "init writing data" );

        xi_debug_format( "writing status %d and coroutine state %d",
                         layer_data->tls_layer_write_state,
                         layer_data->tls_lib_handler_sending_cs );

        /* sanity checks */
        assert( NULL == layer_data->to_write_buffer );
        assert( 0 == XI_CR_IS_RUNNING( layer_data->tls_layer_send_cs ) );
        assert( 0 == XI_CR_IS_RUNNING( layer_data->tls_lib_handler_sending_cs ) );

        layer_data->to_write_buffer = buffer;

        /* sanity check */
        assert( NULL != layer_data->tls_layer_logic_send_handler );
        return layer_data->tls_layer_logic_send_handler( context, NULL, XI_STATE_OK );
    }

    return XI_PROCESS_PUSH_ON_PREV_LAYER( context, data, in_out_state );
}

xi_state_t xi_tls_layer_pull( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* data_desc = ( xi_data_desc_t* )data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );
        goto err_handling;
    }

    /* there is data to read */
    if ( in_out_state == XI_STATE_OK && NULL != data_desc )
    {
        assert( data_desc->length - data_desc->curr_pos > 0 );

        /* assign the raw data so that the cyassl is able to read
         * through handler */
        XI_LIST_PUSH_BACK( xi_data_desc_t, layer_data->raw_buffer, data_desc );
    }

    assert( NULL != layer_data->tls_layer_logic_recv_handler );
    return layer_data->tls_layer_logic_recv_handler( context, data_desc, in_out_state );

err_handling:
    xi_free_desc( &data_desc );
    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

xi_state_t xi_tls_layer_init( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;
    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    XI_CHECK_CND_DBGMESSAGE( NULL == connection_data, XI_TLS_INITALIZATION_ERROR,
                             in_out_state,
                             "no connection data during TLS initialization" );

    /* if coroutine returns layer_data will be set so we have to be prepared */
    if ( NULL == layer_data )
    {
        XI_ALLOC_AT( xi_tls_layer_state_t, layer_data, in_out_state );
        XI_THIS_LAYER( context )->user_data = layer_data;
    }

    /* let's use the connection coroutine state */
    XI_CR_START( layer_data->tls_layer_conn_cs );

    /* make the resource manager context */
    in_out_state = xi_resource_manager_make_context( NULL, &layer_data->rm_context );

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to create a resource manager context, reason: %d",
                         in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    in_out_state = xi_resource_manager_open(
        layer_data->rm_context,
        xi_make_handle( &xi_tls_layer_init, context, data, in_out_state ),
        XI_FS_CERTIFICATE, XI_GLOBAL_CERTIFICATE_FILE_NAME, XI_FS_OPEN_READ, NULL );

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to start open on CA certificate using resource manager "
                         "context, reason: %d",
                         in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    XI_CR_YIELD( layer_data->tls_layer_conn_cs, XI_STATE_OK );

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to open CA certificate from filesystem, reason: %d",
                         in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    in_out_state = xi_resource_manager_read(
        layer_data->rm_context,
        xi_make_handle( &xi_tls_layer_init, context, data, in_out_state ), NULL );


    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format(
            "failed to start read on CA certificate using resource manager, reason: %d",
            in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    /* here the resource manager will start reading the resource content from a choosen
     * filesystem */
    XI_CR_YIELD( layer_data->tls_layer_conn_cs, XI_STATE_OK );
    /* here the resource manager finished reading the resource content from a choosen
     * filesystem */

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to read CA certificate from filesystem, reason: %d",
                         in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    /* POST/PRE-CONDITIONS */
    assert( NULL != layer_data->rm_context->data_buffer->data_ptr );
    assert( 0 < layer_data->rm_context->data_buffer->length );

    { /* initialisation block for bsp tls init function */
        xi_bsp_tls_init_params_t init_params;
        memset( &init_params, 0, sizeof( init_params ) );

        init_params.xively_io_callback_context = context;
        init_params.fp_xively_alloc            = xi_alloc_ptr;
        init_params.fp_xively_calloc           = xi_calloc_ptr;
        init_params.fp_xively_free             = xi_free_ptr;
        init_params.fp_xively_realloc          = xi_realloc_ptr;
        init_params.domain_name                = connection_data->host;
        init_params.ca_cert_pem_buf = layer_data->rm_context->data_buffer->data_ptr;
        init_params.ca_cert_pem_buf_length = layer_data->rm_context->data_buffer->length;

        /* bsp init function call */
        const xi_bsp_tls_state_t bsp_tls_state =
            xi_bsp_tls_init( &layer_data->tls_context, &init_params );

        /*  */
        if ( XI_BSP_TLS_STATE_OK != bsp_tls_state )
        {
            in_out_state = XI_BSP_TLS_STATE_CERT_ERROR == bsp_tls_state
                               ? XI_TLS_FAILED_LOADING_CERTIFICATE
                               : XI_TLS_INITALIZATION_ERROR;
            xi_debug_logger( "ERROR: during BSP TLS initialization" );
            goto err_handling;
        }
    }

    xi_debug_logger( "BSP TLS initialization successfull" );

    in_out_state = xi_resource_manager_close(
        layer_data->rm_context,
        xi_make_handle( &xi_tls_layer_init, context, data, in_out_state ), NULL );

    /* here the resource manger will start the close action */
    XI_CR_YIELD( layer_data->tls_layer_conn_cs, XI_STATE_OK );
    /* here the resource manger finished closing this resource */

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to close the CA certificate resource, reason: %d",
                         in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    in_out_state = xi_resource_manager_free_context( &layer_data->rm_context );

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "failed to free the context memory, reason: %d", in_out_state );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    /* setup the logic handlers for connection purposes */
    layer_data->tls_layer_logic_recv_handler = &connect_handler;
    layer_data->tls_layer_logic_send_handler = &connect_handler;

    xi_debug_logger( "successfully initialized BSP TLS module" );

    XI_CR_EXIT( layer_data->tls_layer_recv_cs,
                XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state ) );

    /* end of coroutine scope */
    XI_CR_END();

err_handling:
    /* whenever error before layer_data created */
    if ( NULL != layer_data )
    {
        XI_CR_RESET( layer_data->tls_layer_conn_cs );
        xi_bsp_tls_cleanup( &layer_data->tls_context );
    }

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_tls_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        /* cleaning of not finished requests */
        goto err_handling;
    }

    if ( XI_STATE_OK != in_out_state )
    {
        xi_debug_format( "error connecting state = %d", in_out_state );
        goto err_handling;
    }

    return connect_handler( context, NULL, XI_STATE_OK );

err_handling:
    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, in_out_state );
}

xi_state_t xi_tls_layer_close( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    const xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    return ( NULL == layer_data )
               ? XI_PROCESS_CLOSE_EXTERNALLY_ON_THIS_LAYER( context, data, in_out_state )
               : XI_PROCESS_CLOSE_ON_PREV_LAYER( context, data, in_out_state );
}

#define RELEASE_DATADESCRIPTOR( ds )                                                     \
    {                                                                                    \
        xi_free_desc( &ds );                                                             \
    }

xi_state_t
xi_tls_layer_close_externally( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( NULL != layer_data )
    {
        if ( layer_data->rm_context )
        {
            if ( layer_data->rm_context->resource_handle >= 0 )
            {
                if ( XI_STATE_OK == xi_resource_manager_close(
                                        layer_data->rm_context,
                                        xi_make_handle( &xi_tls_layer_close_externally,
                                                        context, data, in_out_state ),
                                        NULL ) )
                {
                    return XI_STATE_OK;
                }
            }

            xi_resource_manager_free_context( &layer_data->rm_context );
        }

        xi_debug_logger( "cleaning TLS library" );
        xi_bsp_tls_cleanup( &layer_data->tls_context );

        if ( layer_data->raw_buffer )
        {
            xi_debug_logger( "cleaning received buffer" );

            XI_LIST_FOREACH( xi_data_desc_t, layer_data->raw_buffer,
                             RELEASE_DATADESCRIPTOR );
        }

        if ( layer_data->decoded_buffer )
        {
            xi_debug_logger( "cleaning decoded buffer" );
            xi_free_desc( &layer_data->decoded_buffer );
        }

        if ( layer_data->to_write_buffer )
        {
            xi_debug_logger( "cleaning to write buffer" );
            xi_free_desc( &layer_data->to_write_buffer );
        }

        /* user data removed */
        XI_SAFE_FREE( XI_THIS_LAYER( context )->user_data );
    }

    if ( in_out_state != XI_STATE_OK &&
         XI_THIS_LAYER_STATE( context ) == XI_LAYER_STATE_CONNECTING )
    {
        /* error handling */
        return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
    }

    return XI_PROCESS_CLOSE_EXTERNALLY_ON_NEXT_LAYER( context, data, in_out_state );
}
