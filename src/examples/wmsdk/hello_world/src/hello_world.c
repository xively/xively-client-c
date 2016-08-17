/*
 *  Copyright (C) 2008-2013 Marvell International Ltd.
 *  All Rights Reserved.
 */
/*
 * Hello World Application
 *
 * Summary:
 *
 * Prints Hello World every 5 seconds on the serial console.
 * The serial console is set on UART-0.
 *
 * A serial terminal program like HyperTerminal, putty, or
 * minicom can be used to see the program output.
 */
#include <wmstdio.h>
#include <wm_os.h>
#include <xively.h>

/*
 * The application entry point is main(). At this point, minimal
 * initialization of the hardware is done, and also the RTOS is
 * initialized.
 */
int main( void )
{
    int count = 0;

    /* Initialize console on uart0 */
    wmstdio_init( UART0_ID, 0 );

    wmprintf( "Hello World application Started\r\n" );

    // initialize xi library.
    if ( XI_STATE_OK != xi_create_default_context() )
    {
        wmprintf( " xi failed initialization\n" );
        return -1;
    }

    while ( 1 )
    {
        count++;
        wmprintf( "Hello World: iteration %d\r\n", count );
        wmprintf( "Hello World: iteration os_ticks_get() =  %d\r\n",
                  os_ticks_to_msec( os_ticks_get() ) );
        wmprintf( "Hello World: iteration os_total_ticks_get() = %d\r\n",
                  os_ticks_to_msec( os_total_ticks_get() ) );

        /* Sleep  5 seconds */
        os_thread_sleep( os_msec_to_ticks( 1000 ) );
    }

    xi_delete_default_context();
    xi_shutdown();

    return 0;
}
