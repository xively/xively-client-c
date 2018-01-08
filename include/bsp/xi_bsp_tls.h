/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_TLS_H__
#define __XI_BSP_TLS_H__

/**
 * @file xi_bsp_tls.h
 * @brief Xively Client's Board Support Platform (BSP) for Transport Layer Security (TLS)
 *
 * This file defines the API of a TLS implementation that the Xively Client would use to
 * secure its connection to the Xively Message Broker. In order to build this secure
 * connection you will need to implement these functions with the TLS library that you
 * plan to use on your platform. A few implementations of the TLS BSP have been provided
 * in source form for your reference.
 *
 * These functions are supposed to be implemented in a non-blocking fashion.
 * This means that these function should fit into a coopeartive-multitasking environment.
 * Some functions like xi_bsp_tls_connect can be called multiple times. Those functions
 * must be prepared for receiving WANT_READ and WANT_WRITE states.
 */

#include <stddef.h>
#include <stdint.h>

/**
 * @typedef xi_bsp_io_net_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to Xively Client through these values.
 */
typedef enum xi_bsp_tls_state_e {
    /** operation finished successfully */
    XI_BSP_TLS_STATE_OK = 0,
    /** init operation failed */
    XI_BSP_TLS_STATE_INIT_ERROR = 1,
    /** ca certification validation failed */
    XI_BSP_TLS_STATE_CERT_ERROR = 2,
    /** error during connecting operation */
    XI_BSP_TLS_STATE_CONNECT_ERROR = 3,
    /** io is busy wait for data */
    XI_BSP_TLS_STATE_WANT_READ = 4,
    /** io buffer is full wait for signal */
    XI_BSP_TLS_STATE_WANT_WRITE = 5,
    /** error during reading operation */
    XI_BSP_TLS_STATE_READ_ERROR = 6,
    /** error during writing operation */
    XI_BSP_TLS_STATE_WRITE_ERROR = 7,
} xi_bsp_tls_state_t;

/**
 * @typedef xi_bsp_tls_init_params_t
 * @brief Xively Client BSP TLS init function parameters.
 *
 * Contains information required during initialisation of TLS functionality.
 */
typedef struct xi_bsp_tls_init_params_s
{
    /** context variable required by xi_bsp_tls_send_callback and xi_bsp_tls_recv_callback
     * functions */
    void* xively_io_callback_context;

    /** pointer to a buffer containing the CA certificate in PEM format */
    uint8_t* ca_cert_pem_buf;
    /** length of the buffer containing certificate */
    size_t ca_cert_pem_buf_length;

    /** may be used for memory tracking and limitation if TLS library handles setting
     * custom allocation */
    void* ( *fp_xively_alloc )( size_t );
    void* ( *fp_xively_calloc )( size_t, size_t );
    void* ( *fp_xively_realloc )( void*, size_t );
    void ( *fp_xively_free )( void* );

    /** pointer to a NULL terminated string with domain name - required for domain name
     * check and SNI */
    const char* domain_name;

} xi_bsp_tls_init_params_t;

/**
 * @typedef xi_bsp_tls_context_t
 * @brief Xively Client BSP TLS's context representation type
 *
 * The Xively Client BSP TLS solution can store context of a TLS library in this abstract
 * form. Each BSP TLS function receives it later on as a parameter. The Xively Client is
 * unaware of the actual content and structure thus does not read or write this context.
 */
typedef void xi_bsp_tls_context_t;

/**
 * @function
 * @brief Provides a method for the Xively library to initialise a TLS library.
 *
 * Initialises the TLS library and retuns its context through a tls_context parameter.
 *
 * This function will be called before other BSP TLS functions. The implementation should
 * return XI_BSP_TLS_STATE_OK in case of a success or XI_BSP_TLS_STATE_INIT_ERROR in case
 * of a failure.
 *
 * Content of the init_params will be destroyed after the xi_bsp_tls_init exits. Any data
 * required from init_params to live outside the scope of this initialization function
 * must be copied and stored by your implementation.
 *
 * Xively Client library I/O implementation requires TLS library to register custom send
 * and recv functions. These custom send and recv function then must call
 * xi_bsp_tls_recv_callback and xi_bsp_tls_send_callback in order for the data to be sent
 * or received correctly throught the Xively Client's I/O system. For more details please
 * refer to our reference implementations for WolfSSL and MBEDTLS libraries.
 *
 * @param [out] tls_context pointer to a pointer to a xi_bsp_tls_context_t
 * @param [in] init_params data required for TLS library initialisation
 * @return
 *  - XI_BSP_TLS_STATE_OK in case of success
 *  - XI_BSP_TLS_STATE_INIT_ERROR otherwise
 */
xi_bsp_tls_state_t xi_bsp_tls_init( xi_bsp_tls_context_t** tls_context,
                                    xi_bsp_tls_init_params_t* init_params );
/**
 * @function
 * @brief Provides a method for the Xively library to clean the TLS library.
 *
 * Must deallocate the TLS library's previously allocated resources. Must clean content of
 * the *tls_context.
 *
 * @param [in|out] tls_context
 * @return XI_BSP_TLS_STATE_OK
 */
xi_bsp_tls_state_t xi_bsp_tls_cleanup( xi_bsp_tls_context_t** tls_context );

/**
 * @function
 * @brief Implements the TLS connect functionality.
 *
 * This function may be called several times by Xively Client library before the handshake
 * is completed. This function shouldn't block it should just return
 * XI_BSP_TLS_STATE_WANT_WRITE or XI_BSP_TLS_STATE_WANT_READ instead.
 *
 * This function will send or receive data internally during it's execution so it will
 * result in calling TLS library send and recv functions.
 *
 * In case of any error this function must return XI_BSP_TLS_STATE_CONNECT_ERROR.
 *
 * @param [in] tls_context
 * @return
 *  - XI_BSP_TLS_STATE_OK in case of succesfully finished hanshake
 *  - XI_BSP_TLS_STATE_WANT_READ | XI_BSP_TLS_STATE_WANT_WRITE in case handshake requires
 * to receive or sent more data in order to continue
 *  - XI_BSP_TLS_STATE_CERT_ERROR
 *  - XI_BSP_TLS_STATE_CONNECT_ERROR in case of failure
 */
xi_bsp_tls_state_t xi_bsp_tls_connect( xi_bsp_tls_context_t* tls_context );

/**
 * @function
 * @brief Implements the TLS read.
 *
 * Implementation of this function should call TLSs library read function. It should also
 * translate the result returned by TLS library read function to one of:
 * - XI_BSP_TLS_STATE_OK - whenever the read operation finished succesfully
 * - XI_BSP_TLS_STATE_WANT_READ - whenver the read operation requires more data
 * - XI_BSP_TLS_STATE_READ_ERROR - whenver any other error occured during the read
 * operation
 *
 * Argument bytes_read should be set to a number of bytes actually read.
 *
 * @param [in] tls_context - context of the TLS library
 * @param [in] data_ptr - ptr to a buffer pleaceholder for a received data
 * @param [in] data_size - size of the buffer
 * @param [out] bytes_read - number of bytes read via the TLS library read function
 * @return
 * - XI_BSP_TLS_STATE_OK
 * - XI_BSP_TLS_STATE_WANT_READ
 * - XI_BSP_TLS_STATE_READ_ERROR
 */
xi_bsp_tls_state_t xi_bsp_tls_read( xi_bsp_tls_context_t* tls_context,
                                    uint8_t* data_ptr,
                                    size_t data_size,
                                    int* bytes_read );

/**
 * @function
 * @brief Implements the TLS pending.
 *
 * Implementation of this function should call TLSs library pending function.
 * It should return the the number of bytes that are available to be read by the tls
 * library's read function.
 *
 * @param [in] tls_context - context of the TLS library
 * @return number of bytes available to read
 */
int xi_bsp_tls_pending( xi_bsp_tls_context_t* tls_context );

/**
 * @function
 * @brief Implements the TLS write.
 *
 * Implementation of this function should call TLSs library write function. It should also
 * translate the result returned by TLS library write function to one of:
 * - XI_BSP_TLS_STATE_OK - whenever the operation finished succesfully
 * - XI_BSP_TLS_STATE_WANT_WRITE - whenver the write operation wants to send more data
 * - XI_BSP_TLS_STATE_READ_ERROR - whenver any other error occured during the write
 * operation
 *
 * Argument bytes_written should be set to a number of bytes actually written.
 *
 * @param [in] tls_context - context of the TLS library
 * @param [in] data_ptr - ptr to a buffer with data to be sent
 * @param [in] data_size - size of the buffer
 * @param [out] bytes_written - number of bytes written via the TLS library write function
 * @return
 * - XI_BSP_TLS_STATE_OK
 * - XI_BSP_TLS_STATE_WANT_WRITE
 * - XI_BSP_TLS_STATE_READ_ERROR
 */
xi_bsp_tls_state_t xi_bsp_tls_write( xi_bsp_tls_context_t* tls_context,
                                     uint8_t* data_ptr,
                                     size_t data_size,
                                     int* bytes_written );

/* These are implemented by Xively C Client. These shouldn't be implemented by any BSP TLS
 * solutions but called whenever TLS library wants read or write data. */
xi_bsp_tls_state_t
xi_bsp_tls_recv_callback( char* buf, int sz, void* context, int* bytes_sent );
xi_bsp_tls_state_t
xi_bsp_tls_send_callback( char* buf, int sz, void* context, int* bytes_sent );

#endif /* __XI_BSP_TLS_H__ */
