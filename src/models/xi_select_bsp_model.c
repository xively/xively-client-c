/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/**
 * This is proposal of the solution for bsp select implementation. It's mainly focused on
 * making the user side practicaly simple to use and it may impact the library side by
 * requiring more memory in terms of creating the abstraction over the platform side and
 * by providing security by copying some data partially like sockets fd etc.
 * I'll try to make pin-point these places so that we can think if we need those and if we
 * can peel down the cost of handling
 * the bsp select.
 */

#include <xi_bsp_io_net.h>
#include <xi_time.h>

#define MAX( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )

/* helper function for setting flag on value */
int xi_bsp_set_bitflag( int* value, const int flag )
{
    *value |= flag;
    return *value;
}

/* helper function for unsetting flag from value */
int xi_bsp_unset_bitflag( int* value, const int flag )
{
    *value &= ~flag;
    return *value;
}

/* helper function for testing if the bitflag isset */
int xi_bsp_isset_bitflag( const int value, const int flag )
{
    return ( ( value & flag ) == flag ) ? ( 1 ) : ( 0 );
}

/* BSP SELECT STRUCTURE/DATA SECTION */
/* This part of this file contains structures and types that are revealed on the USER side
 */

/**
 * @enum xi_bsp_socket_event_flags_t
 * @brief represents possible flags of requirements and results that library may require
 * to pass and receive to/from bsp select call.
 */
typedef enum {
    XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ    = 1 << 0,
    XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE   = 1 << 1,
    XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR   = 1 << 2,
    XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT = 1 << 3,
} xi_bsp_socket_status_flags_t;

/**
 * @enum xi_bsp_select_socket_descriptor_t
 * @brief represents the data passed to the user
 */
typedef struct xi_bsp_socket_descriptor_s
{
    xi_bsp_socket_t socket;         /*!< this is in directed variable */
    int in_out_socket_status_flags; /*!< in out variable represents which state socket is
                                       awaiting or ready for */
} xi_bsp_socket_descriptor_t;

int xi_bsp_check_socket_want_read( const xi_bsp_socket_descriptor_t* descriptor )
{
    return xi_bsp_isset_bitflag( descriptor->in_out_socket_status_flags,
                                 XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ );
}

int xi_bsp_does_socket_want_write( const xi_bsp_socket_descriptor_t* descriptor )
{
    return xi_bsp_isset_bitflag( descriptor->in_out_socket_status_flags,
                                 XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE );
}

int xi_bsp_does_socket_want_error( const xi_bsp_socket_descriptor_t* descriptor )
{
    return xi_bsp_isset_bitflag( descriptor->in_out_socket_status_flags,
                                 XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR );
}

int xi_bsp_does_socket_want_connect( const xi_bsp_socket_descriptor_t* descriptor )
{
    return xi_bsp_isset_bitflag( descriptor->in_out_socket_status_flags,
                                 XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT );
}

void xi_bsp_set_socket_can_read( xi_bsp_socket_descriptor_t* descriptor )
{
    return xi_bsp_unset_bitflag( &descriptor->in_out_socket_status_flags,
                                 XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ );
}

void xi_bsp_set_socket_can_write( xi_bsp_socket_descriptor_t* descriptor )
{
}

void xi_bsp_set_socket_error( xi_bsp_socket_descriptor_t* descriptor )
{
}

void xi_bsp_set_socket_is_connected( xi_bsp_socket_descriptor_t* descriptor )
{
}

/* BSP SELECT USER/PLATFORM IMPLEMENTATION SECTION */
/* POSIX VERSION */
/* This part of the file contains information about the example usage of the
 * xi_bsp_socket_select function on the USER side */

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/**
 * @brief xi_bsp_socket_status_update
 *
 * @param socket_descriptor_array - array of socket descriptors - this array can be
 created on
 the stack via the library so we won't waste any heap memory on it as it's needed only
 temporarly for the moment of execution the xi_bsp_socket_select
 * @param socket_descriptor_length - this tells the user how many sockets he has to handle
 * @param timeout - this is library requirement of timeout - why do we need this ? -
 because our handlers might be timeout based, so we
                    need to give the first timeout in line that we have in order to be
 able to execute that timeout whenever the time is passed so it's a minimum timeout we can
 allow user spend in select call ( he can spend less time but he must NOT spend MORE time)
 */
xi_bsp_io_net_state_t
xi_bsp_socket_status_update( xi_bsp_socket_descriptor_t* socket_descriptor_array,
                             size_t socket_descriptor_array_length,
                             xi_time_t timeout )
{
    /* BODY OF THE SOCKET SELECT */

    /* The very first part would be to translate the given data to user specific platform
     * data */
    /* for POSIX like platforms implementation might look like this: */

    /* prepare the platform specific data - it's local no allocations required */
    fd_set rfds;
    fd_set wfds;
    fd_set efds;

    int max_fd_read  = 0;
    int max_fd_write = 0;
    int max_fd_error = 0;

    FD_ZERO( &rfds );
    FD_ZERO( &wfds );
    FD_ZERO( &efds );

    struct timeval tv;

    /* Iterate over given socket descriptors and fill the platform specific data with the
     * requirements passed from the library */
    {
        size_t it_socket = 0;
        for ( ; it_socket < socket_descriptor_array_length; ++it_socket )
        {
            xi_bsp_socket_descriptor_t* curr_descr = &socket_descriptor_array[it_socket];

            if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                            XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ ) )
            {
                FD_SET( curr_descr->socket, &rfds );
                max_fd_read =
                    curr_descr->socket > max_fd_read ? curr_descr->socket : max_fd_read;
            }

            /* This will be the new state we will enable to separate the case from
             * want connect to actual write capability of the socket on POSIX platforms */
            if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                            XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE ) ||
                 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                            XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT ) )
            {
                FD_SET( curr_descr->socket, &wfds );
                max_fd_write =
                    curr_descr->socket > max_fd_write ? curr_descr->socket : max_fd_write;
            }

            if ( ( curr_descr->in_out_socket_status_flags &
                   XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR ) > 0 )
            {
                FD_SET( curr_descr->socket, &efds );
                max_fd_error =
                    curr_descr->socket > max_fd_error ? curr_descr->socket : max_fd_error;
            }
        }
    }

    /* depends on the units but probably we would like to use seconds or milliseconds */
    tv.tv_sec =
        timeout; /* so the timeout parameter might go here if decide for seconds */
    tv.tv_usec = 0;

    /* prepare invocation of POSIX select function */
    const int max_fd = MAX( MAX( max_fd_read, max_fd_write ), max_fd_error );
    const int result = select( max_fd + 1, &rfds, &wfds, &efds, &tv );

    /* this is the case where there was no timeout and no error */
    if ( result > 0 )
    {
        /* if user iterates over the results he easily mark which sockets received which
         * event */
        size_t it_socket = 0;
        for ( ; it_socket < socket_descriptor_array_length; ++it_socket )
        {
            xi_bsp_socket_descriptor_t* curr_descr = &socket_descriptor_array[it_socket];

            if ( FD_ISSET( curr_descr->socket, &rfds ) )
            {
                xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                      XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ );
            }

            if ( FD_ISSET( socket_descriptor_array[it_socket].socket, &wfds ) )
            {
                if ( 1 ==
                     xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                           XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT ) )
                {
                    xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                          XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT );
                }
                else
                {
                    xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                          XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE );
                }
            }

            if ( FD_ISSET( curr_descr->socket, &efds ) )
            {
                xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                      XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR );
            }
        }

        /* return apropriate state code */
        return XI_BSP_IO_NET_STATE_OK;
    }

    if ( result == 0 )
    {
        /* return apropriate state code */
        return XI_BSP_IO_NET_STATE_TIMEOUT;
    }

    /* return apropriate state code */
    return XI_BSP_IO_NET_STATE_ERROR;
}

/* BSP SELECT USER/PLATFORM IMPLEMENTATION SECTION */
/* MICROCHIP VERSION */
/* This part of the file contains information about the example usage of the
 * xi_bsp_socket_select function on the USER side */
xi_bsp_io_net_state_t xi_bsp_socket_status_update_microchip(
    xi_bsp_socket_descriptor_t* socket_descriptor_array,
    size_t socket_descriptor_length,
    xi_time_t timeout )
{
    /* BODY OF THE SOCKET SELECT */
    /* Iterate over given socket descriptors and fill the platform specific data with the
     * requirements passed from the library */
    size_t it_socket = 0;
    for ( ; it_socket < socket_descriptor_length; ++it_socket )
    {
        xi_bsp_socket_descriptor_t* curr_descr = &socket_descriptor_array[it_socket];

        /* special case in case of microchip it first detects if library was waiting for
         * connected state change */
        /* this however requires a special knowledge about how our library operates in
         * terms of the asynchronious connect operation */
        if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                        XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT ) &&
             TRUE == TCPIsConnected( curr_descr->socket ) )
        {
            TCPFlush( curr_descr->socket );
            TCPTick();

            curr_descr->in_out_socket_status_flags =
                xi_bsp_unset_bitflag( curr_descr->in_out_socket_status_flags,
                                      XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT );

            TCPWasReset( curr_descr->socket );

            continue;
        }

        /* Skip the logic if the socket is not connected */
        if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                        XI_BSP_SELECT_SOCKET_EVENT_FLAG_CONNECT ) )
        {
            continue;
        }

        if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                        XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ ) &&
             ( 0 < TCPIsGetReady( curr_descr->socket ) ||
               0 == TCPIsConnected( curr_descr->socket ) ) )
        {
            xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                  XI_BSP_SELECT_SOCKET_EVENT_FLAG_READ );
        }
        else if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                             XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE ) &&
                  ( 0 < TCPIsPutReady( curr_descr->socket ) ||
                    0 == TCPIsConnected( curr_descr->socket ) ) )
        {
            xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                  XI_BSP_SELECT_SOCKET_EVENT_FLAG_WRITE );
        }
        else if ( 1 == xi_bsp_isset_bitflag( curr_descr->in_out_socket_status_flags,
                                             XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR ) )
        {
            xi_bsp_unset_bitflag( &curr_descr->in_out_socket_status_flags,
                                  XI_BSP_SELECT_SOCKET_EVENT_FLAG_ERROR );
        }
    }

    /* return apropriate state code */
    return XI_BSP_SELECT_STATE_OK;
}
