/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * This is a usage example of the SenML serializer module of
 * libxively. A xi_senml_t structure created, some entries are
 * injected into it. Then this structure is serialized and
 * printed to stdout.
 */

#include <stdio.h>
#include <xively_senml.h>


#ifndef XI_CROSS_TARGET
int main( int argc, char* argv[] )
#else
int xi_mqtt_logic_producer_main( xi_embedded_args_t* xi_embedded_args )
#endif
{
    ( void )( argc );
    ( void )( argv );

    /* Create SenML struct pointer on stack to have a handle locally.
     * Memory allocations will be handled by the Xively Library. */
    xi_senml_t* senml_structure = 0;
    xi_state_t state            = XI_STATE_OK;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    /* first create the senml structure
     * use XI_CREATE_SENML_EMPTY_STRUCT if no base values are required */
    XI_CREATE_SENML_STRUCT( state, senml_structure, XI_SENML_BASE_NAME( "thermostat" ),
                            XI_SENML_BASE_UNITS( "F" ), XI_SENML_BASE_TIME( 23 ) );

    /* adding entries to the already created senml structure */
    XI_ADD_SENML_ENTRY( state, senml_structure, XI_SENML_ENTRY_NAME( "kitchen" ),
                        XI_SENML_ENTRY_FLOAT_VALUE( 62.02 ) );

    XI_ADD_SENML_ENTRY( state, senml_structure, XI_SENML_ENTRY_NAME( "kitchen" ),
                        XI_SENML_ENTRY_FLOAT_VALUE( 61.02 ), XI_SENML_ENTRY_TIME( 25 ) );

    XI_ADD_SENML_ENTRY( state, senml_structure, XI_SENML_ENTRY_NAME( "heating" ),
                        XI_SENML_ENTRY_TIME( 25 ), XI_SENML_ENTRY_BOOLEAN_VALUE( 0 ),
                        XI_SENML_ENTRY_UNITS( "[ON/OFF]" ) );

#pragma GCC diagnostic pop

    /* serializer output variables */
    uint8_t* out_buffer = NULL;
    uint32_t out_size   = 0;

    state = xi_senml_serialize( senml_structure, &out_buffer, &out_size );

    /* Destroy senml structure since it is not required after the serialization */
    xi_senml_destroy( &senml_structure );

    if ( XI_STATE_OK == state )
    {
        printf( "\n" );
        printf( "SenML JSON, size: %d:\n", out_size );
        printf( " %s\n", out_buffer ? ( const char* )out_buffer : "NULL" );
        printf( "\n" );
    }

    /* Application is responsible to call destructor function in order to free up
     * memory allocated during the serialization. */
    xi_senml_free_buffer( &out_buffer );
    return 0;
}
