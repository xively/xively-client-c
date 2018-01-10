/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_ITEST_LAYERCHAIN_TLS_LAYER_H__
#define __XI_ITEST_LAYERCHAIN_TLS_LAYER_H__

#include "xi_layer_macros.h"

#include "xi_mock_layer_tls_prev.h"
#include "xi_tls_layer.h"
#include "xi_mock_layer_tls_next.h"
#include "xi_layer_default_functions.h"

enum xi_tls_layer_stack_order_e
{
    XI_LAYER_TYPE_MOCK_TLS_PREV = 0,
    XI_LAYER_TYPE_SUT_TLS,
    XI_LAYER_TYPE_MOCK_TLS_NEXT
};

#define XI_TLS_LAYER_CHAIN                                                               \
    XI_LAYER_TYPE_MOCK_TLS_PREV                                                          \
    , XI_LAYER_TYPE_SUT_TLS, XI_LAYER_TYPE_MOCK_TLS_NEXT

XI_DECLARE_LAYER_TYPES_BEGIN( itest_layer_chain_tls )
XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_TLS_PREV,
                    xi_mock_layer_tls_prev_push,
                    xi_mock_layer_tls_prev_pull,
                    xi_mock_layer_tls_prev_close,
                    xi_mock_layer_tls_prev_close_externally,
                    xi_mock_layer_tls_prev_init,
                    xi_mock_layer_tls_prev_connect,
                    xi_layer_default_post_connect )
, XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_SUT_TLS,
                      xi_tls_layer_push,
                      xi_tls_layer_pull,
                      xi_tls_layer_close,
                      xi_tls_layer_close_externally,
                      xi_tls_layer_init,
                      xi_tls_layer_connect,
                      xi_layer_default_post_connect ),
    XI_LAYER_TYPES_ADD( XI_LAYER_TYPE_MOCK_TLS_NEXT,
                        xi_mock_layer_tls_next_push,
                        xi_mock_layer_tls_next_pull,
                        xi_mock_layer_tls_next_close,
                        xi_mock_layer_tls_next_close_externally,
                        xi_mock_layer_tls_next_init,
                        xi_mock_layer_tls_next_connect,
                        xi_layer_default_post_connect ) XI_DECLARE_LAYER_TYPES_END()

        XI_DECLARE_LAYER_CHAIN_SCHEME( XI_LAYER_CHAIN_TLS, XI_TLS_LAYER_CHAIN );

#endif /* __XI_ITEST_LAYERCHAIN_TLS_LAYER_H__ */
