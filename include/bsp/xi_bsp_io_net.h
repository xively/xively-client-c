/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_BSP_IO_NET_H__
#define __XI_BSP_IO_NET_H__


/**
 * \mainpage Xively Client Board Support Package (BSP)
 *
 * # Welcome
 * This doxygen catalogs the Board Support Package (BSP), an abstracted
 * framework for hosting all of the platform-specific code used by the
 * Xively C Client.
 *
 * Porting engineers should focus most of their work to a custom
 * implementation of these collection of files. The rest of the
 * Xively client sources, such as the event system, mqtt serializer, and
 * callback system, use platform generic C code that should not needs
 * any tailoring to specific devcies.
 *
 * # Out of the Box
 * The Xively C Client includes a POSIX implementation of the BSP
 * which it uses by default on MacOSX, and Linux desktops and devcies.
 * For non POSIX platforms your will need to customize the reference
 * implementation, or begin one from scratch using this documentation
 * as a guide.  More information on the porting process can be found
 * in the Xively C Porting Guide which resides in the main /doc directory
 * of this project.
 *
 * # Browsing the Sources
 * The BSP is segmented into several distinct files, each focused
 * around a particular platform library requirement:
 *  - Networking
 *  - Memory Allocators / Dealloactors
 *  - Randon Number Generator
 *  - Time
 *  - Transport Layer Security (TLS)
 *  - File System (for Secure File Transfer)
 *  - Firmware Updates (for Secure File Transfer)
 *
 * The best place to start would be the NET
 * networking library.
 *
 * Implementations for Time, Rng and Memory might be highly
 * portable and might not need any customization at all.
 *
 * # TLS BSPs
 * The Xively C Client also ships with support for two TLS implementations
 * out of the box.
 *
 * ## &nbsp;WolfSSL <small>(https://www.wolfssl.com)</small>
 * The default make target will download a tagged WolfSSL release from
 * their repository, and build it to link against. Additionally the
 * Xively Client Sources will be configured to build the WolfSSL TLS BSP
 * that resides in: <code>/src/bsp/wolfssl</code>.
 *
 * This should get you up and running quickly but their sources must be
 * licensed for distribution.
 *
 * ## &nbsp;mbedTLS <small>(https://tls.mbed.org)</small>
 * An alternative build envirionment and source configuration exists for mbedTLS.
 * Please see our User Guide and Porting Guide for more information.  Both of
 * these documents reside in the base <code>/doc</code> directory.
 *
 * ## &nbsp;Other Implementations
 * If your platform has a TLS implementation built in, then you
 * can couple it to the Xively C Client in the TLS BSP.  Othewrise
 * the Xively C Client ships with a reference implementation
 * for WolfSSL and/or mbedTLS, both of which must be licensed
 * separately.
 *
 * # Further Reading
 * ### Xively C Client
 * Information on how to use the Xively C Library from the
 * applications perspective can be found in:
 * <ul><li>
 * <a href="../../api/html/index.html">The Xively Cilent doxygen</a></li>
 * <li>The Xively Client User Guide in: <code>/doc/user_guide.md</code></li>
 * </ul>
 *
 * ### Porting Process
 * Documentation on the porting process and more information about
 * the Xively C Client BSP can be found in the Xively C Client Porting Guide located
 * in: <code>/doc/porting_guide.md</code>.
 *
 * \copyright 2003-2018, LogMeIn, Inc.  All rights reserved.
 *
 */

/**
 * @file xi_bsp_io_net.h
 * @brief Xively Client's Board Support Platform (BSP) for Asynchronous Networking
 *
 * This file defines the API of an asynchronous platform specific networking
 * implementation. These are all the functions one should implement on a platform to
 * make Xively Client able to do network communication.
 *
 * These functions are supposed to be implemented in a non-blocking fashion.
 * This means that these function should fit into a coopeartive-multitasking environment.
 *
 * A basic call flow looks like this: create_socket, connect, connection_check,
 * some iterations of read-write operations then close_socket at the end.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef xi_bsp_io_net_state_e
 * @brief Return value of the BSP NET API functions.
 *
 * The implementation reports internal status to Xively Client through these values.
 */
typedef enum xi_bsp_io_net_state_e {
    /** operation finished successfully */
    XI_BSP_IO_NET_STATE_OK = 0,
    /** operation failed */
    XI_BSP_IO_NET_STATE_ERROR = 1,
    /** resource is busy, means: please invoke this function again later */
    XI_BSP_IO_NET_STATE_BUSY = 2,
    /** connection lost during read or write operation */
    XI_BSP_IO_NET_STATE_CONNECTION_RESET = 3,
    /** timeout has appeared during the operation */
    XI_BSP_IO_NET_STATE_TIMEOUT = 4,

} xi_bsp_io_net_state_t;

/**
 * @typedef xi_bsp_socket_t
 * @brief Xively Client BSP NET's socket representation type.
 *
 * The Xively Client BSP NET solution stores platform specific socket representations
 * in a variable of this type. That typed variable will be passed along the BSP's
 * functions to keep track of the socket in question.
 */
typedef intptr_t xi_bsp_socket_t;

/**
 * @typedef xi_bsp_socket_event_s
 * @brief Ties socket with its in/out state required for bsp select call
 *
 * This structure is used by the Xively internal system to track native socket state.The
 * BSP implementation should query native socket states using a native socket call like
 * select() and map those states to the flags in this structure.
 */
typedef struct xi_bsp_socket_events_s
{
    /** platform specific value of socket */
    xi_bsp_socket_t xi_socket;
    /** 1 if socket wants to read 0 otherwise */
    uint8_t in_socket_want_read : 1;
    /** 1 if socket wants to write 0 otherwise */
    uint8_t in_socket_want_write : 1;
    /** 1 if socket wants to know about error 0 otherwise */
    uint8_t in_socket_want_error : 1;
    /** 1 if socket waits to get connected 0 othwerwise */
    uint8_t in_socket_want_connect : 1;
    /** set to 1 if socket can read 0 otherwise */
    uint8_t out_socket_can_read : 1;
    /** set to 1 if socket can write 0 otherwise */
    uint8_t out_socket_can_write : 1;
    /** set to 1 if there was an error on the socket 0 otherwise */
    uint8_t out_socket_error : 1;
    /** set to 1 if the connection process is finished 0 otherwise */
    uint8_t out_socket_connect_finished : 1;
} xi_bsp_socket_events_t;

/**
 * @function
 * @brief Provides a method for the Xively library to query socket states. These states
 * will be used by the Xively Client Library to schedule various read and write operations
 * as the library communicates with the Xively Messaging Gateway.
 *
 *
 * The function is passed an array of socket descriptors as were returned by the
 * xi_bsp_io_net_create_socket call. Each element in the array corresponds to a specific
 * socket, and contains an initialized xi_bsp_socket_state_t structure to be filled out by
 * this BSP implementation.
 *
 * The BSP function should invoke functions in the native socket library to query the
 * state of the socket and fill in the corresponding fields in xi_bsp_socket_state_t for
 * that descriptor.
 *
 * @param [in] socket_events_array an array of sockets and sockets' events
 * @param [in] socket_events_array_size size of the socket_events_array
 * @param [in] timeout used for passive waiting function must not wait longer than the
 * given timeout ( in seconds )
 *
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if select call updated any socket event
 * - XI_BSP_IO_NET_STATE_TIMEOUT - if select call has encountered timeout
 * - XI_BSP_IO_NET_STATE_ERROR - if select call finished with error
 */
xi_bsp_io_net_state_t xi_bsp_io_net_select( xi_bsp_socket_events_t* socket_events_array,
                                            size_t socket_events_array_size,
                                            long timeout_sec /* in seconds */ );

/**
 * @function
 * @brief Creates the non-blocking socket.
 *
 * Creates the platform specific non-blocking socket and stores in it in the
 * function parameter xi_socket_nonblocking.
 *
 * @param [out] xi_socket_nonblocking the platform specific socket representation
 *                                    should be stored in this variable. This value
 *                                    will be passed along further BSP function calls.
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if socket created successfully
 * - XI_BSP_IO_NET_STATE_ERROR - otherwise
 */
xi_bsp_io_net_state_t
xi_bsp_io_net_create_socket( xi_bsp_socket_t* xi_socket_nonblocking );

/**
 * @function
 * @brief Connects the socket to an endpoint defined by the parameters.
 *
 * @param [in] xi_socket_nonblocking the socket which needs to be connected
 *                                   (generated by xi_bsp_io_net_create_socket function)
 * @param [in] host Null terminated IP or FQDN of the host to connect to
 * @param [in] port the port number of the endpoint
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if successfully connected
 * - XI_BSP_IO_NET_STATE_ERROR - otherwise
 */
xi_bsp_io_net_state_t xi_bsp_io_net_connect( xi_bsp_socket_t* xi_socket_nonblocking,
                                             const char* host,
                                             uint16_t port );

/**
 * @function
 * @brief Reports to the Xively Client whether the provided socket is connected.
 *
 * This is called after the 'connect' function. Depending on return value Xively
 * Client starts to use (read/write) this socket or reports a failed connection to
 * the client Application.
 *
 * The two separate functions (connect and connection_check) might be confusing. The
 * asynchronous property of the Xively Client requires the separation of these functions.
 * Actual socket connection is performed in the time between these two function calls,
 * when select will be called on POSIX platforms, and networking tick operations
 * invoked on No-OS devcies.
 *
 * @param [in] xi_socket_nonblocking connection check is performed on this socket
 * @param [in] host Null terminated IP or FQDN of the host to connect to
 * @param [in] port the port number of the endpoint
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if socket is successfully connected
 * - XI_BSP_IO_NET_STATE_ERROR - otherwise
 */
xi_bsp_io_net_state_t
xi_bsp_io_net_connection_check( xi_bsp_socket_t xi_socket_nonblocking,
                                const char* host,
                                uint16_t port );

/**
 * @function
 * @brief Sends data on the socket.
 *
 * The Xively Client calls this function if there is data to send. As the other functions
 * this shouldn't block as well. This function should write as many bytes in a chunk
 * which does not break the normal operation of Xively Client. Keep in mind that as long
 * as this function doesn't return no other event occurs and processed by Xively
 * Client. Out parameter and return value can tell the unfinished state
 * of the send operation. If that occurs this same function will be called again later
 * on with remaining data. Between the subsequent calls the system is able to
 * perform any other actions.
 *
 * @param [in] xi_socket_nonblocking data is sent on this socket
 * @param [out] out_written_count upon return this should contain the number of sent bytes
 * @param [in] buf the data to send
 * @param [in] count number of bytes to send from the buffer. This is the size of
 *                   the buffer.
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if the whole buffer is sent. i.e. #count
 *                            bytes are sent. *out_written_count == count
 * - XI_BSP_IO_NET_STATE_BUSY - if not the whole buffer is sent. This return value tells
 *                              the system to call this function again with remaining
 *                              data. *out_written_count < count. Outgoing prameter
 *                              number of written bytes must be set properly!
 * - XI_BSP_IO_NET_STATE_ERROR - error occurred during the write operation
 */
xi_bsp_io_net_state_t xi_bsp_io_net_write( xi_bsp_socket_t xi_socket_nonblocking,
                                           int* out_written_count,
                                           const uint8_t* buf,
                                           size_t count );
/**
 * @function
 * @brief Reads data from the socket.
 *
 * Xively Client calls this function if data is expected to arrive on the socket. This
 * shouldn't block as well. It is up to the implementation how many bytes to read at
 * once which does not block the operation flow for too much time. If Xively Client
 * requires more data this function will be called again.
 *
 * @param [in] xi_socket_nonblocking data is read from this socket
 * @param [out] out_read_count upon return this should contain the number of read bytes
 * @param [out] buf upon return this buffer should contain the read bytes
 * @param [in] count available capacity of the buffer. Maximum this number of bytes can
 *                   be stored in the buffer.
 * @return
 * - XI_BSP_IO_NET_STATE_OK - some bytes were read from the socket, number of bytes
 *                            should be set properly. 0 < *out_read_count
 * - XI_BSP_IO_NET_STATE_BUSY - if no data is available on the socket currently. This
 *                              will cause Xively Client to revisit this function
 *                              later on. *out_read_count == 0
 * - XI_BSP_IO_NET_STATE_ERROR - error occurred during the read operation
 */
xi_bsp_io_net_state_t xi_bsp_io_net_read( xi_bsp_socket_t xi_socket_nonblocking,
                                          int* out_read_count,
                                          uint8_t* buf,
                                          size_t count );

/**
 * @function
 * @brief Closes the socket.
 *
 * Platform dependent socket close implementation.
 *
 * @param [in] xi_socket_nonblocking the socket to be closed
 * @return
 * - XI_BSP_IO_NET_STATE_OK - if socket closed successfully
 * - XI_BSP_IO_NET_STATE_ERROR - otherwise
 */
xi_bsp_io_net_state_t
xi_bsp_io_net_close_socket( xi_bsp_socket_t* xi_socket_nonblocking );

#ifdef __cplusplus
}
#endif

#endif /* __XI_BSP_IO_NET_H__ */
