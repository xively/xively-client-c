/* Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 * This is part of Xively C library. */

#ifndef __XI_TLS_BSP_LAYER_H__
#define __XI_TLS_BSP_LAYER_H__

#include <stdint.h>

#define XI_TLS_BSP_SUCCESS 0
#define XI_TLS_BSP_NO_STRUCT 1
#define XI_TLS_BSP_ERROR 2
#define XI_TLS_BSP_FAILED_INIT 3
#define XI_TLS_BSP_FAILED_CREATE_CONTEXT 4
#define XI_TLS_BSP_FAILED_CREATE_OBJECT 5
#define XI_TLS_BSP_FAILED_TO_LOAD_TLS_CERTIFICATE 6
#define XI_TLS_BSP_ERROR_WANT_READ 7
#define XI_TLS_BSP_ERROR_WANT_WRITE 8
#define XI_TLS_BSP_ERROR_BUFFER_TOO_SMALL 9

#define XI_TLS_BSP_CBIO_ERR_GENERAL -1
#define XI_TLS_BSP_CBIO_ERR_WANT_READ -2
#define XI_TLS_BSP_CBIO_ERR_WANT_WRITE -3


#ifdef __cplusplus
extern "C" {
#endif

typedef int xi_tls_bsp_status_t;
typedef void* xi_tls_context_t;
typedef void* xi_tls_object_t;

int tls_bsp_recv_callback( char* buf, int sz, void* context );
int tls_bsp_send_callback( char* buf, int sz, void* context );


typedef struct xi_tls_bsp_t
{
    uint32_t version;

    xi_tls_bsp_status_t ( *xi_tls_bsp_init )( struct xi_tls_bsp_t* bsp );
    xi_tls_bsp_status_t ( *xi_tls_bsp_cleanup )( struct xi_tls_bsp_t* bsp );

    xi_tls_bsp_status_t ( *xi_tls_bsp_create_context )( struct xi_tls_bsp_t* bsp,
                                                        xi_tls_context_t* context );
    xi_tls_bsp_status_t ( *xi_tls_bsp_free_context )( struct xi_tls_bsp_t* bsp,
                                                      xi_tls_context_t context );

    xi_tls_bsp_status_t ( *xi_tls_bsp_create_object )( struct xi_tls_bsp_t* bsp,
                                                       xi_tls_context_t context,
                                                       void* xi_layer_context,
                                                       xi_tls_object_t* obj );
    xi_tls_bsp_status_t ( *xi_tls_bsp_free_object )( struct xi_tls_bsp_t* bsp,
                                                     xi_tls_context_t context,
                                                     xi_tls_object_t obj );

    xi_tls_bsp_status_t ( *xi_tls_bsp_connect )( struct xi_tls_bsp_t* bsp,
                                                 xi_tls_context_t context,
                                                 xi_tls_object_t obj,
                                                 int* native_errno );
    xi_tls_bsp_status_t ( *xi_tls_bsp_get_pending )( struct xi_tls_bsp_t* bsp,
                                                     xi_tls_context_t context,
                                                     xi_tls_object_t obj,
                                                     int* num_bytes );

    xi_tls_bsp_status_t ( *xi_tls_bsp_create_error_string_buf )( struct xi_tls_bsp_t* bsp,
                                                                 char** bytes );
    xi_tls_bsp_status_t ( *xi_tls_bsp_free_error_string_buf )( struct xi_tls_bsp_t* bsp,
                                                               char* bytes );

    xi_tls_bsp_status_t ( *xi_tls_bsp_get_platform_err_string )( struct xi_tls_bsp_t* bsp,
                                                                 xi_tls_context_t context,
                                                                 xi_tls_object_t obj,
                                                                 int errno,
                                                                 char* buf );

    xi_tls_bsp_status_t ( *xi_tls_bsp_write )( struct xi_tls_bsp_t* bsp,
                                               xi_tls_context_t context,
                                               xi_tls_object_t obj,
                                               void* buf,
                                               int num_bytes,
                                               int* bytes_written,
                                               int* native_errno );
    xi_tls_bsp_status_t ( *xi_tls_bsp_read )( struct xi_tls_bsp_t* bsp,
                                              xi_tls_context_t context,
                                              xi_tls_object_t obj,
                                              void* buf,
                                              int num_bytes,
                                              int* byte_read,
                                              int* native_errno );

} xi_tls_bsp_s;

xi_tls_bsp_status_t xls_tls_bsp_set( struct xi_tls_bsp_t* bsp );


#ifdef __cplusplus
}
#endif

#endif
