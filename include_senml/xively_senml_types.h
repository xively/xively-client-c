/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_SENML_TYPES_H__
#define __XIVELY_SENML_TYPES_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xi_senml_set_s
{
    uint8_t base_time_set : 1;
    uint8_t base_name_set : 1;
    uint8_t base_units_set : 1;
} xi_senml_set_t;

typedef struct xi_senml_entry_set_s
{
    uint8_t name_set : 1;
    uint8_t units_set : 1;
    uint8_t value_set : 1;
    uint8_t time_set : 1;
    uint8_t update_time_set : 1;
} xi_senml_entry_set_t;

typedef enum xi_senml_value_type_e {
    XI_SENML_VALUE_TYPE_FLOAT,
    XI_SENML_VALUE_TYPE_STRING,
    XI_SENML_VALUE_TYPE_BOOLEAN
} xi_senml_value_type_t;

typedef union xi_senml_value_u {
    float float_value;
    char* string_value;
    uint8_t boolean_value;
    float value_sum;
} xi_senml_value_ut;

typedef struct xi_senml_value_s
{
    xi_senml_value_ut value;
    xi_senml_value_type_t value_type;
} xi_senml_value_t;

typedef struct xi_senml_entry_s
{
    char* name;
    char* units;
    xi_senml_value_t value_cnt;
    int32_t time;
    int32_t update_time;

    xi_senml_entry_set_t set;
    struct xi_senml_entry_s* __next;
} xi_senml_entry_t;

typedef struct xi_senml_s
{
    char* base_name;
    int32_t base_time;
    char* base_units;
    xi_senml_set_t set;
    struct xi_senml_entry_s* entries_list;
} xi_senml_t;

#ifdef __cplusplus
}
#endif

#endif /* __XIVELY_SENML_ENTRY_H__ */
