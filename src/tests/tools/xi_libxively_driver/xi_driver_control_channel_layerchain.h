// Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#ifndef __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__
#define __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__

#include "xi_driver_codec_protobuf_layer.h"
#include "xi_driver_logic_layer.h"
#include "xi_layer_default_functions.h"
#include "xi_layer_macros.h"

enum xi_driver_control_channel_layerchain_e
{
    XI_LAYER_TYPE_DRIVER_IO = 0,
    XI_LAYER_TYPE_DRIVER_CODEC,
    XI_LAYER_TYPE_DRIVER_LOGIC
};

// io layer selection
#include "xi_io_net_layer.h"

#define XI_DRIVER_CONTROL_CHANNEL_LAYER_CHAIN                                            \
    XI_LAYER_TYPE_DRIVER_IO                                                              \
    , XI_LAYER_TYPE_DRIVER_CODEC, XI_LAYER_TYPE_DRIVER_LOGIC

XI_DECLARE_LAYER_TYPES_BEGIN( xi_driver_control_channel_layerchain )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_DRIVER_IO,
                    xi_io_net_layer_push,
                    xi_io_net_layer_pull,
                    xi_io_net_layer_close,
                    xi_io_net_layer_close_externally,
                    xi_io_net_layer_init,
                    xi_io_net_layer_connect,
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

#endif /* __XI_DRIVER_CONTROL_CHANNEL_LAYERCHAIN_H__ */
