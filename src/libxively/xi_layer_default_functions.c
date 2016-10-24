#include "xi_layer_default_functions.h"

xi_state_t
xi_layer_default_post_connect( void* context, void* data, xi_state_t in_out_state )
{
    XI_LAYER_FUNCTION_PRINT_FUNCTION_DIGEST();

    void* layer_data = ( void* )XI_THIS_LAYER( context )->user_data;

    if ( NULL == layer_data )
    {
        return in_out_state;
    }

    XI_PROCESS_POST_CONNECT_ON_PREV_LAYER( context, data, in_out_state );

    return in_out_state;
}
