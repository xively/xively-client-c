// Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.

#ifndef __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__
#define __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__

#include "xi_layer_macros.h"
#include "xi_driver_codec_protobuf_layer.h"
#include "xi_driver_logic_layer.h"
#include "xi_io_layer_ids.h"
#include "xi_layer_default_functions.h"

enum xi_driver_control_channel_layerchain_e
{
    XI_LAYER_TYPE_DRIVER_IO = 0,
    XI_LAYER_TYPE_DRIVER_CODEC,
    XI_LAYER_TYPE_DRIVER_LOGIC
};

#define XI_CONCATENATE_2( A, B ) A##B
#define XI_CONCATENATE( A, B ) XI_CONCATENATE_2( A, B )

// io layer selection
#if XI_IO_LAYER == XI_IO_POSIX
#include "xi_io_posix_layer.h"
#define XI_IO_LAYER_FUNCTION_PREFIX xi_io_posix_layer
#elif XI_IO_LAYER == XI_IO_DUMMY
#include "xi_io_dummy_layer.h"
#define XI_IO_LAYER_FUNCTION_PREFIX xi_io_dummy_layer
#elif XI_IO_LAYER == XI_IO_MICROCHIP
#include "xi_io_microchip_layer.h"
#define XI_IO_LAYER_FUNCTION_PREFIX xi_io_microchip_layer
#elif XI_IO_LAYER == XI_IO_BSP
#include "xi_io_net_layer.h"
#define XI_IO_LAYER_FUNCTION_PREFIX xi_io_net_layer
#endif

#define XI_DRIVER_CONTROL_CHANNEL_LAYER_CHAIN                                            \
    XI_LAYER_TYPE_DRIVER_IO                                                              \
    , XI_LAYER_TYPE_DRIVER_CODEC, XI_LAYER_TYPE_DRIVER_LOGIC

XI_DECLARE_LAYER_TYPES_BEGIN( xi_driver_control_channel_layerchain )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_DRIVER_IO,
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _push ),
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _pull ),
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _close ),
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _close_externally ),
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _init ),
                    XI_CONCATENATE( XI_IO_LAYER_FUNCTION_PREFIX, _connect ),
                    &xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_DRIVER_CODEC,
                      xi_driver_codec_protobuf_layer_push,
                      xi_driver_codec_protobuf_layer_pull,
                      xi_driver_codec_protobuf_layer_close,
                      xi_driver_codec_protobuf_layer_close_externally,
                      xi_driver_codec_protobuf_layer_init,
                      xi_driver_codec_protobuf_layer_connect,
                      &xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_DRIVER_LOGIC,
                        xi_driver_logic_layer_push,
                        xi_driver_logic_layer_pull,
                        xi_driver_logic_layer_close,
                        xi_driver_logic_layer_close_externally,
                        xi_driver_logic_layer_init,
                        xi_driver_logic_layer_connect,
                        &xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_DRIVER_CONTROL_CHANNEL,
                                       XI_DRIVER_CONTROL_CHANNEL_LAYER_CHAIN );

#endif // __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__
