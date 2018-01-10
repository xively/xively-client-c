/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>

#include "xively_senml.h"
#include "xi_macros.h"
#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_helpers.h"
#include "xi_senml_json_serializer.h"
#include <stdarg.h>
#include <xi_list.h>

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_senml_serialize( xi_senml_t* senml_structure,
                               uint8_t** out_buffer,
                               uint32_t* out_size )
{
    if ( NULL == out_buffer || NULL == out_size )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_data_desc_t* buffer = NULL;

    xi_state_t state = xi_senml_json_serialize( &buffer, senml_structure );

    XI_CHECK_STATE( state );
    XI_CHECK_CND( buffer == NULL, XI_SERIALIZATION_ERROR, state );

    *out_buffer = buffer->data_ptr;
    *out_size   = buffer->length;

    /* release memory */
    buffer->memory_type = XI_MEMORY_TYPE_UNMANAGED;

    xi_free_desc( &buffer );

    return state;

err_handling:

    xi_free_desc( &buffer );
    *out_size = 0;

    return state;
}

xi_state_t xi_senml_free_buffer( uint8_t** buffer )
{
    if ( NULL == buffer )
    {
        return XI_INVALID_PARAMETER;
    }

    XI_SAFE_FREE( *buffer );

    return XI_STATE_OK;
}

void xi_senml_entry_destroy( xi_senml_entry_t** senml_entry )
{
    assert( senml_entry != 0 );

    XI_SAFE_FREE( ( *senml_entry )->name );
    XI_SAFE_FREE( ( *senml_entry )->units );

    if ( ( *senml_entry )->set.value_set )
    {
        if ( ( *senml_entry )->value_cnt.value_type == XI_SENML_VALUE_TYPE_STRING )
        {
            XI_SAFE_FREE( ( *senml_entry )->value_cnt.value.string_value );
        }
    }

    XI_SAFE_FREE( *senml_entry );
}

void xi_senml_destroy( xi_senml_t** senml_structure )
{
    assert( senml_structure != 0 );

    xi_senml_entry_t** senml_entries = &( *senml_structure )->entries_list;

    while ( *senml_entries != 0 )
    {
        xi_senml_entry_t* out = 0;
        XI_LIST_POP( xi_senml_entry_t, *senml_entries, out );
        xi_senml_entry_destroy( &out );
    }

    XI_SAFE_FREE( ( *senml_structure )->base_name );
    XI_SAFE_FREE( ( *senml_structure )->base_units );
    XI_SAFE_FREE( *senml_structure );
}

xi_state_t xi_create_senml_struct( xi_senml_t** senml_ptr, int count, ... )
{
    xi_state_t state = XI_STATE_OK;
 
    XI_CHECK_CND( NULL == senml_ptr, XI_INVALID_PARAMETER, state );
    XI_CHECK_CND( NULL != *senml_ptr, XI_INVALID_PARAMETER, state );

    XI_ALLOC_AT( xi_senml_t, *senml_ptr, state );

    va_list ap;
    uint8_t i = 0;

    va_start( ap, count );

    xi_senml_t senml_in;
    for ( i = 0; i < count; ++i )
    {
        senml_in = va_arg( ap, xi_senml_t );

        if ( 1 == senml_in.set.base_name_set )
        {
            XI_CHECK_CND( NULL == senml_in.base_name, XI_INVALID_PARAMETER, state );

            if ( 1 == ( *senml_ptr )->set.base_name_set )
            {
                XI_SAFE_FREE( ( *senml_ptr )->base_name );
            }

            ( *senml_ptr )->set.base_name_set = 1;
            ( *senml_ptr )->base_name         = xi_str_dup( senml_in.base_name );
        }

        if ( 1 == senml_in.set.base_units_set )
        {
            XI_CHECK_CND( NULL == senml_in.base_units, XI_INVALID_PARAMETER, state );

            if ( 1 == ( *senml_ptr )->set.base_units_set )
            {
                XI_SAFE_FREE( ( *senml_ptr )->base_units );
            }

            ( *senml_ptr )->set.base_units_set = 1;
            ( *senml_ptr )->base_units         = xi_str_dup( senml_in.base_units );
        }

        if ( 1 == senml_in.set.base_time_set )
        {
            ( *senml_ptr )->set.base_time_set = 1;
            ( *senml_ptr )->base_time         = senml_in.base_time;
        }
    }

    va_end( ap );

err_handling:
    return state;
}

void xi_debug_dump_senml_entry( const xi_senml_entry_t* entry, const char* prefix )
{
    XI_UNUSED( prefix );

    xi_debug_printf( "%s", prefix );

    if ( entry->set.name_set )
    {
        xi_debug_printf( " name :       [%s]\n", entry->name );
    }
    if ( entry->set.units_set )
    {
        xi_debug_printf( " units:       [%s]\n", entry->units );
    }
    if ( entry->set.time_set )
    {
        xi_debug_printf( " time:        [%d]\n", entry->time );
    }
    if ( entry->set.update_time_set )
    {
        xi_debug_printf( " utime:       [%d]\n", entry->update_time );
    }

    if ( entry->set.value_set )
    {
        xi_debug_printf( " value type:  [%d]\n", entry->value_cnt.value_type );

        switch ( entry->value_cnt.value_type )
        {
            case XI_SENML_VALUE_TYPE_FLOAT:
                xi_debug_printf( " float:       [%f]\n",
                                 entry->value_cnt.value.float_value );
                break;
            case XI_SENML_VALUE_TYPE_STRING:
                xi_debug_printf( " string:      [%s]\n",
                                 entry->value_cnt.value.string_value );
                break;
            case XI_SENML_VALUE_TYPE_BOOLEAN:
                xi_debug_printf( " boolean:     [%d]\n",
                                 entry->value_cnt.value.boolean_value );
                break;
        }
    }
}

xi_state_t xi_add_senml_entry( xi_senml_t* senml_ptr, int count, ... )
{
    xi_state_t state = XI_STATE_OK;

    XI_ALLOC( xi_senml_entry_t, entry, state );

    va_list ap;
    uint8_t i = 0;

    va_start( ap, count );

    xi_senml_entry_t entry_in;
    for ( ; i < count; ++i )
    {
        entry_in = va_arg( ap, xi_senml_entry_t );

        if ( 1 == entry_in.set.name_set )
        {
            XI_CHECK_CND( NULL == entry_in.name, XI_INVALID_PARAMETER, state );

            if ( 1 == entry->set.name_set )
            {
                XI_SAFE_FREE( entry->name );
            }

            entry->set.name_set = 1;
            entry->name         = xi_str_dup( entry_in.name );
        }

        if ( 1 == entry_in.set.units_set )
        {
            XI_CHECK_CND( NULL == entry_in.units, XI_INVALID_PARAMETER, state );

            if ( 1 == entry->set.units_set )
            {
                XI_SAFE_FREE( entry->units );
            }

            entry->set.units_set = 1;
            entry->units         = xi_str_dup( entry_in.units );
        }

        if ( 1 == entry_in.set.value_set )
        {
            if ( 1 == entry->set.value_set &&
                 XI_SENML_VALUE_TYPE_STRING == entry->value_cnt.value_type )
            {
                XI_SAFE_FREE( entry->value_cnt.value.string_value );
            }

            entry->set.value_set = 1;
            entry->value_cnt     = entry_in.value_cnt;

            if ( XI_SENML_VALUE_TYPE_STRING == entry_in.value_cnt.value_type )
            {
                XI_CHECK_CND( NULL == entry_in.value_cnt.value.string_value,
                              XI_INVALID_PARAMETER, state );

                entry->value_cnt.value.string_value =
                    xi_str_dup( entry_in.value_cnt.value.string_value );
            }
        }

        if ( 1 == entry_in.set.time_set )
        {
            entry->set.time_set = 1;
            entry->time         = entry_in.time;
        }

        if ( 1 == entry_in.set.update_time_set )
        {
            entry->set.update_time_set = 1;
            entry->update_time         = entry_in.update_time;
        }

        // xi_debug_dump_senml_entry( &entry_in, "--- --- --- --- --- --- ---\n---
        // entry_in:\n" );
        // xi_debug_dump_senml_entry( entry, "--- entry_sum:\n" );
    }

    XI_LIST_PUSH_BACK( xi_senml_entry_t, senml_ptr->entries_list, entry );

    va_end( ap );

err_handling:
    return state;
}

#ifdef __cplusplus
}
#endif
