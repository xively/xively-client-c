/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */
#ifndef __XI_FACTORY_CONF_H__
#define __XI_FACTORY_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xi_layer_factory_interface.h"

#define SIZE_SUFFIX _SIZE

#define XI_DECLARE_LAYER_CHAIN_SCHEME( name, args )                                      \
    static xi_layer_type_id_t name[] = {args};                                           \
    static size_t name##SIZE_SUFFIX  = sizeof( name ) / sizeof( xi_layer_type_id_t )

#define XI_LAYER_CHAIN_SCHEME_LENGTH( name ) name##SIZE_SUFFIX

#define XI_DECLARE_LAYER_TYPES_BEGIN( name ) static xi_layer_type_t name[] = {
#define XI_LAYER_TYPES_ADD( type_id, push, pull, close, close_externally, init, connect, \
                            post_connect )                                               \
    {                                                                                    \
        type_id,                                                                         \
        {                                                                                \
            push, pull, close, close_externally, init, connect, post_connect             \
        }                                                                                \
    }

#define XI_DECLARE_LAYER_TYPES_END()                                                     \
    }                                                                                    \
    ;

#ifdef __cplusplus
}
#endif

#endif /* __XI_FACTORY_CONF_H__ */
