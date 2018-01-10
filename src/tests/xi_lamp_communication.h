/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_LAMP_COMUNNICATION_H__
#define __XI_LAMP_COMUNNICATION_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XI_BUILD_OSX

static const char XI_TEST_RESULT_COMMAND_TEMPLATE[] =
    "open -g \"unittest-notifier://xi.unittest/{"
    "\\\"id\\\":\\\"%s\\\","
    "\\\"initiale\\\":\\\"%s\\\","
    "\\\"testdetailsfile\\\":\\\"none\\\","
    "\\\"running\\\":\\\"%s\\\","
    "\\\"passed\\\":\\\"%s\\\"}\"";

void xi_test_report_result( const char* const test_id,
                            const char* const initiale,
                            const uint8_t running,
                            uint8_t failed )
{
    char test_result_command_buffer[1000] = {0};

    snprintf( test_result_command_buffer, sizeof( test_result_command_buffer ),
              XI_TEST_RESULT_COMMAND_TEMPLATE, test_id, initiale,
              running ? "true" : "false", failed ? "false" : "true" );

    system( test_result_command_buffer );
}

#else
#define xi_test_report_result( test_id, initiale, running, failed )
#endif

#ifdef __cplusplus
}
#endif

#endif /* __XI_LAMP_COMUNNICATION_H__ */
