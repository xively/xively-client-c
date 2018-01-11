/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XIVELY_SENML_H__
#define __XIVELY_SENML_H__

#include <xively_error.h>

#include <xively_senml_macros.h>
#include <xively_senml_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name xi_senml_serialize
 * @brief Creates a JSON string of a senml structure.
 *
 * Example usage:
 *
 *     xi_senml_t* senml_structure = 0;
 *     xi_state_t state = XI_STATE_OK;
 *
 *     XI_CREATE_SENML_STRUCT(
 *       state, senml_structure,
 *       XI_SENML_BASE_NAME( "thermostat" ),
 *       XI_SENML_BASE_UNITS( "F" ),
 *       XI_SENML_BASE_TIME( 23 ) );
 *
 *     XI_ADD_SENML_ENTRY(
 *       state, senml_structure,
 *       XI_SENML_ENTRY_NAME( "kitchen" ),
 *       XI_SENML_ENTRY_FLOAT_VALUE( 62.02 ) );
 *
 *     XI_ADD_SENML_ENTRY(
 *       state, senml_structure,
 *       XI_SENML_ENTRY_NAME( "kitchen" ),
 *       XI_SENML_ENTRY_FLOAT_VALUE( 61.02 ),
 *       XI_SENML_ENTRY_TIME( 25 ) );
 *
 *     XI_ADD_SENML_ENTRY(
 *       state, senml_structure,
 *       XI_SENML_ENTRY_NAME( "heating" ),
 *       XI_SENML_ENTRY_TIME( 25 ),
 *       XI_SENML_ENTRY_BOOLEAN_VALUE( 0 ),
 *       XI_SENML_ENTRY_UNITS( "[ON/OFF]" ) );
 *
 *
 *     uint8_t* out_buffer = NULL;
 *     uint32_t out_size = 0;
 *
 *     state = xi_senml_serialize( senml_structure, &out_buffer, &out_size );
 *
 * This function creates a JSON string from the macro generated xi_senml_t structure.
 * Allocates memory for output buffer, allocation size also returned. Memory should be
 * released by the corresponding free buffer function.
 *
 * @param [in] senml_structure the structure to be converted into its string
 *                             represenation
 * @param [out] out_buffer this buffer will contain the string. Required memory is
 *                         allocated inside. Output is not zero terminated string.
 * @param [out] out_size the allocation size equals with the string lenght (trailing zero
 *                       NOT included since it isn't always there)
 *
 * @retval XI_STATE_OK if succeeded, other in case of failure,
 *         see xively_error.h for error codes
 */
extern xi_state_t xi_senml_serialize( xi_senml_t* senml_structure,
                                      uint8_t** out_buffer,
                                      uint32_t* out_size );

/**
 * @name xi_create_senml_struct
 * @brief Allocates and initializes a senml structure, recommended usage through the API
 *        macros XI_CREATE_SENML_STRUCT or XI_CREATE_SENML_EMPTY_STRUCT.
 *
 * Please try to avoid direct call this function use macros instead. Takes up to
 * 3 parameters which set base name, base time and base unit.
 *
 * @param [out] senml_ptr this parameter will point to the allocated senml structure
 * @param [in]  count number of following parameters, filled properly by API macro
 * @param [in]  ... base value descriptors usually created by XI_SENML_BASE_*
 *
 * @retval XI_STATE_OK if succeeded, other in case of failure,
 *         see xively_error.h for error codes
 */
extern xi_state_t xi_create_senml_struct( xi_senml_t** senml_ptr, int count, ... );

/**
 * @name xi_add_senml_entry
 * @brief Creates an entry in an existing senml structure, recommended usage
 *        through the API macro XI_ADD_SENML_ENTRY.
 *
 * Please try to avoid direct call this function use macros instead. Takes up to
 * 5 parameters which set name, unit, value, time and update time.
 *
 * @param [in] senml_structure entry is added to this structure
 * @param [in] count number of following parameters, filled properly by API macro
 * @param [in] ... entry field descriptors usually created by XI_SENML_ENTRY_* and
 *                 XI_SENML_ENTRY_*_VALUE
 *
 * @retval XI_STATE_OK if succeeded, other in case of failure,
 *         see xively_error.h for error codes
 */
extern xi_state_t xi_add_senml_entry( xi_senml_t* senml_ptr, int count, ... );

/**
 * @name xi_senml_free_buffer
 * @brief Releases the buffer memory allocated by the serializer function.
 *
 * @param [in] buffer the buffer to be freed
 *
 * @retval XI_STATE_OK if succeeded, other in case of failure,
 *         see xively_error.h for error codes
 */
extern xi_state_t xi_senml_free_buffer( uint8_t** buffer );

/**
 * @name xi_senml_destroy
 * @brief Releases the senml structure created by macros.
 *
 * @param [in] senml_structure the structure to be freed
 *
 * @retval XI_STATE_OK if succeeded, other in case of failure,
 *         see xively_error.h for error codes
 */
extern void xi_senml_destroy( xi_senml_t** senml_structure );

#ifdef __cplusplus
{
#endif

#endif /* __XIVELY_SENML_H__ */
