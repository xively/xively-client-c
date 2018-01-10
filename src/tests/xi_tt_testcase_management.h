/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stddef.h>

/**
 * Xi TinyTest extension macros to ease usage of TinyTest framework.
 * Twofolded precompiler run is used to define the test functions (test cases) and the TT
 * test group structure.
 */
#ifdef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN // TT test group structure
                                                         // generation mode

#undef XI_TT_TESTGROUP_BEGIN
#define XI_TT_TESTGROUP_BEGIN( testgroupname ) struct testcase_t testgroupname[] = {
#undef XI_TT_TESTGROUP_END
#define XI_TT_TESTGROUP_END                                                              \
    END_OF_TESTCASES                                                                     \
    }                                                                                    \
    ;

#undef XI_TT_TESTCASE
#define XI_TT_TESTCASE( testcasename, ... )                                              \
    {#testcasename, testcasename, TT_ENABLED_, 0, 0},

#undef XI_TT_TESTCASE_WITH_SETUP
#define XI_TT_TESTCASE_WITH_SETUP( testcasename, setupfnname, cleanfnname, setup_data,   \
                                   ... )                                                 \
    {#testcasename, testcasename, TT_ENABLED_, &testcasename##setupfnname##cleanfnname,  \
     setup_data},

#undef SKIP_XI_TT_TESTCASE
#define SKIP_XI_TT_TESTCASE( testcasename, ... )                                         \
    {#testcasename, testcasename, TT_SKIP, 0, 0},

#else // XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN, TT test case definition mode

#undef XI_TT_TESTGROUP_BEGIN
#define XI_TT_TESTGROUP_BEGIN( testgroupname )

#undef XI_TT_TESTGROUP_END
#define XI_TT_TESTGROUP_END

#undef XI_TT_TESTCASE
#define XI_TT_TESTCASE( testcasename, ... ) void testcasename() __VA_ARGS__

#undef XI_TT_TESTCASE_WITH_SETUP
#define XI_TT_TESTCASE_WITH_SETUP( testcasename, setupfnname, cleanfnname, setup_data,   \
                                   ... )                                                 \
                                                                                         \
    void testcasename() __VA_ARGS__;                                                     \
                                                                                         \
    const struct testcase_setup_t testcasename##setupfnname##cleanfnname = {             \
        setupfnname, cleanfnname};

#undef SKIP_XI_TT_TESTCASE
#define SKIP_XI_TT_TESTCASE( testcasename, ... ) void testcasename() __VA_ARGS__

#endif // XI_TT_TESTCASE_ENUMERATION

extern char xi_test_load_level;
