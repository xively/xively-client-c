/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_SENML_MACROS_H__
#define __XIVELY_SENML_MACROS_H__

#define XI_SENML_ENTRY1( _1 ) _1

#define XI_SENML_ENTRY2( _1, _2 ) _1, _2

#define XI_SENML_ENTRY3( _1, _2, _3 ) _1, _2, _3

#define XI_SENML_ENTRY4( _1, _2, _3, _4 ) _1, _2, _3, _4

#define XI_SENML_ENTRY5( _1, _2, _3, _4, _5 ) _1, _2, _3, _4, _5

#define XI_SENML_ENTRY_ERROR( ... ) error wrong parameter count

#define XI_SENML_ENTRY_IMPL( _0, _1, _2, _3, _4, N, ... ) N

/**
 * @name XI_CREATE_SENML_EMPTY_STRUCT
 * @brief Creates a senml struct with no base name, time, units set.
 *
 * @param [out] state the return state of the action, XI_STATE_OK in case of success
 * @param [out] senml_ptr a pointer on a xi_senml_t structure, this will point on
 *                        the newly allocated structure
 */
#define XI_CREATE_SENML_EMPTY_STRUCT( state, senml_ptr )                                 \
    state = xi_create_senml_struct( &senml_ptr, 0 );

/**
 * @name XI_CREATE_SENML_STRUCT
 * @brief Creates a senml struct with at least one set from base name, unit and time.
 *
 * @param [out] state the return state of the action, XI_STATE_OK in case of success
 * @param [out] senml_ptr a pointer on a xi_senml_t structure, this will point on
 *                        the newly allocated structure
 * @param [in] ... base field descriptors at most 3: base name, unit, time. Use the
 *                 XI_SENML_BASE_* macros to fill in these parameters. At least one
 *                 must be provided otherwise use the XI_CREATE_SENML_EMPTY_STRUCT
 *                 macro if no base values are required.
 */
#define XI_CREATE_SENML_STRUCT( state, senml_ptr, ... )                                  \
    state = xi_create_senml_struct(                                                      \
        &senml_ptr,                                                                      \
        XI_SENML_ENTRY_IMPL( __VA_ARGS__, XI_SENML_ENTRY_ERROR( __VA_ARGS__ ),           \
                             XI_SENML_ENTRY_ERROR( __VA_ARGS__ ), 3, 2, 1 ),             \
        XI_SENML_ENTRY_IMPL(                                                             \
            __VA_ARGS__, XI_SENML_ENTRY_ERROR( __VA_ARGS__ ),                            \
            XI_SENML_ENTRY_ERROR( __VA_ARGS__ ), XI_SENML_ENTRY3( __VA_ARGS__ ),         \
            XI_SENML_ENTRY2( __VA_ARGS__ ), XI_SENML_ENTRY1( __VA_ARGS__ ) ) );

/**
 * @name XI_ADD_SENML_ENTRY
 * @brief Adds an entry to an existing senml structure.
 *
 * @param [out] state the return state of the action, XI_STATE_OK in case of success
 * @param [in] senml_ptr a pointer to an existing xi_senml_t structure, entry will
 *                       be added to this structure
 * @param [in] ... entry field descriptors at most 5: name, unit, value, time and
 *                 update_time. To generate a parameter here use macros
 *                 XI_SENML_ENTRY_* and XI_SENML_ENTRY_*_VALUE.
 */
#define XI_ADD_SENML_ENTRY( state, senml_ptr, ... )                                      \
    state = xi_add_senml_entry(                                                          \
        senml_ptr, XI_SENML_ENTRY_IMPL( __VA_ARGS__, 5, 4, 3, 2, 1 ),                    \
        XI_SENML_ENTRY_IMPL(                                                             \
            __VA_ARGS__, XI_SENML_ENTRY5( __VA_ARGS__ ), XI_SENML_ENTRY4( __VA_ARGS__ ), \
            XI_SENML_ENTRY3( __VA_ARGS__ ), XI_SENML_ENTRY2( __VA_ARGS__ ),              \
            XI_SENML_ENTRY1( __VA_ARGS__ ) ) );

#define XI_SENML_BASE_NAME( bn )                                                         \
    ( ( xi_senml_t ){.base_name = bn, .set.base_name_set = 1} )

#define XI_SENML_BASE_TIME( bt )                                                         \
    ( ( xi_senml_t ){.base_time = bt, .set.base_time_set = 1} )

#define XI_SENML_BASE_UNITS( bu )                                                        \
    ( ( xi_senml_t ){.base_units = bu, .set.base_units_set = 1} )

#define XI_SENML_ENTRY_NAME( n ) ( ( xi_senml_entry_t ){.name = n, .set.name_set = 1} )

#define XI_SENML_ENTRY_UNITS( u ) ( ( xi_senml_entry_t ){.units = u, .set.units_set = 1} )

#define XI_SENML_ENTRY_FLOAT_VALUE( v )                                                  \
    ( ( xi_senml_entry_t ){.value_cnt.value.float_value = v,                             \
                           .value_cnt.value_type        = XI_SENML_VALUE_TYPE_FLOAT,     \
                           .set.value_set               = 1} )

#define XI_SENML_ENTRY_BOOLEAN_VALUE( bv )                                               \
    ( ( xi_senml_entry_t ){.value_cnt.value.boolean_value = bv,                          \
                           .value_cnt.value_type          = XI_SENML_VALUE_TYPE_BOOLEAN, \
                           .set.value_set                 = 1} )

#define XI_SENML_ENTRY_STRING_VALUE( sv )                                                \
    ( ( xi_senml_entry_t ){.value_cnt.value.string_value = sv,                           \
                           .value_cnt.value_type         = XI_SENML_VALUE_TYPE_STRING,   \
                           .set.value_set                = 1} )

#define XI_SENML_ENTRY_TIME( t ) ( ( xi_senml_entry_t ){.time = t, .set.time_set = 1} )

#define XI_SENML_ENTRY_UPDATE_TIME( ut )                                                 \
    ( ( xi_senml_entry_t ){.update_time = ut, .set.update_time_set = 1} )

#endif /* __XIVELY_SENML_MACROS_H__ */
