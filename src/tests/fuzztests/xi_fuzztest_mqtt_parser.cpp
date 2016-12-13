#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <xi_mqtt_parser.h>
#include <xi_mqtt_message.h>
#include <xi_macros.h>

const static size_t XI_BUFFER_SIZE = 32;

/* This is the fuzzer signature, and we cannot change it */
extern "C" int LLVMFuzzerTestOneInput( const uint8_t* data, size_t size )
{
    xi_mqtt_parser_t parser;
    xi_state_t local_state = XI_STATE_OK;
    xi_mqtt_message_t* msg = NULL;

    size_t size_eaten = 0;

    XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
    xi_mqtt_parser_init( &parser );

    while ( size_eaten != size )
    {
        const size_t copied_buffer_size = XI_MIN( XI_BUFFER_SIZE, ( size - size_eaten ) );

        xi_data_desc_t* data_desc =
            xi_make_desc_from_buffer_copy( data + size_eaten, copied_buffer_size );

        local_state = xi_mqtt_parser_execute( &parser, msg, data_desc );

        if ( local_state == XI_STATE_OK || local_state == XI_MQTT_PARSER_ERROR )
        {
            xi_mqtt_message_free( &msg );
            XI_ALLOC_AT( xi_mqtt_message_t, msg, local_state );
        }

        size_eaten += copied_buffer_size - ( data_desc->length - data_desc->curr_pos );

        xi_free_desc( &data_desc );
    }

    xi_mqtt_message_free( &msg );

err_handling:
    return 0;
}
