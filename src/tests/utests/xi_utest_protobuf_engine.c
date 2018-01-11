/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

#include "xi_debug.h"
#include "control_topic.pb-c.h"
#include "xi_types.h"

#include "xi_memory_checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN

/////////////////////////////////////////////////////////////////////
void* xi_utest_protobuf_alloc( void* data, size_t len )
{
    XI_UNUSED( data );
    return xi_alloc( len );
}

void xi_utest_protobuf_free( void* data, void* ptr )
{
    XI_UNUSED( data );
    xi_free( ptr );
    return;
}

ProtobufCAllocator xi_utest_protobuf_engine_allocator = {&xi_utest_protobuf_alloc,
                                                         &xi_utest_protobuf_free, NULL};

/////////////////////////////////////////////////////////////////////
uint8_t xi_utest_protobuf_engine_checksum_data[] = "checksum";

#endif

XI_TT_TESTGROUP_BEGIN( utest_protobuf_engine )

XI_TT_TESTCASE(
    utest__xi_protobuf_engine__correct_data__serialise_credentials_deserialise, {
        CTRPC rpc_message             = CT__RPC__INIT;
        CTRPC__Header rpc_header      = CT__RPC__HEADER__INIT;
        CTRPC__SndCreds rpc_snd_creds = CT__RPC__SND_CREDS__INIT;

        rpc_message.header   = &rpc_header;
        rpc_message.sndcreds = &rpc_snd_creds;

        rpc_header.functype      = CT__RPC__FUNC_TYPE__SND_CREDS;
        rpc_header.msgid         = "1234";
        rpc_header.checksum.data = xi_utest_protobuf_engine_checksum_data;
        rpc_header.checksum.len  = sizeof( xi_utest_protobuf_engine_checksum_data );

        rpc_snd_creds.cred = "test credentials";

        size_t len = ct__rpc__get_packed_size( &rpc_message );
        uint8_t packed_message[len];
        size_t bytes = ct__rpc__pack( &rpc_message, packed_message );

        tt_want_int_op( len, ==, bytes );

        CTRPC* rpc_unpacked =
            ct__rpc__unpack( &xi_utest_protobuf_engine_allocator, bytes, packed_message );

        // check header
        tt_want_int_op( rpc_message.header->checksum.len, ==,
                        rpc_unpacked->header->checksum.len );
        tt_want_int_op( memcmp( rpc_message.header->checksum.data,
                                rpc_unpacked->header->checksum.data,
                                rpc_unpacked->header->checksum.len ),
                        ==, 0 );

        tt_want_int_op( rpc_message.header->functype, ==,
                        rpc_unpacked->header->functype );
        tt_want_str_op( rpc_message.header->msgid, ==, rpc_unpacked->header->msgid );

        // check credentials
        tt_want_ptr_op( rpc_unpacked->sndcreds, !=, NULL );
        tt_want_str_op( rpc_message.sndcreds->cred, ==, rpc_unpacked->sndcreds->cred );

        ct__rpc__free_unpacked( rpc_unpacked, &xi_utest_protobuf_engine_allocator );

        tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
    } )

XI_TT_TESTCASE( utest__xi_protobuf_engine__correct_data__serialise_response_deserialise, {
    CTRPC rpc_message            = CT__RPC__INIT;
    CTRPC__Header rpc_header     = CT__RPC__HEADER__INIT;
    CTRPC__Response rpc_response = CT__RPC__RESPONSE__INIT;

    rpc_message.header   = &rpc_header;
    rpc_message.response = &rpc_response;

    rpc_header.functype      = CT__RPC__FUNC_TYPE__RESPONSE;
    rpc_header.msgid         = "1234";
    rpc_header.checksum.data = xi_utest_protobuf_engine_checksum_data;
    rpc_header.checksum.len  = sizeof( xi_utest_protobuf_engine_checksum_data );

    rpc_response.result    = CT__RPC__FUNC_RESULT__OK;
    rpc_response.resultmsg = "Team Left Shark!";

    size_t len = ct__rpc__get_packed_size( &rpc_message );
    uint8_t packed_message[len];
    size_t bytes = ct__rpc__pack( &rpc_message, packed_message );

    tt_want_int_op( len, ==, bytes );

    CTRPC* rpc_unpacked =
        ct__rpc__unpack( &xi_utest_protobuf_engine_allocator, bytes, packed_message );

    // check header
    tt_want_int_op( rpc_message.header->checksum.len, ==,
                    rpc_unpacked->header->checksum.len );
    tt_want_int_op( memcmp( rpc_message.header->checksum.data,
                            rpc_unpacked->header->checksum.data,
                            rpc_unpacked->header->checksum.len ),
                    ==, 0 );

    tt_want_int_op( rpc_message.header->functype, ==, rpc_unpacked->header->functype );
    tt_want_str_op( rpc_message.header->msgid, ==, rpc_unpacked->header->msgid );

    // check response
    tt_want_ptr_op( rpc_unpacked->response, !=, NULL );
    tt_want_int_op( rpc_message.response->result, ==, rpc_unpacked->response->result );
    tt_want_str_op( rpc_message.response->resultmsg, ==,
                    rpc_unpacked->response->resultmsg );

    ct__rpc__free_unpacked( rpc_unpacked, &xi_utest_protobuf_engine_allocator );

    tt_want_int_op( xi_is_whole_memory_deallocated(), >, 0 );
} )
XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
