#include "tinytest.h"

#ifndef __XI_UTEST_BASIC_TESTCASE_FRAME_H__
#define __XI_UTEST_BASIC_TESTCASE_FRAME_H__

void* xi_utest_setup_basic( const struct testcase_t* testcase );
int xi_utest_teardown_basic( const struct testcase_t* testcase, void* fixture );

#endif
