#include <wm_os.h>
#include <app_framework.h>
#include <wmtime.h>
#include <cli.h>
#include <wmstdio.h>
#include <board.h>
#include <wmtime.h>
#include <psm.h>
#include <led_indicator.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <stdint.h>

#include "xi_coroutine.h"

/**
 * \brief To be able to pass data between functions
 */
typedef struct
{
    int sock_fd;
    struct sockaddr_in endpoint_addr;
} conn_data_t;

/**
 * states of the async main handle
 */
typedef enum handle_state_t
{
    STATE_OK,
    STATE_ERROR,
    WANT_READ,
    WANT_WRITE,
} handle_state_e;

/* Thread handle */
static os_thread_t app_thread;

/* Buffer to be used as stack */
static os_thread_stack_define( app_stack, 24 * 1024 );

/* prepare buffer that will be used to communicate with the endpoint */
static const char data_to_sent[] = "GET "
                                   "/cgi-bin/"
                                   "fortune.pl?fortune_db=pratchett&text_"
                                   "format=yes&max_lines=2 HTTP/1.0\r\n"
                                   "Host: anduin.eldar.org\r\n"
                                   "Accept: text/html\r\n\r\n";
static const size_t data_to_sent_size = sizeof( data_to_sent );

/* put the ip address and port here */
static char address[] = "24.106.248.90";
static int port       = 80;

/* This function is defined for handling critical error.
 * For this application, we just stall and do nothing when
 * a critical error occurs.
 *
 */
void appln_critical_error_handler( void* data )
{
    while ( 1 )
        ;
    /* do nothing -- stall */
}

/**
 * @brief main_handle
 * @param cs
 * @param connection_data
 * @param data
 * @param data_size
 * @return
 *
 * main handle of xively test application it is the main coroutine which
 * manages the sequence of execution:
 *
 * 1) connect
 * 2) send
 * 3) receive
 * 4) disconnect
 */
static handle_state_e main_handle( uint16_t* cs,
                                   const conn_data_t* const connection_data,
                                   const char* data,
                                   const size_t data_size )
{
    // locals that must exist through yields
    static size_t data_sent = -1;
    static char recv_buffer[256];
    int valopt    = 0;
    int r         = 0;
    socklen_t lon = sizeof( int );

    // we shall begin with connection attempt
    XI_CR_START( *cs );

    data_sent = 0;

    r = connect( connection_data->sock_fd,
                 ( struct sockaddr* )&connection_data->endpoint_addr,
                 sizeof( struct sockaddr ) );

    if ( r == 0 )
    {
        goto connected;
    }

    if ( r == -1 )
    {
        int err = errno;

        if ( err != EINPROGRESS )
        {
            wmprintf( "errno not EINPROGRESS\n" );
            XI_CR_EXIT( *cs, STATE_ERROR );
        }

        // it will get back whenever we can write on the socket
        wmprintf( "errno EINPROGRESS\n" );
        XI_CR_YIELD( *cs, WANT_WRITE );
    }

    // now after re-entry we have to check
    if ( getsockopt( connection_data->sock_fd, SOL_SOCKET, SO_ERROR, ( void* )( &valopt ),
                     &lon ) < 0 )
    {
        wmprintf( "error while getsockopt\n" );
        goto closing;
    }

    if ( valopt )
    {
        wmprintf( "error while connecting\n" );
        goto closing;
    }

connected:
    wmprintf( "Connected! state = %d\n", valopt );
    led_on( board_led_1() );

sending:
    data_sent += write( connection_data->sock_fd, data + data_sent, data_size );

    if ( data_sent == 0 )
    {
        wmprintf( "connection reset by peer\n" );
        goto closing;
    }

    if ( data_sent < 0 )
    {
        int err = errno;

        if ( err == EINPROGRESS || err == EAGAIN )
        {
            XI_CR_YIELD( *cs, WANT_WRITE );
            goto sending;
        }

        wmprintf( "error while sending: [%d]\n", err );
        goto closing;
    }

    if ( data_sent < data_size )
    {
        goto sending;
    }

    XI_CR_YIELD( *cs, WANT_READ );

receiving:
    memset( recv_buffer, 0, sizeof( recv_buffer ) );

    r = read( connection_data->sock_fd, recv_buffer, sizeof( recv_buffer ) - 2 );

    if ( r == 0 )
    {
        wmprintf( "\nConnection reset by peer\n" );
        goto closing;
    }

    if ( r < 0 )
    {
        int err = errno;

        if ( err == EINPROGRESS || err == EAGAIN )
        {
            XI_CR_YIELD( *cs, WANT_READ );
            goto receiving;
        }

        wmprintf( "\nerror while sending: [%d]\n", err );
        goto closing;
    }

#if 0
    unsigned char prev = '\0';

    // replace all single \n with \n
    int i = 0;
    for ( ; i < r; ++i )
    {
        unsigned char c = recv_buffer[ i ];

        if ( c == 10 && prev != 13 )
        {
            wmprintf( "\r" );
        }

        wmstdio_putchar( ( char* ) &c );

        prev = c;
    }
#endif

    wmprintf( "%s", recv_buffer );
    wmstdio_flush();

    goto receiving;

closing:
    close( connection_data->sock_fd );

    wmprintf( "\ndisconnected...\n" );

    led_off( board_led_1() );
    XI_CR_RESET( *cs );

    XI_CR_END();

    return STATE_ERROR;
}

static int init_non_blocking_socket()
{
    int ret = socket( AF_INET, SOCK_STREAM, 0 );

    if ( ret < 0 )
    {
        wmprintf( "socket creation failed \n" );
    }

    int flags = fcntl( ret, F_GETFL, 0 );

    if ( flags == -1 )
    {
        wmprintf( "fcntl failed \n" );
        return -1;
    }

    if ( fcntl( ret, F_SETFL, flags | O_NONBLOCK ) == -1 )
    {
        wmprintf( "fcntl set failed \n" );
        return -1;
    }

    return ret;
}

/**
 * @brief xively_test_main
 * @param data
 *
 * Main thread of the xively test application
 */
static void xively_test_main( os_thread_arg_t data )
{
    wmprintf( "\n --- Xively Test Application Started---\n" );

    /* thread variables */
    conn_data_t connection_data;
    fd_set r_master_set, r_working_set, w_master_set, w_working_set;
    uint16_t cs = 0;
    struct timeval timeout;
    int max_fd                  = 0;
    handle_state_e handle_state = STATE_OK;

    // very infinite loop
    for ( ;; )
    {
        memset( &connection_data, 0, sizeof( connection_data ) );

        connection_data.sock_fd = init_non_blocking_socket();

        if ( connection_data.sock_fd < 0 )
        {
            wmprintf( "socket creation failed\n" );
            break;
        }
        else
        {
            wmprintf( "socket creation succesfull!\n" );
        }

        // fill in connection address and port
        connection_data.endpoint_addr.sin_family      = AF_INET;
        connection_data.endpoint_addr.sin_addr.s_addr = inet_addr( address );
        connection_data.endpoint_addr.sin_port        = htons( port );

        // prepare the r and write sets
        FD_ZERO( &r_master_set );
        FD_ZERO( &w_master_set );
        FD_ZERO( &r_working_set );
        FD_ZERO( &w_working_set );

        // sets the file descriptor max value for select
        max_fd = connection_data.sock_fd;

        // fill in the master read and master write sets
        FD_SET( connection_data.sock_fd, &r_master_set );
        FD_SET( connection_data.sock_fd, &w_master_set );

        // set one minute timeout for select
        memset( &timeout, 0, sizeof( struct timeval ) );
        timeout.tv_sec  = 1 * 60;
        timeout.tv_usec = 0;

        do
        {
            led_on( board_led_4() );

            handle_state =
                main_handle( &cs, &connection_data, data_to_sent, data_to_sent_size );

            led_off( board_led_4() );

            if ( handle_state == WANT_READ || handle_state == WANT_WRITE )
            {
                memcpy( &r_working_set, &r_master_set, sizeof( fd_set ) );
                memcpy( &w_working_set, &w_master_set, sizeof( fd_set ) );

                led_on( board_led_2() );

                select( max_fd + 1, handle_state == WANT_READ ? &r_working_set : NULL,
                        handle_state == WANT_WRITE ? &w_working_set : NULL, NULL,
                        &timeout );

                led_off( board_led_2() );
            }

            // os_thread_sleep( os_msec_to_ticks( 1000 ) );
        } while ( handle_state != STATE_ERROR );
    }
}

/* This is the main event handler for this project. The application framework
 * calls this function in response to the various events in the system.
 */
int common_event_handler( int event, void* data )
{
    int ret;
    static bool is_cloud_started;

    switch ( event )
    {
        case AF_EVT_WLAN_INIT_DONE:
            ret = psm_cli_init();
            if ( ret != WM_SUCCESS )
                wmprintf( "Error: psm_cli_init failed\n" );
            int i = ( int )data;

            if ( i == APP_NETWORK_NOT_PROVISIONED )
            {
                wmprintf( "\nPlease provision the device "
                          "and then reboot it:\n\n" );
                wmprintf( "psm-set network ssid <ssid>\n" );
                wmprintf( "psm-set network security <security_type>"
                          "\n" );
                wmprintf( "    where: security_type: 0 if open,"
                          " 3 if wpa, 4 if wpa2\n" );
                wmprintf( "psm-set network passphrase <passphrase>"
                          " [valid only for WPA and WPA2 security]\n" );
                wmprintf( "psm-set network configured 1\n" );
                wmprintf( "pm-reboot\n\n" );
            }
            else
                app_sta_start();

            break;
        case AF_EVT_NORMAL_CONNECTED:
            if ( !is_cloud_started )
            {
                ret = os_thread_create( &app_thread,          /* thread handle */
                                        "xively_demo_thread", /* thread name */
                                        xively_test_main,     /* entry function */
                                        0,                    /* argument */
                                        &app_stack,           /* stack */
                                        OS_PRIO_2 );          /* priority - medium low */
                is_cloud_started = true;
            }
            break;
        default:
            break;
    }

    return 0;
}

static void modules_init()
{
    int ret;

    ret = wmstdio_init( UART0_ID, 0 );
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: wmstdio_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }

    ret = cli_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: cli_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }

    ret = pm_cli_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: pm_cli_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    /* Initialize time subsystem.
     *
     * Initializes time to 1/1/1970 epoch 0.
     */
    ret = wmtime_init();
    if ( ret != WM_SUCCESS )
    {
        wmprintf( "Error: wmtime_init failed\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    return;
}

int main()
{
    modules_init();

    wmprintf( "Build Time: " __DATE__ " " __TIME__ "\n" );

    /* Start the application framework */
    if ( app_framework_start( common_event_handler ) != WM_SUCCESS )
    {
        wmprintf( "Failed to start application framework\n" );
        appln_critical_error_handler( ( void* )-WM_FAIL );
    }
    return 0;
}
