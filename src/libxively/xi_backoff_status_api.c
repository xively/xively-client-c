/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_backoff_status_api.h"
#include "xi_globals.h"
#include "xi_bsp_rng.h"

#ifdef __cplusplus
extern "C" {
#endif

/* local functions */
static xi_state_t xi_apply_cooldown( void );

void xi_inc_backoff_penalty()
{
    xi_globals.backoff_status.backoff_lut_i =
        XI_MIN( xi_globals.backoff_status.backoff_lut_i + 1,
                xi_globals.backoff_status.backoff_lut->elem_no - 1 );

    xi_restart_update_time();
}

void xi_dec_backoff_penalty()
{
    xi_globals.backoff_status.backoff_lut_i =
        XI_MAX( xi_globals.backoff_status.backoff_lut_i - 1, 0 );
}

uint32_t xi_get_backoff_penalty()
{
    const xi_backoff_lut_index_t prev_backoff_index =
        XI_MAX( xi_globals.backoff_status.backoff_lut_i - 1, 0 );

    const int32_t prev_backoff_base_value =
        xi_globals.backoff_status.backoff_lut->array[prev_backoff_index]
            .selector_t.ui32_value;

    /* full_range = previous backoff value | 0 if index == 0 */
    const int32_t full_range =
        ( xi_globals.backoff_status.backoff_lut_i == 0 ) ? 0 : prev_backoff_base_value;

    /* base_value = current backoff value */
    const int32_t base_value = xi_globals.backoff_status.backoff_lut
                                   ->array[xi_globals.backoff_status.backoff_lut_i]
                                   .selector_t.ui32_value;

    /* half_range = max( previous backoff value * 0.5, 1 ) */
    const int32_t half_range = XI_MAX( full_range / 2, 1 );

    /* rand_value = random( 0, full_range ) */
    const int32_t rand_value = xi_bsp_rng_get() % ( full_range + 1 );

    /* backoff_value =
     *      base_value + random( -0.5 * prev_backoff_base_value
     *                          , 0.5 * prev_backoff_base_value )
     * because if:
     *
     *      rand_value == 0:
     *
     *          then: backoff_value = base_value +
     *              0 - prev_backoff_base_value * 0.5
     *
     *      rand_value == prev_backoff_base_value:
     *
     *          then: backoff_value = base_value +
     *              prev_backoff_base_value - prev_backoff_base_value * 0.5
     *          so: backoff_value = base_value + prev_backoff_base_value * 0.5
     */
    const int32_t backoff_value = base_value + rand_value - half_range;

    /*
     * Clamp the ret_value so that it's not lesser than backoff_lut[ 0 ]
     */
    const int32_t ret_value =
        XI_MAX( backoff_value, ( int32_t )xi_globals.backoff_status.backoff_lut->array[0]
                                   .selector_t.ui32_value );

    return ret_value;
}

void xi_cancel_backoff_event()
{
    if ( NULL != xi_globals.backoff_status.next_update.ptr_to_position )
    {
        xi_evtd_cancel( xi_globals.evtd_instance,
                        &xi_globals.backoff_status.next_update );
    }
}

#ifdef XI_BACKOFF_GODMODE
void xi_reset_backoff_penalty()
{
    xi_globals.backoff_status.backoff_lut_i = 0;
    xi_globals.backoff_status.decay_lut_i   = 0;

    xi_cancel_backoff_event();
}
#endif

static void xi_backoff_release_luts( void )
{
    if ( xi_globals.backoff_status.backoff_lut != NULL )
    {
        xi_globals.backoff_status.backoff_lut =
            xi_vector_destroy( xi_globals.backoff_status.backoff_lut );
    }

    if ( xi_globals.backoff_status.decay_lut != NULL )
    {
        xi_globals.backoff_status.decay_lut =
            xi_vector_destroy( xi_globals.backoff_status.decay_lut );
    }
}

xi_state_t xi_backoff_configure_using_data( xi_vector_elem_t* backoff_lut,
                                            xi_vector_elem_t* decay_lut,
                                            size_t len,
                                            xi_memory_type_t memory_type )
{
    if ( backoff_lut == NULL || decay_lut == NULL || len == 0 )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_state_t local_state = XI_STATE_OK;

    xi_backoff_release_luts();

    xi_globals.backoff_status.backoff_lut =
        xi_vector_create_from( backoff_lut, len, memory_type );

    XI_CHECK_MEMORY( xi_globals.backoff_status.backoff_lut, local_state );

    xi_globals.backoff_status.decay_lut =
        xi_vector_create_from( decay_lut, len, memory_type );

    XI_CHECK_MEMORY( xi_globals.backoff_status.decay_lut, local_state );

err_handling:
    return local_state;
}

extern void xi_backoff_release()
{
    xi_backoff_release_luts();
}

xi_backoff_class_t xi_backoff_classify_state( const xi_state_t state )
{
    switch ( state )
    {
        case XI_CONNECTION_RESET_BY_PEER_ERROR:
        case XI_MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
        case XI_MQTT_IDENTIFIER_REJECTED:
        case XI_MQTT_BAD_USERNAME_OR_PASSWORD:
        case XI_MQTT_NOT_AUTHORIZED:
            return XI_BACKOFF_CLASS_TERMINAL;
        case XI_STATE_OK:
        case XI_STATE_WRITTEN:
            return XI_BACKOFF_CLASS_NONE;
        default:
            return XI_BACKOFF_CLASS_RECOVERABLE;
    }
}

xi_backoff_class_t xi_update_backoff_penalty( const xi_state_t state )
{
    xi_backoff_class_t backoff_class = xi_backoff_classify_state( state );

    xi_globals.backoff_status.backoff_class = backoff_class;

    switch ( backoff_class )
    {
        case XI_BACKOFF_CLASS_TERMINAL:
        case XI_BACKOFF_CLASS_RECOVERABLE:
            xi_inc_backoff_penalty();
            xi_debug_format( "inc backoff index: %d",
                             xi_globals.backoff_status.backoff_lut_i );
            break;
        case XI_BACKOFF_CLASS_NONE:
            break;
        default:
            xi_debug_logger( "invalid backoff class!" );
            assert( 0 );
    }

    return backoff_class;
}

xi_state_t xi_restart_update_time()
{
    xi_state_t local_state               = XI_STATE_OK;
    xi_evtd_instance_t* event_dispatcher = xi_globals.evtd_instance;

    if ( NULL != xi_globals.backoff_status.next_update.ptr_to_position )
    {
        local_state =
            xi_evtd_restart( event_dispatcher, &xi_globals.backoff_status.next_update,
                             xi_globals.backoff_status.decay_lut
                                 ->array[xi_globals.backoff_status.backoff_lut_i]
                                 .selector_t.ui32_value );
    }
    else
    {
        local_state =
            xi_evtd_execute_in( event_dispatcher, xi_make_handle( &xi_apply_cooldown ),
                                xi_globals.backoff_status.decay_lut
                                    ->array[xi_globals.backoff_status.backoff_lut_i]
                                    .selector_t.ui32_value,
                                &xi_globals.backoff_status.next_update );
    }

    return local_state;
}

static xi_state_t xi_apply_cooldown( void )
{
    /* clearing the event pointer is the first thing to do */
    assert( NULL == xi_globals.backoff_status.next_update.ptr_to_position );

    if ( xi_globals.backoff_status.backoff_class == XI_BACKOFF_CLASS_NONE )
    {
        xi_dec_backoff_penalty();
        xi_debug_format( "dec backoff index: %d",
                         xi_globals.backoff_status.backoff_lut_i );
    }

    /* if the backoff lut index is greater than 0 */
    if ( xi_globals.backoff_status.backoff_lut_i > 0 )
    {
        return xi_restart_update_time();
    }

    return XI_STATE_OK;
}

#ifdef __cplusplus
}
#endif
