/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_debug.h"
#include "xi_macros.h"

#include "xi_senml_json_serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/* static buffers used to generate senml content */
static const char xi_senml_pat_open[]                  = "{\"e\":[";
static const char xi_senml_pat_entries_close[]         = "]";
static const char xi_senml_pat_close[]                 = "}";
static const char xi_senml_pat_base_time[]             = "\"bt\":";
static const char xi_senml_pat_base_units[]            = "\"bu\":";
static const char xi_senml_pat_base_name[]             = "\"bn\":";
static const char xi_senml_pat_quot[]                  = "\"";
static const char xi_senml_pat_coln[]                  = ",";
static const char xi_senml_pat_true[]                  = "true";
static const char xi_senml_pat_false[]                 = "false";
static const char xi_senml_pat_entry_open_next[]       = ",{";
static const char xi_senml_pat_entry_open_elem_count[] = "{";
static const char xi_senml_pat_entry_name[]            = "\"n\":";
static const char xi_senml_pat_entry_units[]           = "\"u\":";
static const char xi_senml_pat_entry_float_value[]     = "\"v\":";
static const char xi_senml_pat_entry_boolean_value[]   = "\"bv\":";
static const char xi_senml_pat_entry_string_value[]    = "\"sv\":";
static const char xi_senml_pat_entry_time[]            = "\"t\":";
static const char xi_senml_pat_entry_update_time[]     = "\"ut\":";
static const char xi_senml_pat_entry_close[]           = "}";

xi_state_t xi_senml_json_serialize_init( xi_data_desc_t* out )
{
    assert( out != 0 );

    return xi_data_desc_append_data_resize( out, xi_senml_pat_open,
                                            sizeof( xi_senml_pat_open ) - 1 );
}

xi_state_t xi_senml_json_serialize_key( xi_data_desc_t* out,
                                        const char* const key,
                                        uint16_t elem_count )
{
    assert( out != 0 );
    assert( key != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    if ( elem_count > 0 )
    {
        XI_CHECK_STATE( ret_state = xi_data_desc_append_data_resize(
                            out, xi_senml_pat_coln, sizeof( xi_senml_pat_coln ) - 1 ) );
    }

    XI_CHECK_STATE( ret_state =
                        xi_data_desc_append_data_resize( out, key, strlen( key ) ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_string( xi_data_desc_t* out, const char* const string )
{
    assert( out != 0 );
    assert( string != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_data_desc_append_data_resize(
                        out, xi_senml_pat_quot, sizeof( xi_senml_pat_quot ) - 1 ) );

    XI_CHECK_STATE(
        ret_state = xi_data_desc_append_data_resize( out, string, strlen( string ) ) );

    XI_CHECK_STATE( ret_state = xi_data_desc_append_data_resize(
                        out, xi_senml_pat_quot, sizeof( xi_senml_pat_quot ) - 1 ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_float( xi_data_desc_t* out, const float value )
{
    assert( out != 0 );

    char buf[32] = {'\0'};

    int ret = snprintf( buf, sizeof( buf ), "%.5g", value );

    if ( ret <= 0 )
    {
        return XI_SERIALIZATION_ERROR;
    }

#if 0 /* @TODO to further discussion */
    if( ret <= sizeof( buf ) )
    {
        return XI_TRUNCATION_WARNING;
    }
#endif

    return xi_data_desc_append_data_resize( out, buf, ret );
}

xi_state_t xi_senml_json_serialize_int( xi_data_desc_t* out, const uint32_t value )
{
    assert( out != 0 );

    char buf[32] = {'\0'};

    int ret = snprintf( buf, sizeof( buf ), "%d", value );

    if ( ret <= 0 )
    {
        return XI_SERIALIZATION_ERROR;
    }

#if 0 /* @TODO to further discussion */
    if( ret <= sizeof( buf ) )
    {
        return XI_TRUNCATION_WARNING;
    }
#endif

    return xi_data_desc_append_data_resize( out, buf, ret );
}

xi_state_t xi_senml_json_serialize_boolean( xi_data_desc_t* out, const uint8_t boolean )
{
    assert( out != 0 );

    if ( boolean > 0 )
    {
        return xi_data_desc_append_data_resize( out, xi_senml_pat_true,
                                                sizeof( xi_senml_pat_true ) - 1 );
    }

    return xi_data_desc_append_data_resize( out, xi_senml_pat_false,
                                            sizeof( xi_senml_pat_false ) - 1 );
}

xi_state_t
xi_senml_json_serialize_name( xi_data_desc_t* out, const char* name, uint16_t elem_count )
{
    assert( out != 0 );
    assert( name != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    /* elem_count part serialize the key value */
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key( out, xi_senml_pat_entry_name,
                                                             elem_count ) );

    /* then the value itself */
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_string( out, name ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_float_value( xi_data_desc_t* out,
                                                const float value,
                                                uint16_t elem_count )
{
    assert( out != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key(
                        out, xi_senml_pat_entry_float_value, elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_float( out, value ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_string_value( xi_data_desc_t* out,
                                                 const char* string,
                                                 uint16_t elem_count )
{
    assert( out != 0 );
    assert( string != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key(
                        out, xi_senml_pat_entry_string_value, elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_string( out, string ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_boolean_value( xi_data_desc_t* out,
                                                  const uint8_t boolean,
                                                  uint16_t elem_count )
{
    assert( out != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key(
                        out, xi_senml_pat_entry_boolean_value, elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_boolean( out, boolean ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_units( xi_data_desc_t* out,
                                          const char* units,
                                          uint16_t elem_count )
{
    assert( out != 0 );
    assert( units != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key(
                        out, xi_senml_pat_entry_units, elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_string( out, units ) );

err_handling:
    return ret_state;
}

xi_state_t
xi_senml_json_serialize_time( xi_data_desc_t* out, int32_t time, uint16_t elem_count )
{
    assert( out != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key( out, xi_senml_pat_entry_time,
                                                             elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_int( out, time ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_update_time( xi_data_desc_t* out,
                                                int32_t time,
                                                uint16_t elem_count )
{
    assert( out != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key(
                        out, xi_senml_pat_entry_update_time, elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_int( out, time ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_base_name( xi_data_desc_t* out,
                                              const char* base_name,
                                              uint16_t elem_count )
{
    assert( out != 0 );
    assert( base_name != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    /* elem_count part serialize the key value */
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key( out, xi_senml_pat_base_name,
                                                             elem_count ) );

    /* then the value itself */
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_string( out, base_name ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_base_units( xi_data_desc_t* out,
                                               const char* base_units,
                                               uint16_t elem_count )
{
    assert( out != 0 );
    assert( base_units != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key( out, xi_senml_pat_base_units,
                                                             elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_string( out, base_units ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_base_time( xi_data_desc_t* out,
                                              int32_t base_time,
                                              uint16_t elem_count )
{
    assert( out != 0 );

    xi_state_t ret_state = XI_STATE_OK;

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_key( out, xi_senml_pat_base_time,
                                                             elem_count ) );
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_int( out, base_time ) );

err_handling:
    return ret_state;
}

xi_state_t xi_senml_json_serialize_close( xi_data_desc_t* out )
{
    assert( out != 0 );

    return xi_data_desc_append_data_resize( out, xi_senml_pat_close,
                                            sizeof( xi_senml_pat_close ) - 1 );
}

xi_state_t xi_senml_json_serialize_close_entries( xi_data_desc_t* out )
{
    assert( out != 0 );

    return xi_data_desc_append_data_resize( out, xi_senml_pat_entries_close,
                                            sizeof( xi_senml_pat_entries_close ) - 1 );
}

xi_state_t xi_senml_json_serialize_value_set( xi_data_desc_t* out,
                                              xi_senml_value_t* value_cnt,
                                              uint16_t elem_count )
{
    switch ( value_cnt->value_type )
    {
        case XI_SENML_VALUE_TYPE_STRING:
            return xi_senml_json_serialize_string_value(
                out, value_cnt->value.string_value, elem_count );
        case XI_SENML_VALUE_TYPE_FLOAT:
            return xi_senml_json_serialize_float_value( out, value_cnt->value.float_value,
                                                        elem_count );
        case XI_SENML_VALUE_TYPE_BOOLEAN:
            return xi_senml_json_serialize_boolean_value(
                out, value_cnt->value.boolean_value, elem_count );
        default:
            return XI_INVALID_PARAMETER;
    }
}

xi_state_t xi_senml_json_serialize_entry( xi_data_desc_t* out,
                                          xi_senml_entry_t* entry,
                                          uint32_t entry_count )
{
    xi_state_t ret_state = XI_STATE_OK;

    uint16_t fields_count = 0;

    /* open either with coln or without */
    if ( entry_count > 0 )
    {
        XI_CHECK_STATE( ret_state = xi_data_desc_append_data_resize(
                            out, xi_senml_pat_entry_open_next,
                            sizeof( xi_senml_pat_entry_open_next ) - 1 ) );
    }
    else
    {
        XI_CHECK_STATE( ret_state = xi_data_desc_append_data_resize(
                            out, xi_senml_pat_entry_open_elem_count,
                            sizeof( xi_senml_pat_entry_open_elem_count ) - 1 ) );
    }

    if ( entry->set.name_set == 1 )
    {
        XI_CHECK_STATE( ret_state = xi_senml_json_serialize_name( out, entry->name,
                                                                  fields_count++ ) );
    }

    if ( entry->set.value_set == 1 )
    {
        XI_CHECK_STATE( ret_state = xi_senml_json_serialize_value_set(
                            out, &entry->value_cnt, fields_count++ ) );
    }
    else
    {
        return XI_INVALID_PARAMETER;
    }

    if ( entry->set.time_set == 1 )
    {
        XI_CHECK_STATE( ret_state = xi_senml_json_serialize_time( out, entry->time,
                                                                  fields_count++ ) );
    }

    if ( entry->set.units_set == 1 )
    {
        XI_CHECK_STATE( ret_state = xi_senml_json_serialize_units( out, entry->units,
                                                                   fields_count++ ) );
    }

    if ( entry->set.update_time_set == 1 )
    {
        XI_CHECK_STATE( ret_state = xi_senml_json_serialize_update_time(
                            out, entry->update_time, fields_count++ ) );
    }

    XI_CHECK_STATE(
        ret_state = xi_data_desc_append_data_resize(
            out, xi_senml_pat_entry_close, sizeof( xi_senml_pat_entry_close ) - 1 ) );

err_handling:
    return ret_state;
}

xi_state_t
xi_senml_json_serialize( xi_data_desc_t** out_buffer, xi_senml_t* senml_structure )
{
    /* we are going to start with some 32 bytes allocation */
    static const uint32_t start_size = 32;

    xi_state_t ret_state = XI_STATE_OK;

    if ( out_buffer == 0 || senml_structure == 0 )
    {
        return XI_INVALID_PARAMETER;
    }

    xi_data_desc_t* dst = *out_buffer = xi_make_empty_desc_alloc( start_size );
    XI_CHECK_MEMORY( dst, ret_state );

    /* initialization of the senml buffer */
    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_init( dst ) );

    /* serialization of entities */
    {
        uint32_t entries_count  = 0;
        xi_senml_entry_t* entry = senml_structure->entries_list;

        while ( entry )
        {
            XI_CHECK_STATE( ret_state = xi_senml_json_serialize_entry( *out_buffer, entry,
                                                                       entries_count ) );
            entry = entry->__next;
            ++entries_count;
        }
    }

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_close_entries( dst ) );

    uint16_t elements_count = 1;

    if ( senml_structure->set.base_name_set == 1 )
    {
        xi_senml_json_serialize_base_name( dst, senml_structure->base_name,
                                           elements_count++ );
    }

    if ( senml_structure->set.base_time_set == 1 )
    {
        xi_senml_json_serialize_base_time( dst, senml_structure->base_time,
                                           elements_count++ );
    }

    if ( senml_structure->set.base_units_set == 1 )
    {
        xi_senml_json_serialize_base_units( dst, senml_structure->base_units,
                                            elements_count++ );
    }

    XI_CHECK_STATE( ret_state = xi_senml_json_serialize_close( dst ) );

err_handling:
    return ret_state;
}

#ifdef __cplusplus
}
#endif
