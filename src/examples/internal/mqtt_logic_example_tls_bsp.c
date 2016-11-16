/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xively.h>
#include <xi_helpers.h>
#include <xi_mqtt_message.h>
#include <xi_globals.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <xi_tls_bsp.h>

#include <cyassl/ctaocrypt/types.h>
#include "commandline.h"

static char* test_topic2    = NULL;
static char* shutdown_topic = NULL;
static uint8_t connected    = 0;

#ifndef BSP_DEBUG
#define BSP_DEBUG 1
#endif

#if BSP_DEBUG
#define BSP_ENTER()                                                                      \
    printf( "[XI_TLS_BSP ENTER %s:%d (%s)]\n", __FILE__, __LINE__, __func__ );
#define BSP_EXIT( status )                                                               \
    printf( "[XI_TLS_BSP EXIT %s:%d (%s)] status: %d\n", __FILE__, __LINE__, __func__, \
            status );                                                                    \
    return status;
#else
#define BSP_ENTER()
#define BSP_EXIT( status ) return status;
#endif

const unsigned char tls_cert[1260] = {
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x42, 0x45, 0x47, 0x49, 0x4e, 0x20, 0x43, 0x45, 0x52,
    0x54, 0x49, 0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x0a,
    0x4d, 0x49, 0x49, 0x44, 0x64, 0x54, 0x43, 0x43, 0x41, 0x6c, 0x32, 0x67, 0x41, 0x77,
    0x49, 0x42, 0x41, 0x67, 0x49, 0x4c, 0x42, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x42,
    0x46, 0x55, 0x74, 0x61, 0x77, 0x35, 0x51, 0x77, 0x44, 0x51, 0x59, 0x4a, 0x4b, 0x6f,
    0x5a, 0x49, 0x68, 0x76, 0x63, 0x4e, 0x41, 0x51, 0x45, 0x46, 0x42, 0x51, 0x41, 0x77,
    0x56, 0x7a, 0x45, 0x4c, 0x4d, 0x41, 0x6b, 0x47, 0x0a, 0x41, 0x31, 0x55, 0x45, 0x42,
    0x68, 0x4d, 0x43, 0x51, 0x6b, 0x55, 0x78, 0x47, 0x54, 0x41, 0x58, 0x42, 0x67, 0x4e,
    0x56, 0x42, 0x41, 0x6f, 0x54, 0x45, 0x45, 0x64, 0x73, 0x62, 0x32, 0x4a, 0x68, 0x62,
    0x46, 0x4e, 0x70, 0x5a, 0x32, 0x34, 0x67, 0x62, 0x6e, 0x59, 0x74, 0x63, 0x32, 0x45,
    0x78, 0x45, 0x44, 0x41, 0x4f, 0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x73, 0x54, 0x42,
    0x31, 0x4a, 0x76, 0x0a, 0x62, 0x33, 0x51, 0x67, 0x51, 0x30, 0x45, 0x78, 0x47, 0x7a,
    0x41, 0x5a, 0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x4d, 0x54, 0x45, 0x6b, 0x64, 0x73,
    0x62, 0x32, 0x4a, 0x68, 0x62, 0x46, 0x4e, 0x70, 0x5a, 0x32, 0x34, 0x67, 0x55, 0x6d,
    0x39, 0x76, 0x64, 0x43, 0x42, 0x44, 0x51, 0x54, 0x41, 0x65, 0x46, 0x77, 0x30, 0x35,
    0x4f, 0x44, 0x41, 0x35, 0x4d, 0x44, 0x45, 0x78, 0x4d, 0x6a, 0x41, 0x77, 0x0a, 0x4d,
    0x44, 0x42, 0x61, 0x46, 0x77, 0x30, 0x79, 0x4f, 0x44, 0x41, 0x78, 0x4d, 0x6a, 0x67,
    0x78, 0x4d, 0x6a, 0x41, 0x77, 0x4d, 0x44, 0x42, 0x61, 0x4d, 0x46, 0x63, 0x78, 0x43,
    0x7a, 0x41, 0x4a, 0x42, 0x67, 0x4e, 0x56, 0x42, 0x41, 0x59, 0x54, 0x41, 0x6b, 0x4a,
    0x46, 0x4d, 0x52, 0x6b, 0x77, 0x46, 0x77, 0x59, 0x44, 0x56, 0x51, 0x51, 0x4b, 0x45,
    0x78, 0x42, 0x48, 0x62, 0x47, 0x39, 0x69, 0x0a, 0x59, 0x57, 0x78, 0x54, 0x61, 0x57,
    0x64, 0x75, 0x49, 0x47, 0x35, 0x32, 0x4c, 0x58, 0x4e, 0x68, 0x4d, 0x52, 0x41, 0x77,
    0x44, 0x67, 0x59, 0x44, 0x56, 0x51, 0x51, 0x4c, 0x45, 0x77, 0x64, 0x53, 0x62, 0x32,
    0x39, 0x30, 0x49, 0x45, 0x4e, 0x42, 0x4d, 0x52, 0x73, 0x77, 0x47, 0x51, 0x59, 0x44,
    0x56, 0x51, 0x51, 0x44, 0x45, 0x78, 0x4a, 0x48, 0x62, 0x47, 0x39, 0x69, 0x59, 0x57,
    0x78, 0x54, 0x0a, 0x61, 0x57, 0x64, 0x75, 0x49, 0x46, 0x4a, 0x76, 0x62, 0x33, 0x51,
    0x67, 0x51, 0x30, 0x45, 0x77, 0x67, 0x67, 0x45, 0x69, 0x4d, 0x41, 0x30, 0x47, 0x43,
    0x53, 0x71, 0x47, 0x53, 0x49, 0x62, 0x33, 0x44, 0x51, 0x45, 0x42, 0x41, 0x51, 0x55,
    0x41, 0x41, 0x34, 0x49, 0x42, 0x44, 0x77, 0x41, 0x77, 0x67, 0x67, 0x45, 0x4b, 0x41,
    0x6f, 0x49, 0x42, 0x41, 0x51, 0x44, 0x61, 0x44, 0x75, 0x61, 0x5a, 0x0a, 0x6a, 0x63,
    0x36, 0x6a, 0x34, 0x30, 0x2b, 0x4b, 0x66, 0x76, 0x76, 0x78, 0x69, 0x34, 0x4d, 0x6c,
    0x61, 0x2b, 0x70, 0x49, 0x48, 0x2f, 0x45, 0x71, 0x73, 0x4c, 0x6d, 0x56, 0x45, 0x51,
    0x53, 0x39, 0x38, 0x47, 0x50, 0x52, 0x34, 0x6d, 0x64, 0x6d, 0x7a, 0x78, 0x7a, 0x64,
    0x7a, 0x78, 0x74, 0x49, 0x4b, 0x2b, 0x36, 0x4e, 0x69, 0x59, 0x36, 0x61, 0x72, 0x79,
    0x6d, 0x41, 0x5a, 0x61, 0x76, 0x70, 0x0a, 0x78, 0x79, 0x30, 0x53, 0x79, 0x36, 0x73,
    0x63, 0x54, 0x48, 0x41, 0x48, 0x6f, 0x54, 0x30, 0x4b, 0x4d, 0x4d, 0x30, 0x56, 0x6a,
    0x55, 0x2f, 0x34, 0x33, 0x64, 0x53, 0x4d, 0x55, 0x42, 0x55, 0x63, 0x37, 0x31, 0x44,
    0x75, 0x78, 0x43, 0x37, 0x33, 0x2f, 0x4f, 0x6c, 0x53, 0x38, 0x70, 0x46, 0x39, 0x34,
    0x47, 0x33, 0x56, 0x4e, 0x54, 0x43, 0x4f, 0x58, 0x6b, 0x4e, 0x7a, 0x38, 0x6b, 0x48,
    0x70, 0x0a, 0x31, 0x57, 0x72, 0x6a, 0x73, 0x6f, 0x6b, 0x36, 0x56, 0x6a, 0x6b, 0x34,
    0x62, 0x77, 0x59, 0x38, 0x69, 0x47, 0x6c, 0x62, 0x4b, 0x6b, 0x33, 0x46, 0x70, 0x31,
    0x53, 0x34, 0x62, 0x49, 0x6e, 0x4d, 0x6d, 0x2f, 0x6b, 0x38, 0x79, 0x75, 0x58, 0x39,
    0x69, 0x66, 0x55, 0x53, 0x50, 0x4a, 0x4a, 0x34, 0x6c, 0x74, 0x62, 0x63, 0x64, 0x47,
    0x36, 0x54, 0x52, 0x47, 0x48, 0x52, 0x6a, 0x63, 0x64, 0x47, 0x0a, 0x73, 0x6e, 0x55,
    0x4f, 0x68, 0x75, 0x67, 0x5a, 0x69, 0x74, 0x56, 0x74, 0x62, 0x4e, 0x56, 0x34, 0x46,
    0x70, 0x57, 0x69, 0x36, 0x63, 0x67, 0x4b, 0x4f, 0x4f, 0x76, 0x79, 0x4a, 0x42, 0x4e,
    0x50, 0x63, 0x31, 0x53, 0x54, 0x45, 0x34, 0x55, 0x36, 0x47, 0x37, 0x77, 0x65, 0x4e,
    0x4c, 0x57, 0x4c, 0x42, 0x59, 0x79, 0x35, 0x64, 0x34, 0x75, 0x78, 0x32, 0x78, 0x38,
    0x67, 0x6b, 0x61, 0x73, 0x4a, 0x0a, 0x55, 0x32, 0x36, 0x51, 0x7a, 0x6e, 0x73, 0x33,
    0x64, 0x4c, 0x6c, 0x77, 0x52, 0x35, 0x45, 0x69, 0x55, 0x57, 0x4d, 0x57, 0x65, 0x61,
    0x36, 0x78, 0x72, 0x6b, 0x45, 0x6d, 0x43, 0x4d, 0x67, 0x5a, 0x4b, 0x39, 0x46, 0x47,
    0x71, 0x6b, 0x6a, 0x57, 0x5a, 0x43, 0x72, 0x58, 0x67, 0x7a, 0x54, 0x2f, 0x4c, 0x43,
    0x72, 0x42, 0x62, 0x42, 0x6c, 0x44, 0x53, 0x67, 0x65, 0x46, 0x35, 0x39, 0x4e, 0x38,
    0x0a, 0x39, 0x69, 0x46, 0x6f, 0x37, 0x2b, 0x72, 0x79, 0x55, 0x70, 0x39, 0x2f, 0x6b,
    0x35, 0x44, 0x50, 0x41, 0x67, 0x4d, 0x42, 0x41, 0x41, 0x47, 0x6a, 0x51, 0x6a, 0x42,
    0x41, 0x4d, 0x41, 0x34, 0x47, 0x41, 0x31, 0x55, 0x64, 0x44, 0x77, 0x45, 0x42, 0x2f,
    0x77, 0x51, 0x45, 0x41, 0x77, 0x49, 0x42, 0x42, 0x6a, 0x41, 0x50, 0x42, 0x67, 0x4e,
    0x56, 0x48, 0x52, 0x4d, 0x42, 0x41, 0x66, 0x38, 0x45, 0x0a, 0x42, 0x54, 0x41, 0x44,
    0x41, 0x51, 0x48, 0x2f, 0x4d, 0x42, 0x30, 0x47, 0x41, 0x31, 0x55, 0x64, 0x44, 0x67,
    0x51, 0x57, 0x42, 0x42, 0x52, 0x67, 0x65, 0x32, 0x59, 0x61, 0x52, 0x51, 0x32, 0x58,
    0x79, 0x6f, 0x6c, 0x51, 0x4c, 0x33, 0x30, 0x45, 0x7a, 0x54, 0x53, 0x6f, 0x2f, 0x2f,
    0x7a, 0x39, 0x53, 0x7a, 0x41, 0x4e, 0x42, 0x67, 0x6b, 0x71, 0x68, 0x6b, 0x69, 0x47,
    0x39, 0x77, 0x30, 0x42, 0x0a, 0x41, 0x51, 0x55, 0x46, 0x41, 0x41, 0x4f, 0x43, 0x41,
    0x51, 0x45, 0x41, 0x31, 0x6e, 0x50, 0x6e, 0x66, 0x45, 0x39, 0x32, 0x30, 0x49, 0x32,
    0x2f, 0x37, 0x4c, 0x71, 0x69, 0x76, 0x6a, 0x54, 0x46, 0x4b, 0x44, 0x4b, 0x31, 0x66,
    0x50, 0x78, 0x73, 0x6e, 0x43, 0x77, 0x72, 0x76, 0x51, 0x6d, 0x65, 0x55, 0x37, 0x39,
    0x72, 0x58, 0x71, 0x6f, 0x52, 0x53, 0x4c, 0x62, 0x6c, 0x43, 0x4b, 0x4f, 0x7a, 0x0a,
    0x79, 0x6a, 0x31, 0x68, 0x54, 0x64, 0x4e, 0x47, 0x43, 0x62, 0x4d, 0x2b, 0x77, 0x36,
    0x44, 0x6a, 0x59, 0x31, 0x55, 0x62, 0x38, 0x72, 0x72, 0x76, 0x72, 0x54, 0x6e, 0x68,
    0x51, 0x37, 0x6b, 0x34, 0x6f, 0x2b, 0x59, 0x76, 0x69, 0x69, 0x59, 0x37, 0x37, 0x36,
    0x42, 0x51, 0x56, 0x76, 0x6e, 0x47, 0x43, 0x76, 0x30, 0x34, 0x7a, 0x63, 0x51, 0x4c,
    0x63, 0x46, 0x47, 0x55, 0x6c, 0x35, 0x67, 0x45, 0x0a, 0x33, 0x38, 0x4e, 0x66, 0x6c,
    0x4e, 0x55, 0x56, 0x79, 0x52, 0x52, 0x42, 0x6e, 0x4d, 0x52, 0x64, 0x64, 0x57, 0x51,
    0x56, 0x44, 0x66, 0x39, 0x56, 0x4d, 0x4f, 0x79, 0x47, 0x6a, 0x2f, 0x38, 0x4e, 0x37,
    0x79, 0x79, 0x35, 0x59, 0x30, 0x62, 0x32, 0x71, 0x76, 0x7a, 0x66, 0x76, 0x47, 0x6e,
    0x39, 0x4c, 0x68, 0x4a, 0x49, 0x5a, 0x4a, 0x72, 0x67, 0x6c, 0x66, 0x43, 0x6d, 0x37,
    0x79, 0x6d, 0x50, 0x0a, 0x41, 0x62, 0x45, 0x56, 0x74, 0x51, 0x77, 0x64, 0x70, 0x66,
    0x35, 0x70, 0x4c, 0x47, 0x6b, 0x6b, 0x65, 0x42, 0x36, 0x7a, 0x70, 0x78, 0x78, 0x78,
    0x59, 0x75, 0x37, 0x4b, 0x79, 0x4a, 0x65, 0x73, 0x46, 0x31, 0x32, 0x4b, 0x77, 0x76,
    0x68, 0x48, 0x68, 0x6d, 0x34, 0x71, 0x78, 0x46, 0x59, 0x78, 0x6c, 0x64, 0x42, 0x6e,
    0x69, 0x59, 0x55, 0x72, 0x2b, 0x57, 0x79, 0x6d, 0x58, 0x55, 0x61, 0x64, 0x0a, 0x44,
    0x4b, 0x71, 0x43, 0x35, 0x4a, 0x6c, 0x52, 0x33, 0x58, 0x43, 0x33, 0x32, 0x31, 0x59,
    0x39, 0x59, 0x65, 0x52, 0x71, 0x34, 0x56, 0x7a, 0x57, 0x39, 0x76, 0x34, 0x39, 0x33,
    0x6b, 0x48, 0x4d, 0x42, 0x36, 0x35, 0x6a, 0x55, 0x72, 0x39, 0x54, 0x55, 0x2f, 0x51,
    0x72, 0x36, 0x63, 0x66, 0x39, 0x74, 0x76, 0x65, 0x43, 0x58, 0x34, 0x58, 0x53, 0x51,
    0x52, 0x6a, 0x62, 0x67, 0x62, 0x4d, 0x45, 0x0a, 0x48, 0x4d, 0x55, 0x66, 0x70, 0x49,
    0x42, 0x76, 0x46, 0x53, 0x44, 0x4a, 0x33, 0x67, 0x79, 0x49, 0x43, 0x68, 0x33, 0x57,
    0x5a, 0x6c, 0x58, 0x69, 0x2f, 0x45, 0x6a, 0x4a, 0x4b, 0x53, 0x5a, 0x70, 0x34, 0x41,
    0x3d, 0x3d, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x45, 0x4e, 0x44, 0x20, 0x43, 0x45,
    0x52, 0x54, 0x49, 0x46, 0x49, 0x43, 0x41, 0x54, 0x45, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d};


static int cyassl_recv( CYASSL* ssl, char* buf, int sz, void* context )
{
    XI_UNUSED( ssl );
    BSP_ENTER();

    xi_tls_bsp_status_t result = 0;

    result = tls_bsp_recv_callback( buf, sz, context );

    switch ( result )
    {
        case XI_TLS_BSP_CBIO_ERR_GENERAL:
            result = CYASSL_CBIO_ERR_GENERAL;
            break;
        case XI_TLS_BSP_CBIO_ERR_WANT_READ:
            result = CYASSL_CBIO_ERR_WANT_READ;
            break;
        default:
            // all other results represent the number of bytes recv
            break;
    }

    BSP_EXIT( result );
}

static int cyassl_send( CYASSL* ssl, char* buf, int sz, void* context )
{
    XI_UNUSED( ssl );
    BSP_ENTER();

    xi_tls_bsp_status_t result = 0;

    result = tls_bsp_send_callback( buf, sz, context );

    switch ( result )
    {
        case XI_TLS_BSP_CBIO_ERR_GENERAL:
            result = CYASSL_CBIO_ERR_GENERAL;
            break;
        case XI_TLS_BSP_CBIO_ERR_WANT_WRITE:
            result = CYASSL_CBIO_ERR_WANT_WRITE;
            break;
        default:
            // all other results represent the number of bytes sent
            break;
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_init( struct xi_tls_bsp_t* bsp )
{
    XI_UNUSED( bsp );

    int result = 0;

    BSP_ENTER();

    if ( SSL_SUCCESS != CyaSSL_Init() )
    {
        result = XI_TLS_BSP_FAILED_INIT;
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_cleanup( struct xi_tls_bsp_t* bsp )
{
    XI_UNUSED( bsp );

    BSP_ENTER();

    CyaSSL_Cleanup();

    BSP_EXIT( XI_TLS_BSP_SUCCESS );
}

xi_tls_bsp_status_t
xi_tls_bsp_create_context( struct xi_tls_bsp_t* bsp, xi_tls_context_t* context )
{
    XI_UNUSED( bsp );

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    BSP_ENTER();
    CYASSL_CTX* ctx = CyaSSL_CTX_new( CyaSSLv23_client_method() );
    if ( NULL == ctx )
    {
        result = XI_TLS_BSP_FAILED_CREATE_CONTEXT;
    }
    else
    {
        *context = ctx;

        // register the recv and send
        CyaSSL_SetIORecv( ctx, cyassl_recv );
        CyaSSL_SetIOSend( ctx, cyassl_send );

        int ret = CyaSSL_CTX_load_verify_buffer( ctx, tls_cert, sizeof( tls_cert ),
                                                 SSL_FILETYPE_PEM );

        if ( SSL_SUCCESS != ret )
        {
            xi_debug_logger( "failed to load CA certificate!" );
            result = XI_TLS_BSP_FAILED_TO_LOAD_TLS_CERTIFICATE;
        }
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t
xi_tls_bsp_free_context( struct xi_tls_bsp_t* bsp, xi_tls_context_t context )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    CyaSSL_CTX_free( context );

    BSP_EXIT( XI_TLS_BSP_SUCCESS );
}

xi_tls_bsp_status_t xi_tls_bsp_create_object( struct xi_tls_bsp_t* bsp,
                                              xi_tls_context_t context,
                                              void* xi_layer_context,
                                              xi_tls_object_t* obj )
{
    XI_UNUSED( bsp );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    CYASSL* cyasslobj = NULL;
    cyasslobj         = CyaSSL_new( context );
    if ( NULL == obj )
    {
        result = XI_TLS_BSP_FAILED_CREATE_OBJECT;
    }
    else
    {
        // make the cyassl nonblockable
        CyaSSL_set_using_nonblock( cyasslobj, 1 );

        // set proper context instead the file descriptor
        CyaSSL_SetIOReadCtx( cyasslobj, xi_layer_context );
        CyaSSL_SetIOWriteCtx( cyasslobj, xi_layer_context );
        *obj = cyasslobj;
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_free_object( struct xi_tls_bsp_t* bsp,
                                            xi_tls_context_t context,
                                            xi_tls_object_t obj )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    CyaSSL_free( obj );

    BSP_EXIT( XI_TLS_BSP_SUCCESS );
}

xi_tls_bsp_status_t xi_tls_bsp_connect( struct xi_tls_bsp_t* bsp,
                                        xi_tls_context_t context,
                                        xi_tls_object_t obj,
                                        int* native_errno )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    int cyassl_result = CyaSSL_connect( obj );

    if ( SSL_SUCCESS != cyassl_result )
    {
        *native_errno = CyaSSL_get_error( obj, cyassl_result );
        switch ( *native_errno )
        {
            case SSL_ERROR_WANT_READ:
                result = XI_TLS_BSP_ERROR_WANT_READ;
                break;
            case SSL_ERROR_WANT_WRITE:
                result = XI_TLS_BSP_ERROR_WANT_WRITE;
                break;
            case SSL_FATAL_ERROR:
                result = XI_TLS_BSP_ERROR;
                break;
            default:
                xi_debug_format( "Warning! Unhandled connection error code: %d, "
                                 "defaulting to XI_TLS_BSP_ERROR",
                                 cyassl_result );
                result = XI_TLS_BSP_ERROR;
        }
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_get_pending( struct xi_tls_bsp_t* bsp,
                                            xi_tls_context_t context,
                                            xi_tls_object_t obj,
                                            int* num_bytes )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    *num_bytes = CyaSSL_pending( obj );

    BSP_EXIT( result );
}

xi_tls_bsp_status_t
xi_tls_bsp_create_error_string_buf( struct xi_tls_bsp_t* bsp, char** bytes )
{
    XI_UNUSED( bsp );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    *bytes = malloc( CYASSL_MAX_ERROR_SZ );
    if ( NULL == *bytes )
    {
        result = XI_TLS_BSP_ERROR;
    }
    else
    {
        memset( *bytes, 0, CYASSL_MAX_ERROR_SZ );
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t
xi_tls_bsp_free_error_string_buf( struct xi_tls_bsp_t* bsp, char* bytes )
{
    XI_UNUSED( bsp );

    BSP_ENTER();

    free( bytes );

    BSP_EXIT( XI_TLS_BSP_SUCCESS );
}

xi_tls_bsp_status_t xi_tls_bsp_get_platform_err_string( struct xi_tls_bsp_t* bsp,
                                                        xi_tls_context_t context,
                                                        xi_tls_object_t obj,
                                                        int errno,
                                                        char* buf )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );
    XI_UNUSED( obj );

    BSP_ENTER();

    CyaSSL_ERR_error_string( errno, buf );

    BSP_EXIT( XI_TLS_BSP_SUCCESS );
}

xi_tls_bsp_status_t
xi_tls_bsp_get_error_string_buf_size( struct xi_tls_bsp_t* bsp, int* bytes )
{
    XI_UNUSED( bsp );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;

    *bytes = CYASSL_MAX_ERROR_SZ;

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_write( struct xi_tls_bsp_t* bsp,
                                      xi_tls_context_t context,
                                      xi_tls_object_t obj,
                                      void* buf,
                                      int num_bytes,
                                      int* bytes_written,
                                      int* native_errno )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;
    *native_errno              = SSL_SUCCESS;

    *bytes_written = CyaSSL_write( obj, buf, num_bytes );

    if ( *bytes_written <= 0 )
    {
        *native_errno = CyaSSL_get_error( obj, *bytes_written );

        switch ( *native_errno )
        {
            case SSL_ERROR_WANT_READ:
                result = XI_TLS_BSP_ERROR_WANT_READ;
                break;
            case SSL_ERROR_WANT_WRITE:
                result = XI_TLS_BSP_ERROR_WANT_WRITE;
                break;
            default:
                result = XI_TLS_BSP_ERROR;
                break;
        }
    }

    BSP_EXIT( result );
}

xi_tls_bsp_status_t xi_tls_bsp_read( struct xi_tls_bsp_t* bsp,
                                     xi_tls_context_t context,
                                     xi_tls_object_t obj,
                                     void* buf,
                                     int num_bytes,
                                     int* bytes_read,
                                     int* native_errno )
{
    XI_UNUSED( bsp );
    XI_UNUSED( context );

    BSP_ENTER();

    xi_tls_bsp_status_t result = XI_TLS_BSP_SUCCESS;
    *native_errno              = SSL_SUCCESS;

    *bytes_read = CyaSSL_read( obj, buf, num_bytes );

    if ( *bytes_read <= 0 )
    {
        *native_errno = CyaSSL_get_error( obj, *bytes_read );

        switch ( *native_errno )
        {
            case SSL_ERROR_WANT_READ:
                result = XI_TLS_BSP_ERROR_WANT_READ;
                break;
            case SSL_ERROR_WANT_WRITE:
                result = XI_TLS_BSP_ERROR_WANT_WRITE;
                break;
            default:
                result = XI_TLS_BSP_ERROR;
                break;
        }
    }

    BSP_EXIT( result );
}


xi_tls_bsp_s bsp = {sizeof( xi_tls_bsp_s ),
                    xi_tls_bsp_init,
                    xi_tls_bsp_cleanup,
                    xi_tls_bsp_create_context,
                    xi_tls_bsp_free_context,
                    xi_tls_bsp_create_object,
                    xi_tls_bsp_free_object,
                    xi_tls_bsp_connect,
                    xi_tls_bsp_get_pending,
                    xi_tls_bsp_create_error_string_buf,
                    xi_tls_bsp_free_error_string_buf,
                    xi_tls_bsp_get_platform_err_string,
                    xi_tls_bsp_write,
                    xi_tls_bsp_read};

xi_state_t on_test_message( void* context, void* message, xi_state_t state )
{
    XI_UNUSED( context );
    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            xi_debug_logger( "Subscription failed." );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
            xi_debug_logger( "Subscription successfull" );
            return XI_STATE_OK;
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )message;
            xi_debug_logger( "received message: " );
            // xi_mqtt_message_dump( msg );
            xi_debug_logger( "" );

            fflush( stdout );

            xi_mqtt_message_free( &msg );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

xi_state_t on_disconnected( void* context )
{
    XI_UNUSED( context );
    xi_evtd_stop( xi_globals.evtd_instance );
    return XI_STATE_OK;
}

xi_state_t on_test2_message( void* context, void* message, xi_state_t state )
{
    XI_UNUSED( context );
    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            xi_debug_logger( "Subscription failed." );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
            xi_debug_logger( "Subscription successfull" );
            return XI_STATE_OK;
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )message;
            xi_debug_logger( "received message: " );
            xi_mqtt_message_dump( msg );
            xi_debug_logger( "" );

            fflush( stdout );

            xi_mqtt_message_free( &msg );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

xi_state_t on_shutdown_message( void* context, void* message, xi_state_t state )
{
    XI_UNUSED( context );

    switch ( state )
    {
        case XI_MQTT_SUBSCRIPTION_FAILED:
            xi_debug_logger( "Subscription failed." );
            return XI_STATE_OK;
        case XI_MQTT_SUBSCRIPTION_SUCCESSFULL:
        {
            xi_mqtt_suback_status_t status = ( xi_mqtt_suback_status_t )( intptr_t )data;
            xi_debug_format( "Subscription successfull with QoS %d", status );
            return XI_STATE_OK;
        }
        case XI_STATE_OK:
        {
            xi_mqtt_message_t* msg = ( xi_mqtt_message_t* )message;
            xi_debug_logger( "received shutdown message: " );
            xi_mqtt_message_dump( msg );
            xi_debug_logger( "" );

            xi_mqtt_message_free( &msg );

            xi_shutdown_connection( on_disconnected );

            return XI_STATE_OK;
        }

        default:
            return state;
    }
}

void delayed_publish( xi_context_handle_t context_handle,
                      xi_timed_task_handle timed_task_handle,
                      void* user_data )
{
    XI_UNUSED( context_handle );
    XI_UNUSED( timed_task_handle );
    XI_UNUSED( user_data );

    // sending the connect request
    int i = 0;
    for ( ; i < 5; ++i )
    {
        char msg[128] = {'\0'};
        sprintf( msg, "test msg delayed %d @ %d", i, ( int )time( 0 ) );
        xi_publish_to_topic( publishtopic, msg, XI_MQTT_QOS_AT_LEAST_ONCE );
    }
}

void first_delay_before_publish( xi_context_handle_t context_handle,
                                 xi_timed_task_handle timed_task_handle,
                                 void* user_data )
{
    XI_UNUSED( timed_task_handle );
    XI_UNUSED( user_data );

    delayed_publish( context_handle, XI_INVALID_TIMED_TASK_HANDLE, NULL );
    xi_schedule_timed_task( context_handle, delayed_publish, 1, 1, NULL );
}

xi_state_t on_connected( void* in_context, void* data, xi_state_t state )
{
    XI_UNUSED( data );

    if ( state == XI_STATE_OK )
    {
        printf( "connected!\n" );
    }
    else
    {
        printf( "error while connecting!\n" );
    }

    // sending the connect request
    // xi_publish_to_topic( publishtopic, "test_msg01", XI_MQTT_QOS_AT_LEAST_ONCE );
    // xi_publish_to_topic( publishtopic, "test_msg02", XI_MQTT_QOS_AT_LEAST_ONCE );
    xi_publish_to_topic( publishtopic, "test_msg03", XI_MQTT_QOS_AT_LEAST_ONCE );

    if ( connected == 0 ) // we want to call it only once
    {                     // register delayed publish
        /* You can pass any custom data to your callbacks if you want. */
        void* user_data = NULL;
        xi_schedule_timed_task( in_context, first_delay_before_publish, 3, 0, user_data );
    }

    xi_subscribe_to_topic( publishtopic, &on_test_message );

    if ( test_topic2 != 0 )
    {
        xi_subscribe_to_topic( test_topic2, &on_test2_message );
    }

    connected = 1;

    return XI_STATE_OK;
}

const char* get_unique_device_id()
{
    // this needs to return an identifier unique to this particlar device. For instance,
    // the device's serial number in string form.
    return "";
}


#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_example_tls_bsp_main( int argc, char* argv[] )
#endif
{
    char options[]       = "hu:P:t:";
    int missingparameter = 0;

    printf( "\n%s\nMajor = %d Minor = %d Revision = %d\n", xi_cilent_version_str,
            xi_major, xi_minor, xi_revision );

    /* Parse the argv array for ONLY the options specified in the options string */
    parse( argc, argv, options, sizeof( options ) );
    /* Check to see that the required paramters were all present on the command line */
    if ( NULL == username )
    {
        missingparameter = 1;
        printf( "-u --username is required\n" );
    }
    if ( NULL == password )
    {
        missingparameter = 1;
        printf( "-P --password is required\n" );
    }
    if ( NULL == subscribetopic )
    {
        missingparameter = 1;
        printf( "-t --subscribetopic is required\n" );
    }
    if ( 1 == missingparameter )
    {
        exit( -1 );
    }

    // initialize xi library.
    if ( XI_STATE_OK != xi_create_default_context() )
    {
        printf( " xi failed initialization\n" );
        return -1;
    }

    if ( XI_TLS_BSP_SUCCESS != xls_tls_bsp_set( ( struct xi_tls_bsp_t* )&bsp ) )
    {
        printf( "Could not set bsp structure\n" );
        return -1;
    }

    char* local_shutdown_topic = "shutdown_topic";

    shutdown_topic = local_shutdown_topic;

    xi_connect_with_callback( argv[1], argv[2], 10, 20, on_connected );

    xi_platform_event_loop();

    xi_delete_default_context();

    xi_shutdown();

    return 0;
}
