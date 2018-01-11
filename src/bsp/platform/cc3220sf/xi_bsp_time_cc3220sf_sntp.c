/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/******************************************************************************
 *                                                                            *
 *  sntp_task.c                                                               *
 *                                                                            *
 ******************************************************************************/
#include "xi_bsp_time_cc3220sf_sntp.h"
#include "simplelink.h"

/******************************************************************************
 *                                                                            *
 *  Macros                                                                    *
 *                                                                            *
 ******************************************************************************/

#define TASK_NAME "SNTP_Task"
#define STACK_SIZE ( 2 * 512 )
#define SNTP_RCV_TO_SEC 30

#define NTP_UPDATE_MS ( 10 * 60 * 1000 ) /* 10 Minutes */
#define NTP_RETRY_MS ( 1 * 60 * 1000 )   /*  1 Minute  */

#define NTP_SERVER_NAMES                                                                 \
    {                                                                                    \
        "pool.ntp.org", "time-a.nist.gov", "time-b.nist.gov", "time-c.nist.gov"          \
    }


#define SNTP_PORT 123
#define SECONDS_1900_TO_1970 ( 2208988800 )

extern int Report( const char*, ... );

/******************************************************************************
 *                                                                            *
 *  SNTP Protocol                                                             *
 *                                                                            *
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9  0  1     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *  00  |LI | VN  |Mode |    Stratum    |     Poll      |   Precision   |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *  04  |                          Root  Delay                          |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *  08  |                       Root  Dispersion                        |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *  12  |                     Reference Identifier                      |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *      |                                                               |     *
 *  16  |                    Reference Timestamp (64)                   |     *
 *      |                                                               |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *      |                                                               |     *
 *  24  |                    Originate Timestamp (64)                   |     *
 *      |                                                               |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *      |                                                               |     *
 *  32  |                     Receive Timestamp (64)                    |     *
 *      |                                                               |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *      |                                                               |     *
 *  40  |                     Transmit Timestamp (64)                   |     *
 *      |                                                               |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *  48  |                 Key Identifier (optional) (32)                |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *      |                                                               |     *
 *      |                                                               |     *
 *  52  |                 Message Digest (optional) (128)               |     *
 *      |                                                               |     *
 *      |                                                               |     *
 *      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+     *
 *                                                                            *
 ******************************************************************************/

#define SNTP_MSG_LEN 48
#define SNTP_XMIT_TIME_OFS 40
#define SNTP_LI_NO_WARNING 0x00
#define SNTP_VERSION ( 4 << 3 ) /* NTP Version 4 */
#define SNTP_MODE_CLIENT 0x03
#define SNTP_MODE_SERVER 0x04
#define SNTP_MODE_BROADCAST 0x05
#define SNTP_MODE_MASK 0x07


/******************************************************************************
 *                                                                            *
 *  SNTP Status                                                               *
 *                                                                            *
 ******************************************************************************/


#define SNTP_STATUS_TABLE                                                                \
    /*             ENUM,                  VALUE,  MSG                */                  \
                                                                                         \
    SNTP_STATUS_ENTRY( SNTP_OK, 0, "OK" )                                                \
    SNTP_STATUS_ENTRY( SNTP_DNS_ERR, -1000, "DNS Error" )                                \
    SNTP_STATUS_ENTRY( SNTP_BAD_SERVER_ADDR, -1001, "Bad Server Address" )               \
    SNTP_STATUS_ENTRY( SNTP_SOCKET_ERR, -1002, "socket () Error" )                       \
    SNTP_STATUS_ENTRY( SNTP_BIND_ERR, -1003, "bind () Error" )                           \
    SNTP_STATUS_ENTRY( SNTP_SETSOCKOPT_ERR, -1004, "setsockopt () Error" )               \
    SNTP_STATUS_ENTRY( SNTP_SENDTO_ERR, -1005, "sendto () Error" )                       \
    SNTP_STATUS_ENTRY( SNTP_RECVFROM_ERR, -1006, "recvfrom () Error" )                   \
    SNTP_STATUS_ENTRY( SNTP_BAD_RESPONSE, -1007, "Bad SNTP Response" )                   \
    SNTP_STATUS_ENTRY( SNTP_BAD_DATA, -1007, "Bad SNTP Data" )                           \
                                                                                         \
    /*  Unknown is last */                                                               \
    SNTP_STATUS_ENTRY( SNTP_UNKNOWN_ERR, -1100, "Unknown Error" )


/*
 *  Status Enums
 */
#define SNTP_STATUS_ENTRY( ENUM, VALUE, MSG ) ENUM = VALUE,


typedef enum { SNTP_STATUS_TABLE } sntp_status_e;


#undef SNTP_STATUS_ENTRY


/*
 *  Status Entries
 */
#define SNTP_STATUS_ENTRY( ENUM, VALUE, MSG )                                            \
    {                                                                                    \
        .value = VALUE, .msg = MSG,                                                      \
    },


typedef struct
{
    int value;
    char* msg;
} sntp_status_t;


static sntp_status_t sntp_status_entries[] = {SNTP_STATUS_TABLE};


#undef SNTP_STATUS_ENTRY


/******************************************************************************
 *                                                                            *
 *  Declarations                                                              *
 *                                                                            *
 ******************************************************************************/

static int ntp_server_index     = 0;
static char* ntp_server_names[] = NTP_SERVER_NAMES;

#ifdef errno
#undef errno
#endif
static int errno;

static uint32_t start_time_ntp = 0;
uint32_t uptime                = 0;

#define NTP_SERVER_COUNT ( sizeof( ntp_server_names ) / sizeof( char* ) )


/******************************************************************************
 *                                                                            *
 *  get_status_msg ()                                                         *
 *                                                                            *
 ******************************************************************************/

static char* get_status_msg( int status )
{
    sntp_status_t* st_p = sntp_status_entries;

    while ( ( st_p->value != status ) && ( st_p->value != SNTP_UNKNOWN_ERR ) )
    {
        st_p++;
    }

    return st_p->msg;
}

static uint32_t dns_lookup( char* host_name )
{
    int status;
    uint32_t rval;

    if ( ( status = sl_NetAppDnsGetHostByName( ( _i8* )host_name, strlen( host_name ),
                                               ( _u32* )&rval, SL_AF_INET ) ) == 0 )
    {
        status = SNTP_OK;
        rval   = htonl( rval );
    }
    else
    {
        errno  = status;
        status = SNTP_DNS_ERR;
        rval   = 0;
    }

    if ( status != SNTP_OK )
    {
        Report( "SNTP Status: %s\n", get_status_msg( status ) );

        if ( errno != 0 )
        {
            Report( "  errno: %d: %s\n", errno, strerror( errno ) );
        }
    }

    return rval;
}


static ntp_time_t sntp_get( uint32_t server_addr )
{
    uint8_t sntp_request[SNTP_MSG_LEN];
    uint8_t sntp_response[SNTP_MSG_LEN];
    int s;
    struct sockaddr_in local_sa;
    struct sockaddr_in ntp_sa;
    socklen_t sa_len;
    struct timeval rcv_to;
    ntp_time_t timestamp;
    int status;
    ntp_time_t rval;

    sa_len = sizeof( struct sockaddr_in );
    errno  = 0;
    rval   = 0;

    do
    {
        if ( server_addr == 0 )
        {
            status = SNTP_BAD_SERVER_ADDR;
            break;
        }

        if ( ( s = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )
        {
            errno  = s;
            status = SNTP_SOCKET_ERR;
            break;
        }

        /*
         * Bind to local address
         */
        memset( &local_sa, 0, sizeof( local_sa ) );
        local_sa.sin_family      = AF_INET;
        local_sa.sin_port        = htons( INADDR_ANY );
        local_sa.sin_addr.s_addr = htonl( INADDR_ANY );

        if ( ( status = bind( s, ( struct sockaddr* )&local_sa, sa_len ) ) != 0 )
        {
            errno  = status;
            status = SNTP_BIND_ERR;
            break;
        }

        /*
         *  Setup server address
         */
        memset( &ntp_sa, 0, sizeof( ntp_sa ) );
        ntp_sa.sin_family      = AF_INET;
        ntp_sa.sin_port        = htons( SNTP_PORT );
        ntp_sa.sin_addr.s_addr = server_addr;

        /*
         *  Prepare SNTP request
         */
        memset( sntp_request, 0, sizeof( sntp_request ) );
        sntp_request[0] = SNTP_LI_NO_WARNING | SNTP_VERSION | SNTP_MODE_CLIENT;

        /*
         *  Set receive timeout
         */
        rcv_to.tv_sec = SNTP_RCV_TO_SEC;

        if ( ( status = setsockopt( s, SOL_SOCKET, SO_RCVTIMEO, &rcv_to,
                                    sizeof( rcv_to ) ) ) != 0 )
        {
            errno  = status;
            status = SNTP_SETSOCKOPT_ERR;
            break;
        }

        /*
         *  Send SNTP request
         */
        if ( ( status = sendto( s, sntp_request, sizeof( sntp_request ), 0,
                                ( struct sockaddr* )&ntp_sa, sa_len ) ) < 0 )
        {
            errno  = status;
            status = SNTP_SENDTO_ERR;
            break;
        }

        /*
         *  Receive SNTP response
         */
        status = recvfrom( s, sntp_response, sizeof( sntp_response ), 0,
                           ( struct sockaddr* )&ntp_sa, &sa_len );

        /*
         *  Check SNTP response
         */
        if ( status != SNTP_MSG_LEN )
        {
            if ( status < 0 )
            {
                errno  = status;
                status = SNTP_RECVFROM_ERR;
            }
            else
            {
                status = SNTP_BAD_RESPONSE;
            }

            break;
        }

        if ( ( ( sntp_response[0] & SNTP_MODE_MASK ) != SNTP_MODE_SERVER ) &&
             ( ( sntp_response[0] & SNTP_MODE_MASK ) != SNTP_MODE_BROADCAST ) )
        {
            status = SNTP_BAD_DATA;
            break;
        }

        /*
         *  Get time from Transmit Timestamp
         */
        memcpy( &timestamp, ( sntp_response + SNTP_XMIT_TIME_OFS ), sizeof( timestamp ) );
        rval   = ntohl( timestamp );
        status = 0;
    } while ( 0 );


    if ( s >= 0 )
    {
        close( s );
    }


    if ( status != 0 )
    {
        Report( "Status: %s\n", get_status_msg( status ) );

        if ( errno != 0 )
        {
            Report( "  errno: %d: %s\n", errno, strerror( errno ) );
        }
    }

    return rval;
}


void xi_bsp_time_sntp_init( void* pvParameters )
{
    uint32_t server_addr;
    ntp_time_t t;
    uint32_t sleep_ms;
    int i;

    while ( 1 )
    {
        for ( i = 0; i < NTP_SERVER_COUNT; i++ )
        {
            ntp_server_index = ( ntp_server_index + i ) % NTP_SERVER_COUNT;
            server_addr      = dns_lookup( ntp_server_names[ntp_server_index] );

            if ( ( t = sntp_get( server_addr ) ) > 0 )
            {
                break;
            }
        }

        if ( t > 0 )
        {
            start_time_ntp = ( t - uptime );
            /* printf( "SNTP_G: %d, %s->0x%08X\n\r", xi_bsp_time_sntp_getseconds_posix(),
                    ntp_server_names[ntp_server_index], server_addr ); */

            sleep_ms = NTP_UPDATE_MS;
        }
        else
        {
            sleep_ms = NTP_RETRY_MS;
        }

        ( void )sleep_ms;
        /* printf( "SNTP_L: %d, UT %d\n\r", xi_bsp_time_sntp_getseconds_posix(), uptime );
         */
        break;
    }
}

ntp_time_t xi_bsp_time_sntp_getseconds_ntp( void )
{
    ntp_time_t rval = 0;

    if ( start_time_ntp > SECONDS_1900_TO_1970 )
    {
        rval = start_time_ntp + uptime;
    }

    return rval;
}

posix_time_t xi_bsp_time_sntp_getseconds_posix( void )
{
    posix_time_t rval = 0;
    if ( start_time_ntp > SECONDS_1900_TO_1970 )
    {
        rval = ( posix_time_t )( start_time_ntp + uptime - SECONDS_1900_TO_1970 );
    }

    return rval;
}
