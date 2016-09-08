// Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.

#define XI_MOCK_TEST_PREPROCESSOR_RUN
#include "xi_itest_clean_session.h"
#include "xi_itest_tls_error.h"
#ifndef XI_DEBUG_NO_TLS
#include "xi_itest_tls_layer.h"
#endif
#include "xi_itest_mqttlogic_layer.h"
#undef XI_MOCK_TEST_PREPROCESSOR_RUN

#include "xi_test_utils.h"
#include "xi_lamp_communication.h"

struct CMGroupTest groups[] = {
    cmocka_test_group( xi_itests_clean_session ), cmocka_test_group( xi_itests_tls_error )
#ifndef XI_DEBUG_NO_TLS
                                                      ,
    cmocka_test_group( xi_itests_tls_layer )
#endif
        ,
    cmocka_test_group( xi_itests_mqttlogic_layer ), cmocka_test_group_end};

int8_t xi_cm_strict_mock = 0;

#ifndef XI_EMBEDDED_TESTS
int main( int argc, char const* argv[] )
#else
int xi_itests_main( int argc, char const* argv[] )
#endif
{
    xi_test_init( argc, argv );

    // report test start
    xi_test_report_result( xi_test_load_level ? "xi_itest_id_l1" : "xi_itest_id_l0",
                           xi_test_load_level ? "xi1" : "xi", 1, 0 );

    const int number_of_failures = cmocka_run_test_groups( groups );

    // report test start
    xi_test_report_result( xi_test_load_level ? "xi_itest_id_l1" : "xi_itest_id_l0",
                           xi_test_load_level ? "xi1" : "xi", 0, number_of_failures );

    return number_of_failures;
}
