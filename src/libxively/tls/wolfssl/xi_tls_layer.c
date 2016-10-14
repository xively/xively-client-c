/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license. 
 */

#include "xi_macros.h"
#include "xi_layer_api.h"
#include "xi_coroutine.h"
#include "xi_event_dispatcher_api.h"
#include "xi_event_thread_dispatcher.h"
#include "xi_globals.h"
#include "xi_common.h"

#include "xi_list.h"

#include "xi_tls_layer.h"
#include "xi_tls_layer_state.h"

#include "xi_fs_filenames.h"
#include "xi_resource_manager.h"

#include "xi_helpers.h"

#include <cyassl/ctaocrypt/memory.h>

#ifndef XI_TLS_BSP

#ifdef __cplusplus
extern "C" {
#endif

static xi_state_t connect_handler( void* context, void* data, xi_state_t state );

static xi_state_t send_handler( void* context, void* data, xi_state_t state );

static xi_state_t recv_handler( void* context, void* data, xi_state_t state );

/* helpers used as cyassl callbacks */
static int cyassl_recv( CYASSL* ssl, char* buf, int sz, void* context )
{
    XI_UNUSED( ssl );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* recvd = layer_data->raw_buffer;

    int len = 0;

    if ( NULL != recvd )
    {
        int recvd_len = recvd->length - recvd->curr_pos;
        assert( recvd_len > 0 );

        len = XI_MIN( sz, recvd_len );
        memcpy( buf, ( recvd->data_ptr + recvd->curr_pos ), len );
        recvd->curr_pos += len;

        if ( recvd->curr_pos == recvd->length )
        {
            xi_data_desc_t* tmp = NULL;

            XI_LIST_POP( xi_data_desc_t, layer_data->raw_buffer, tmp );

            xi_free_desc( &tmp );
        }

        return len;
    }

    /* may happen if the buffer is not yet received
     * in that case cyassl will have to wait till data is
     * there */
    return CYASSL_CBIO_ERR_WANT_READ;
}

static int cyassl_send( CYASSL* ssl, char* buf, int sz, void* context )
{
    XI_UNUSED( ssl );

    xi_state_t state = XI_STATE_OK;

    xi_layer_connectivity_t* ctx = ( xi_layer_connectivity_t* )context;

    XI_UNUSED( ctx );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* buffer_desc = 0;

    XI_CR_START( layer_data->tls_lib_handler_sending_cs );

    assert( layer_data->tls_layer_write_state == XI_TLS_LAYER_DATA_NONE );
    layer_data->tls_layer_write_state = XI_TLS_LAYER_DATA_WRITING;

    buffer_desc = xi_make_desc_from_buffer_share( ( unsigned char* )buf, sz );
    XI_CHECK_MEMORY( buffer_desc, state );

    XI_PROCESS_PUSH_ON_PREV_LAYER( context, buffer_desc, XI_STATE_OK );

    do
    {
        XI_CR_YIELD_UNTIL( layer_data->tls_lib_handler_sending_cs,
                           layer_data->tls_layer_write_state != XI_TLS_LAYER_DATA_WRITTEN,
                           CYASSL_CBIO_ERR_WANT_WRITE );
    } while ( layer_data->tls_layer_write_state != XI_TLS_LAYER_DATA_WRITTEN );

    assert( layer_data->tls_layer_write_state == XI_TLS_LAYER_DATA_WRITTEN );

    layer_data->tls_layer_write_state = XI_TLS_LAYER_DATA_NONE;

    XI_CR_EXIT( layer_data->tls_lib_handler_sending_cs, sz );

    XI_CR_END();

err_handling:
    if ( buffer_desc )
    {
        xi_free_desc( &buffer_desc );
    }
    return CYASSL_CBIO_ERR_GENERAL;
}

static xi_state_t connect_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );
    XI_UNUSED( data );

    /* PRE-CONDITIONS */
    assert( context != 0 );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_format( "connect handler is not operational, layer_data = %p",
                         layer_data );
        return XI_STATE_OK;
    }

    int ret      = 0;
    int cyastate = 0;

    XI_CR_START( layer_data->tls_layer_conn_cs );

    /* make the context */
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
        xi_make_handle( &connect_handler, context, data, in_out_state ),
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
        xi_make_handle( &connect_handler, context, data, in_out_state ), NULL );

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

    ret = CyaSSL_CTX_load_verify_buffer(
        layer_data->cyassl_ctx, layer_data->rm_context->data_buffer->data_ptr,
        layer_data->rm_context->data_buffer->length, SSL_FILETYPE_PEM );

    if ( SSL_SUCCESS != ret )
    {
        xi_debug_format( "failed to load CA certificate, reason: %d", ret );
        in_out_state = XI_TLS_FAILED_LOADING_CERTIFICATE;
        goto err_handling;
    }

    in_out_state = xi_resource_manager_close(
        layer_data->rm_context,
        xi_make_handle( &connect_handler, context, data, in_out_state ), NULL );

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

    xi_debug_logger( "loaded CA certificate" );

    do
    {
        ret = CyaSSL_connect( layer_data->cyassl_obj );

        cyastate =
            ret <= 0 ? CyaSSL_get_error( layer_data->cyassl_obj, ret ) : SSL_SUCCESS;

        XI_CR_YIELD_UNTIL(
            layer_data->tls_layer_conn_cs,
            ( cyastate == SSL_ERROR_WANT_READ || cyastate == SSL_ERROR_WANT_WRITE ),
            ( cyastate == SSL_ERROR_WANT_READ ? XI_STATE_WANT_READ
                                              : XI_STATE_WANT_WRITE ) );

        if ( cyastate != SSL_SUCCESS )
        {
#if XI_DEBUG_OUTPUT
            char errorString[80] = {'\0'};
            CyaSSL_ERR_error_string( cyastate, errorString );
            xi_debug_format( "CyaSSL_connect failed reason [%d]: %s", cyastate,
                             errorString );
#endif
            in_out_state = XI_TLS_CONNECT_ERROR;
            goto err_handling;
        }

    } while ( cyastate != SSL_SUCCESS );

    ret = CyaSSL_CTX_UnloadCAs( layer_data->cyassl_ctx );

    if ( SSL_SUCCESS != ret )
    {
        xi_debug_format( "CyaSSL_CTX_UnloadCAs failed reason: %d", ret );
    }

    /* connection done we can restore the logic handlers */
    layer_data->tls_layer_logic_recv_handler = &recv_handler;
    layer_data->tls_layer_logic_send_handler = &send_handler;

    return XI_PROCESS_CONNECT_ON_NEXT_LAYER( context, data, XI_STATE_OK );

    XI_CR_END();

err_handling:
    ret = CyaSSL_CTX_UnloadCAs( layer_data->cyassl_ctx );

    if ( SSL_SUCCESS != ret )
    {
        xi_debug_format( "CyaSSL_CTX_UnloadCAs failed reason: %d", ret );
    }

    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, 0, in_out_state );
}

static xi_state_t send_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );
    XI_UNUSED( data );

    /* PRECONDITIONS */
    assert( context != 0 );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );
        return XI_STATE_OK;
    }

    int ret      = 0;
    int cyastate = 0;

    size_t offset = layer_data->to_write_buffer->curr_pos;
    size_t size_left =
        layer_data->to_write_buffer->capacity - layer_data->to_write_buffer->curr_pos;

    XI_CR_START( layer_data->tls_layer_send_cs );

    do
    {
        ret = CyaSSL_write( layer_data->cyassl_obj,
                            layer_data->to_write_buffer->data_ptr + offset, size_left );

        if ( ret <= 0 )
        {
            cyastate = CyaSSL_get_error( layer_data->cyassl_obj, ret );
        }
        else
        {
            cyastate = SSL_SUCCESS;
        }

        if ( ret > 0 )
        {
            layer_data->to_write_buffer->curr_pos += ret;
        }

        XI_CR_YIELD_UNTIL(
            layer_data->tls_layer_send_cs,
            ( cyastate == SSL_ERROR_WANT_READ || cyastate == SSL_ERROR_WANT_WRITE ),
            ( cyastate == SSL_ERROR_WANT_READ ? XI_STATE_WANT_READ
                                              : XI_STATE_WANT_WRITE ) );

        if ( cyastate != SSL_SUCCESS )
        {
#if XI_DEBUG_OUTPUT
            char errorString[80] = {'\0'};
            CyaSSL_ERR_error_string( cyastate, errorString );
            xi_debug_format( "CyaSSL_write failed reason: %s", errorString );
#endif
            XI_CR_RESET( layer_data->tls_layer_send_cs );
            goto err_handling;
        }

    } while ( cyastate != SSL_SUCCESS );

    /* free the memory */
    xi_free_desc( &layer_data->to_write_buffer );

    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0, XI_STATE_WRITTEN );

    XI_CR_END();

err_handling:
    /* free the memory */
    if ( NULL != layer_data )
    {
        xi_free_desc( &layer_data->to_write_buffer );
    }

    return XI_PROCESS_PUSH_ON_NEXT_LAYER( context, 0, XI_STATE_FAILED_WRITING );
}

static xi_state_t recv_handler( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( in_out_state );
    XI_UNUSED( data );

    /* PRECONDITIONS */
    assert( context != 0 );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    xi_data_desc_t* data_desc = ( xi_data_desc_t* )data;

    int ret       = 0;
    int cyastate  = 0;
    int size_left = 0;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        xi_debug_logger( "XI_THIS_LAYER_NOT_OPERATIONAL" );
        return XI_STATE_OK;
    }

    /* if we suppose to reuse the buffer */
    if ( XI_STATE_WANT_READ == in_out_state && NULL != data_desc )
    {
        assert( NULL == layer_data->decoded_buffer );

        layer_data->decoded_buffer = data_desc;
        assert( layer_data->decoded_buffer - layer_data->decoded_buffer == 0 );
        layer_data->decoded_buffer->curr_pos = 0;
        layer_data->decoded_buffer->length   = 0;
        memset( layer_data->decoded_buffer->data_ptr, 0, XI_IO_BUFFER_SIZE );
    }

    /* if recv buffer is empty than create one */
    if ( NULL == layer_data->decoded_buffer )
    {
        layer_data->decoded_buffer = xi_make_empty_desc_alloc( XI_IO_BUFFER_SIZE );
        XI_CHECK_MEMORY( layer_data->decoded_buffer, in_out_state );
    }

    XI_CR_START( layer_data->tls_layer_recv_cs );

    do
    {
        size_left =
            layer_data->decoded_buffer->capacity - layer_data->decoded_buffer->length;

        assert( size_left > 0 );

        ret = CyaSSL_read( layer_data->cyassl_obj, layer_data->decoded_buffer->data_ptr +
                                                       layer_data->decoded_buffer->length,
                           size_left );

        cyastate =
            ret <= 0 ? CyaSSL_get_error( layer_data->cyassl_obj, ret ) : SSL_SUCCESS;

        if ( ret > 0 )
        {
            layer_data->decoded_buffer->length += ret;
        }

        XI_CR_YIELD_UNTIL(
            layer_data->tls_layer_recv_cs,
            ( cyastate == SSL_ERROR_WANT_READ || cyastate == SSL_ERROR_WANT_WRITE ),
            ( cyastate == SSL_ERROR_WANT_READ ? XI_STATE_WANT_READ
                                              : XI_STATE_WANT_WRITE ) );

        if ( cyastate != SSL_SUCCESS )
        {
#if XI_DEBUG_OUTPUT
            char errorString[80] = {'\0'};
            CyaSSL_ERR_error_string( cyastate, errorString );
            xi_debug_format( "CyaSSL_recv failed reason: %s", errorString );
#endif
            XI_CR_RESET( layer_data->tls_layer_recv_cs );
            in_out_state = XI_TLS_READ_ERROR;
            goto err_handling;
        }

    } while ( cyastate != SSL_SUCCESS );

#if 0 /* leave it for future use */
    xi_debug_data_logger( "recved", buffer_desc );
#endif

    xi_data_desc_t* ret        = layer_data->decoded_buffer;
    layer_data->decoded_buffer = NULL;

    XI_CR_EXIT( layer_data->tls_layer_recv_cs,
                XI_PROCESS_PULL_ON_NEXT_LAYER( context, ret, XI_STATE_OK ) );

    XI_CR_END();

err_handling:
    if ( layer_data && layer_data->decoded_buffer )
    {
        xi_free_desc( &layer_data->decoded_buffer );
    }

    return XI_PROCESS_CLOSE_ON_THIS_LAYER( context, NULL, in_out_state );
}

xi_state_t xi_tls_layer_push( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

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
        assert( NULL == layer_data->to_write_buffer );
        layer_data->to_write_buffer = buffer;

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

    int ret                          = 0;
    CYASSL_CTX* ctx                  = 0;
    xi_tls_layer_state_t* layer_data = 0;
    CYASSL* obj                      = 0;

    layer_data = ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( layer_data == 0 )
    {
        /* cleaning of not finished requests */
        XI_ALLOC_AT( xi_tls_layer_state_t, XI_THIS_LAYER( context )->user_data,
                     in_out_state );

        layer_data = ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

        assert( layer_data != 0 );
    }

    /* initialize the cyassl library */
    ret = CyaSSL_Init();

    if ( ret != SSL_SUCCESS )
    {
        xi_debug_logger( "failed to initialize CyaSSL library" );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

    ret = CyaSSL_SetAllocators( xi_alloc_ptr, xi_free_ptr, xi_realloc_ptr );

    if ( 0 != ret )
    {
        xi_debug_logger( "failed to initialize CyaSSL library" );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

    xi_debug_logger( "initialized CyaSSL library" );

    ctx = CyaSSL_CTX_new( CyaSSLv23_client_method() );

    if ( NULL == ctx )
    {
        xi_debug_logger( "failed to create CyaSSL context" );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

    xi_debug_logger( "CyaSSL context created" );

    layer_data->cyassl_ctx = ctx;

    /* register the recv and send */
    CyaSSL_SetIORecv( ctx, cyassl_recv );
    CyaSSL_SetIOSend( ctx, cyassl_send );

#ifdef XI_DISABLE_CERTVERIFY
    /* disable verify cause no proper certificate */
    CyaSSL_CTX_set_verify( ctx, SSL_VERIFY_NONE, 0 );
#endif

    // create obj
    obj = CyaSSL_new( ctx );

    if ( NULL == obj )
    {
        xi_debug_logger( "failed to create CYASSL object" );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

    xi_debug_logger( "CyaSSL object created" );

    layer_data->cyassl_obj = obj;

    xi_connection_data_t* connection_data = ( xi_connection_data_t* )data;

    XI_CHECK_CND_DBGMESSAGE( NULL == connection_data, XI_TLS_INITALIZATION_ERROR,
                             in_out_state,
                             "no connection data during TLS initialization" );

    /* enable domain name check */
    if ( CyaSSL_check_domain_name( obj, connection_data->host ) != SSL_SUCCESS )
    {
        xi_debug_format( "failed to set domain name check against host: %s",
                         connection_data->host );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

    /* enable SNI */
    /* Note: we are doing this on the object to future proof ourselves.
       in theory this could be set on the context, and any created objects
       created off that context will have the same SNI hostname. This is fine
       for now but in the future we might reuse the context to connect to
       mulitple hosts so it's best to set the specific name on the single use
       object. */
    if ( SSL_SUCCESS != CyaSSL_UseSNI( obj, CYASSL_SNI_HOST_NAME, connection_data->host,
                                       strlen( connection_data->host ) ) )
    {
        xi_debug_format( "failed to set SNI Host Name: %s", connection_data->host );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

#ifndef NO_OCSP
    const int no_options = 0;
    ret                  = CyaSSL_EnableOCSP( obj, no_options );
    if ( SSL_SUCCESS != ret )
    {
        xi_debug_format( "failed to enable OCSP support, reason: %d", ret );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }

#ifdef OCSP_STAPLING
    /* change this next statement to:
           nonce_option = WOLFSSL_CSR_OCSP_USE_NONCE
       once the gateway can support it */
    const int nonce_options = 0;
    ret = wolfSSL_UseOCSPStapling( obj, WOLFSSL_CSR_OCSP, nonce_options );
    if ( SSL_SUCCESS != ret )
    {
        xi_debug_format( "failed to enable OCSP Stapling, reason: %d", ret );
        in_out_state = XI_TLS_INITALIZATION_ERROR;
        goto err_handling;
    }
#endif /* OCSP_STAPLING */

#endif /* ifndef NO_OCSP */

    /* make the cyassl nonblockable */
    CyaSSL_set_using_nonblock( obj, 1 );

    /* set proper context instead the file descriptor */
    CyaSSL_SetIOReadCtx( obj, context );
    CyaSSL_SetIOWriteCtx( obj, context );

    /* setup the logic handlers for connection purposes */
    layer_data->tls_layer_logic_recv_handler = &connect_handler;
    layer_data->tls_layer_logic_send_handler = &connect_handler;

    /* done! */
    return XI_PROCESS_INIT_ON_PREV_LAYER( context, data, in_out_state );

err_handling:
    if ( NULL != layer_data )
    {
        layer_data->cyassl_obj = 0;
        layer_data->cyassl_ctx = 0;
    }

    CyaSSL_free( obj );
    CyaSSL_CTX_free( ctx );
    CyaSSL_Cleanup();

    return XI_PROCESS_CONNECT_ON_THIS_LAYER( context, data, in_out_state );
}

xi_state_t xi_tls_layer_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    XI_UNUSED( data );

    xi_tls_layer_state_t* layer_data =
        ( xi_tls_layer_state_t* )XI_THIS_LAYER( context )->user_data;

    if ( XI_THIS_LAYER_NOT_OPERATIONAL( context ) || NULL == layer_data )
    {
        /* cleaning of not finished requests */
        goto err_handling;
    }

    if ( XI_STATE_OK != in_out_state )
    {
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

    XI_UNUSED( data );

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

        if ( layer_data->cyassl_obj )
        {
            xi_debug_logger( "freeing CyaSSL object" );
            CyaSSL_free( layer_data->cyassl_obj );
            layer_data->cyassl_obj = 0;
        }

        if ( layer_data->cyassl_ctx )
        {
            xi_debug_logger( "freeing CyaSSL context" );
            CyaSSL_CTX_free( layer_data->cyassl_ctx );
            layer_data->cyassl_ctx = 0;
        }

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

        xi_debug_logger( "cleaning CyaSSL" );
        CyaSSL_Cleanup();

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

#ifdef __cplusplus
}
#endif

#endif /* XI_TLS_BSP */
