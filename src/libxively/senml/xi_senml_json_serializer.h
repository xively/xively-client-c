/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_SENML_JSON_SERIALIZER_H__
#define __XI_SENML_JSON_SERIALIZER_H__

#include "xi_data_desc.h"
#include "xively_senml_types.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern xi_state_t xi_senml_json_serialize_init( xi_data_desc_t* out );

extern xi_state_t xi_senml_json_serialize_key( xi_data_desc_t* out,
                                               const char* const key,
                                               uint16_t elem_count );

extern xi_state_t
xi_senml_json_serialize_string( xi_data_desc_t* out, const char* const string );

extern xi_state_t xi_senml_json_serialize_float( xi_data_desc_t* out, const float value );

extern xi_state_t
xi_senml_json_serialize_int( xi_data_desc_t* out, const uint32_t value );

extern xi_state_t
xi_senml_json_serialize_boolean( xi_data_desc_t* out, const uint8_t boolean );

extern xi_state_t xi_senml_json_serialize_name( xi_data_desc_t* out,
                                                const char* name,
                                                uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_float_value( xi_data_desc_t* out,
                                                       const float value,
                                                       uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_string_value( xi_data_desc_t* out,
                                                        const char* string,
                                                        uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_boolean_value( xi_data_desc_t* out,
                                                         const uint8_t boolean,
                                                         uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_units( xi_data_desc_t* out,
                                                 const char* units,
                                                 uint16_t elem_count );

extern xi_state_t
xi_senml_json_serialize_time( xi_data_desc_t* out, int32_t time, uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_update_time( xi_data_desc_t* out,
                                                       int32_t time,
                                                       uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_base_name( xi_data_desc_t* out,
                                                     const char* base_name,
                                                     uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_base_units( xi_data_desc_t* out,
                                                      const char* base_units,
                                                      uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_base_time( xi_data_desc_t* out,
                                                     int32_t base_time,
                                                     uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_close_entries( xi_data_desc_t* out );
extern xi_state_t xi_senml_json_serialize_close( xi_data_desc_t* out );

extern xi_state_t xi_senml_json_serialize_value_set( xi_data_desc_t* out,
                                                     xi_senml_value_t* value_cnt,
                                                     uint16_t elem_count );

extern xi_state_t xi_senml_json_serialize_entry( xi_data_desc_t* out,
                                                 xi_senml_entry_t* entry,
                                                 uint32_t entry_count );

extern xi_state_t
xi_senml_json_serialize( xi_data_desc_t** out_buffer, xi_senml_t* senml_structure );

#ifdef __cplusplus
}
#endif

#endif /* __XI_SENML_JSON_SERIALIZER_H__ */
