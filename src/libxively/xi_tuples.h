/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TUPLES_H__
#define __XI_TUPLES_H__

#include "xi_tuple.h"
#include "xi_mqtt_message.h"

/** \tuple xi_even_handle_and_void_t may not be used for now */
/* XI_DEF_TUPLE_TYPE( xi_even_handle_and_void_t, xi_event_handle_t, void* ) */

XI_DEF_TUPLE_TYPE( xi_mqtt_written_data_t, uint16_t, xi_mqtt_type_t )

/**
 *  \tuple xi_uint32_and_void_t will be used to differantiate between different
 *          io calls so that the callers of io layer know which call has been
 * finished
 */
/* XI_DEF_TUPLE_TYPE( xi_uint16_and_void_t, uint16_t, void* ) */

/**
 * \tuple xi_uint16_and_uint16_t is used via the sending system to be able
 *          to process sending of msgs related to the given
 *          msg id via coroutines
 */
/* XI_DEF_TUPLE_TYPE( xi_uint16_and_uint16_t, uint16_t, uint16_t ) */

#endif /* __XI_TUPLES_H__ */
