/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_TEST_UTILS_H__
#define __XI_TEST_UTILS_H__

char xi_test_load_level = 0;

#ifndef XI_EMBEDDED_TESTS
/* this is platform specific code */
#include <unistd.h>
#endif /* not XI_EMBEDDED_TESTS */

void xi_test_init( int argc, char const* argv[] )
{
#ifndef XI_EMBEDDED_TESTS
    int result_getopt = 0;

    /* this is platform specific code */
    opterr = 0;
    while ( ( result_getopt = getopt( argc, ( char* const* )argv, "l:" ) ) != -1 )
    {
        switch ( result_getopt )
        {
            case 'l':
                xi_test_load_level = *optarg - '0';
                break;
        }
    }

// printf("*** xi_test_load_level = %d\n", xi_test_load_level);
#endif /* not XI_EMBEDDED_TESTS */
}

#endif /* __XI_TEST_UTILS_H__ */
