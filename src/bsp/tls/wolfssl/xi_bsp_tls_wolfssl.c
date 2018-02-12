/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <assert.h>
#include <cyassl/ctaocrypt/memory.h>
#include <cyassl/ssl.h>
#include <wolfssl/error-ssl.h>
#include <xi_bsp_tls.h>
#include <xi_bsp_debug.h>

#include <stdio.h>

#define WOLFSSL_DEBUG_LOG 0

typedef struct wolfssl_tls_context_s
{
    CYASSL_CTX* ctx;
    CYASSL* obj;
} wolfssl_tls_context_t;

int xi_wolfssl_recv( CYASSL* ssl, char* buf, int sz, void* context )
{
    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    ( void )ssl;

    // here context is the xi_layer_connectivity_t* on the BSP TLS stub layer
    int bytes_read         = 0;
    xi_bsp_tls_state_t ret = xi_bsp_tls_recv_callback( buf, sz, context, &bytes_read );

    switch ( ret )
    {
        case XI_BSP_TLS_STATE_OK:
            return bytes_read;
        case XI_BSP_TLS_STATE_WANT_READ:
            return CYASSL_CBIO_ERR_WANT_READ;
        default:
            xi_bsp_debug_format( "%s\n", "unexpected state" );
            return CYASSL_CBIO_ERR_GENERAL;
    }
}

int xi_wolfssl_send( CYASSL* ssl, char* buf, int sz, void* context )
{
    ( void )ssl;

    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    // here context is the xi_layer_connectivity_t* on the BSP TLS stub layer
    int bytes_sent         = 0;
    xi_bsp_tls_state_t ret = xi_bsp_tls_send_callback( buf, sz, context, &bytes_sent );

    switch ( ret )
    {
        case XI_BSP_TLS_STATE_OK:
            xi_bsp_debug_format( "bytes_sent=%d", bytes_sent );
            return bytes_sent;
        case XI_BSP_TLS_STATE_WANT_WRITE:
            xi_bsp_debug_logger( "Want write" );
            return CYASSL_CBIO_ERR_WANT_WRITE;
        default:
            xi_bsp_debug_format( "%s", "unexpected state" );
            return CYASSL_CBIO_ERR_GENERAL;
    }
}

xi_bsp_tls_state_t xi_bsp_tls_init( xi_bsp_tls_context_t** tls_context,
                                    xi_bsp_tls_init_params_t* init_params )
{
    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    int ret                                    = 0;
    xi_bsp_tls_state_t result                  = XI_BSP_TLS_STATE_OK;
    wolfssl_tls_context_t* wolfssl_tls_context = NULL;

#if WOLFSSL_DEBUG_LOG
    wolfSSL_Debugging_ON();
#endif

#ifdef XI_TLS_OCSP_STAPLING
    const int nonce_options = 0;
#endif

    ret = CyaSSL_Init();

    if ( ret != SSL_SUCCESS )
    {
        xi_bsp_debug_logger( "failed to initialize CyaSSL library" );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

    ret = CyaSSL_SetAllocators( init_params->fp_xively_alloc, init_params->fp_xively_free,
                                init_params->fp_xively_realloc );

    if ( 0 != ret )
    {
        xi_bsp_debug_logger( "failed to initialize CyaSSL library" );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

    xi_bsp_debug_logger( "initialized CyaSSL library" );

    wolfssl_tls_context =
        ( wolfssl_tls_context_t* )wolfSSL_Malloc( sizeof( wolfssl_tls_context_t ) );

    /* save tls context, this value will be passed back in other BSP TLS functions */
    *tls_context = wolfssl_tls_context;

    wolfssl_tls_context->ctx = CyaSSL_CTX_new( CyaSSLv23_client_method() );

    if ( NULL == wolfssl_tls_context->ctx )
    {
        xi_bsp_debug_logger( "failed to create CyaSSL context" );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

    xi_bsp_debug_logger( "CyaSSL context created" );

    CyaSSL_SetIORecv( wolfssl_tls_context->ctx, xi_wolfssl_recv );
    CyaSSL_SetIOSend( wolfssl_tls_context->ctx, xi_wolfssl_send );

#ifdef XI_DISABLE_CERTVERIFY
    /* disable verify cause no proper certificate */
    CyaSSL_CTX_set_verify( wolfssl_tls_context->ctx, SSL_VERIFY_NONE, 0 );
#endif

    wolfssl_tls_context->obj = CyaSSL_new( wolfssl_tls_context->ctx );

    if ( NULL == wolfssl_tls_context->obj )
    {
        xi_bsp_debug_logger( "failed to create CYASSL object" );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

    xi_bsp_debug_logger( "CyaSSL object created" );

    /* enable domain name check */
    if ( SSL_SUCCESS !=
         CyaSSL_check_domain_name( wolfssl_tls_context->obj, init_params->domain_name ) )
    {
        xi_bsp_debug_format( "failed to set domain name check against host: %s",
                             init_params->domain_name );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

    /* enable SNI */
    /* Note: we are doing this on the object to future proof ourselves.
       in theory this could be set on the context, and any created objects
       created off that context will have the same SNI hostname. This is fine
       for now but in the future we might reuse the context to connect to
       mulitple hosts so it's best to set the specific name on the single use
       object. */
    if ( SSL_SUCCESS != CyaSSL_UseSNI( wolfssl_tls_context->obj, CYASSL_SNI_HOST_NAME,
                                       init_params->domain_name,
                                       strlen( init_params->domain_name ) ) )
    {
        xi_bsp_debug_format( "failed to set SNI Host Name: %s",
                             init_params->domain_name );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

#ifdef XI_TLS_OCSP_STAPLING

    /* OCSP Stapling, expecting stappled OCSP attachment during TLS handshake */

    /* change this next statement to:
           nonce_option = WOLFSSL_CSR_OCSP_USE_NONCE
       once the gateway can support it */
    ret = wolfSSL_UseOCSPStapling( wolfssl_tls_context->obj, WOLFSSL_CSR_OCSP,
                                   nonce_options );
    if ( SSL_SUCCESS != ret )
    {
        xi_bsp_debug_format( "failed to enable OCSP Stapling, reason: %d", ret );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

#elif XI_TLS_OCSP

    /* standard OCSP, separate socket connection to a OCSP responder */

    const int no_options = 0;
    ret                  = wolfSSL_EnableOCSP( wolfssl_tls_context->obj, no_options );
    if ( SSL_SUCCESS != ret )
    {
        xi_bsp_debug_format( "failed to enable OCSP support, reason: %d", ret );
        result = XI_BSP_TLS_STATE_INIT_ERROR;
        goto err_handling;
    }

#else

/* no OCSP */

#endif

    CyaSSL_set_using_nonblock( wolfssl_tls_context->obj, 1 );

    CyaSSL_SetIOReadCtx( wolfssl_tls_context->obj,
                         init_params->xively_io_callback_context );
    CyaSSL_SetIOWriteCtx( wolfssl_tls_context->obj,
                          init_params->xively_io_callback_context );

    /* POST/PRE-CONDITIONS */
    assert( NULL != init_params->ca_cert_pem_buf );
    assert( 0 < init_params->ca_cert_pem_buf_length );

    /* loading the certificate */
    ret = CyaSSL_CTX_load_verify_buffer(
        wolfssl_tls_context->ctx, init_params->ca_cert_pem_buf,
        init_params->ca_cert_pem_buf_length, SSL_FILETYPE_PEM );

    if ( SSL_SUCCESS != ret )
    {
        xi_bsp_debug_format( "failed to load CA certificate, reason: %d", ret );
        result = XI_BSP_TLS_STATE_CERT_ERROR;
        goto err_handling;
    }

err_handling:

    return result;
}

xi_bsp_tls_state_t xi_bsp_tls_connect( xi_bsp_tls_context_t* tls_context )
{
    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    /* get back the wolfssl_tls_context */
    wolfssl_tls_context_t* wolfssl_tls_context = tls_context;

    // CyaSSL_connect
    int ret = CyaSSL_connect( wolfssl_tls_context->obj );
    int cyastate =
        ( ret <= 0 ) ? CyaSSL_get_error( wolfssl_tls_context->obj, ret ) : SSL_SUCCESS;

    switch ( cyastate )
    {
        case SSL_ERROR_WANT_READ:
            return XI_BSP_TLS_STATE_WANT_READ;
        case SSL_ERROR_WANT_WRITE:
            return XI_BSP_TLS_STATE_WANT_WRITE;
        /*case VERSION_ERROR: //-326:
            return XI_BSP_TLS_STATE_CERT_ERROR;*/
        case SSL_SUCCESS:
            return XI_BSP_TLS_STATE_OK;
        default:
#if WOLFSSL_DEBUG_LOG
        {
            char errorString[80] = {'\0'};
            CyaSSL_ERR_error_string( cyastate, errorString );
            xi_bsp_debug_format( "CyaSSL_connect failed reason [%d]: %s\n", cyastate,
                                 errorString );
        }
#endif
        break;
    }

    CyaSSL_UnloadCertsKeys( wolfssl_tls_context->obj );

    return XI_BSP_TLS_STATE_CONNECT_ERROR;
}

xi_bsp_tls_state_t xi_bsp_tls_read( xi_bsp_tls_context_t* tls_context,
                                    uint8_t* data_ptr,
                                    size_t data_size,
                                    int* ret_bytes_read )
{
    assert( NULL != tls_context );
    assert( NULL != data_ptr );
    assert( data_size >= 1 );
    assert( NULL != ret_bytes_read );

    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    int cyastate = 0;
    int ret      = 0;

    /* get back the wolfssl_tls_context */
    wolfssl_tls_context_t* wolfssl_tls_context = tls_context;

    ret = CyaSSL_read( wolfssl_tls_context->obj, data_ptr, data_size );

    if ( ret <= 0 )
    {
        cyastate = CyaSSL_get_error( wolfssl_tls_context->obj, ret );
    }
    else
    {
        cyastate = SSL_SUCCESS;
    }

    /* set the additional return value */
    *ret_bytes_read = ret;

    /* return value transformation */
    if ( SSL_ERROR_WANT_READ == cyastate )
    {
        return XI_BSP_TLS_STATE_WANT_READ;
    }

    if ( SSL_SUCCESS != cyastate )
    {
#if WOLFSSL_DEBUG_LOG
        char errorString[80] = {'\0'};
        CyaSSL_ERR_error_string( cyastate, errorString );
        xi_bsp_debug_format( "CyaSSL_read failed reason: %s", errorString );
#endif

        return XI_BSP_TLS_STATE_READ_ERROR;
    }

    return XI_BSP_TLS_STATE_OK;
}

xi_bsp_tls_state_t xi_bsp_tls_write( xi_bsp_tls_context_t* tls_context,
                                     uint8_t* data_ptr,
                                     size_t data_size,
                                     int* ret_bytes_written )
{
    /* PRE-CONDITION */
    assert( NULL != tls_context );
    assert( NULL != ret_bytes_written );
    assert( NULL != data_ptr );
    assert( data_size >= 1 );

    xi_bsp_debug_format( "[ %s ], size=%zu", __FUNCTION__, data_size );

    int ret      = 0;
    int cyastate = 0;

    /* get back the wolfssl_tls_context */
    wolfssl_tls_context_t* wolfssl_tls_context = tls_context;
    assert( NULL != wolfssl_tls_context->obj );

    /* call TLS library write function */
    ret = CyaSSL_write( wolfssl_tls_context->obj, data_ptr, data_size );

    xi_bsp_debug_format( "return value - %d", ret );

    /* check the result of the write operation */
    if ( ret <= 0 )
    {
        cyastate = CyaSSL_get_error( wolfssl_tls_context->obj, ret );
    }
    else
    {
        xi_bsp_debug_format( "written %d bytes", ret );
        cyastate = SSL_SUCCESS;
    }

    /* set the additional return value */
    *ret_bytes_written = ret;

    /* return value transformation */
    if ( SSL_ERROR_WANT_WRITE == cyastate )
    {
        return XI_BSP_TLS_STATE_WANT_WRITE;
    }

    if ( SSL_SUCCESS != cyastate )
    {
#if WOLFSSL_DEBUG_LOG
        char errorString[80] = {'\0'};
        CyaSSL_ERR_error_string( cyastate, errorString );
        xi_bsp_debug_format( "CyaSSL_write failed reason: %s", errorString );
#endif

        return XI_BSP_TLS_STATE_WRITE_ERROR;
    }

    return XI_BSP_TLS_STATE_OK;
}

xi_bsp_tls_state_t xi_bsp_tls_cleanup( xi_bsp_tls_context_t** tls_context )
{
    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    if ( NULL == tls_context || NULL == *tls_context )
    {
        return XI_BSP_TLS_STATE_OK;
    }

    wolfssl_tls_context_t* wolfssl_tls_context = *tls_context;

    // here unload the possibly remaining cert binary
    CyaSSL_CTX_UnloadCAs( wolfssl_tls_context->ctx );
    CyaSSL_free( wolfssl_tls_context->obj );
    CyaSSL_CTX_free( wolfssl_tls_context->ctx );
    CyaSSL_Cleanup();

    wolfSSL_Free( *tls_context );
    *tls_context = NULL;

    return XI_BSP_TLS_STATE_OK;
}

int xi_bsp_tls_pending( xi_bsp_tls_context_t* tls_context )
{
    xi_bsp_debug_format( "[ %s ]", __FUNCTION__ );

    /* get back the wolfssl_tls_context */
    wolfssl_tls_context_t* wolfssl_tls_context = tls_context;
    assert( NULL != wolfssl_tls_context->obj );

    return CyaSSL_pending( wolfssl_tls_context->obj );
}

#ifdef XI_PROVIDE_WOLFSSL_SEED_GENERATOR
#include "xi_bsp_rng.h"
/* WolfSSL API. This function is set via CUSTOM_RAND_GENERATE_SEED in the makefile */
int xi_bsp_rng_generate_wolfssl_seed( byte* output, word32 sz )
{
    /* TODO: We can use more bytes per random number to accelerate this loop and
    lower the number of queries to the hardware RNGs the BSP may be using */
    word32 i;
    for (i = 0; i < sz; i++ )
    {
        output[i] = ( byte )xi_bsp_rng_get();
    }

    return 0;
}
#endif /* XI_PROVIDE_WOLFSSL_SEED_GENERATOR */

#ifdef XI_PROVIDE_WOLFSSL_XTIME_XGMTIME
#include <time.h> /* For the gmtime() function */
#include "xi_bsp_time.h"
time_t XTIME( time_t* timer )
{
    time_t current_time = xi_bsp_time_getcurrenttime_seconds();
    if ( timer )
    {
        *timer = current_time;
    }
    return current_time;
}

struct tm* XGMTIME( const time_t* timer, struct tm* tmp )
{
    ( void )tmp;
    return gmtime( timer );
}
#endif /* XI_PROVIDE_WOLFSSL_XTIME_XGMTIME */
