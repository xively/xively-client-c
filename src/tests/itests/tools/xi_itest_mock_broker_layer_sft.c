/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_itest_mock_broker_layer_sft.h"
#include <stdio.h>

void xi_mock_broker_sft_on_message()
{
    printf( "--- control topic\n" );
}
