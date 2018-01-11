/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tinytest.h"
#include "tinytest_macros.h"
#include "xi_tt_testcase_management.h"

/* PROTOBUF C FILE OUTPUT -- BEGIN */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "xi_utest_protobuf_endianess.pb-c.h"

#ifndef UTEST_PROTOBUF_ENDIANESS_FUNC
#define UTEST_PROTOBUF_ENDIANESS_FUNC

void xi__utest__endianess__init( XIUTESTENDIANESS* message )
{
    static XIUTESTENDIANESS init_value = XI__UTEST__ENDIANESS__INIT;
    *message                           = init_value;
}
size_t xi__utest__endianess__get_packed_size( const XIUTESTENDIANESS* message )
{
    assert( message->base.descriptor == &xi__utest__endianess__descriptor );
    return protobuf_c_message_get_packed_size( ( const ProtobufCMessage* )( message ) );
}
size_t xi__utest__endianess__pack( const XIUTESTENDIANESS* message, uint8_t* out )
{
    assert( message->base.descriptor == &xi__utest__endianess__descriptor );
    return protobuf_c_message_pack( ( const ProtobufCMessage* )message, out );
}
size_t xi__utest__endianess__pack_to_buffer( const XIUTESTENDIANESS* message,
                                             ProtobufCBuffer* buffer )
{
    assert( message->base.descriptor == &xi__utest__endianess__descriptor );
    return protobuf_c_message_pack_to_buffer( ( const ProtobufCMessage* )message,
                                              buffer );
}
XIUTESTENDIANESS* xi__utest__endianess__unpack( ProtobufCAllocator* allocator,
                                                size_t len,
                                                const uint8_t* data )
{
    return ( XIUTESTENDIANESS* )protobuf_c_message_unpack(
        &xi__utest__endianess__descriptor, allocator, len, data );
}
void xi__utest__endianess__free_unpacked( XIUTESTENDIANESS* message,
                                          ProtobufCAllocator* allocator )
{
    assert( message->base.descriptor == &xi__utest__endianess__descriptor );
    protobuf_c_message_free_unpacked( ( ProtobufCMessage* )message, allocator );
}
static const ProtobufCFieldDescriptor xi__utest__endianess__field_descriptors[1] = {
    {
        "test_value", 1, PROTOBUF_C_LABEL_REQUIRED, PROTOBUF_C_TYPE_UINT32,
        0,                                                       /* quantifier_offset */
        offsetof( XIUTESTENDIANESS, test_value ), NULL, NULL, 0, /* flags */
        0, NULL, NULL /* reserved1,reserved2, etc */
    },
};
static const unsigned xi__utest__endianess__field_indices_by_name[] = {
    0, /* field[0] = test_value */
};
static const ProtobufCIntRange xi__utest__endianess__number_ranges[1 + 1] = {{1, 0},
                                                                             {0, 1}};
const ProtobufCMessageDescriptor xi__utest__endianess__descriptor = {
    PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
    "XI_UTEST_ENDIANESS",
    "XIUTESTENDIANESS",
    "XIUTESTENDIANESS",
    "",
    sizeof( XIUTESTENDIANESS ),
    1,
    xi__utest__endianess__field_descriptors,
    xi__utest__endianess__field_indices_by_name,
    1,
    xi__utest__endianess__number_ranges,
    ( ProtobufCMessageInit )xi__utest__endianess__init,
    NULL,
    NULL,
    NULL /* reserved[123] */
};

/* PROTOBUF C FILE OUTPUT -- end */

/* This is the length of the final encoded data that we're going to test serialization
 * against. */
const size_t xi_utest_protobuf_endianess_data_length = 6;

/* This is the final encoded data that we're going to test serialization against, and
 * attempt to deserialization against.
 */
const uint8_t xi_utest_protobuf_packed_data[] = {0x08, 0xCD, 0xA5, 0xAC, 0x85, 0x09};

/* This is the value that we're going to serialize */
const uint32_t xi_utest_protobuf_uint32_value = 0x90AB12CD;

#endif /* UTEST_PROTOBUF_ENDIANESS_FUNC */


XI_TT_TESTGROUP_BEGIN( utest_protobuf_endianess )

/*******************************************************************************************
 * invalid input tests
 **********************************************************************
 *******************************************************************************************/
XI_TT_TESTCASE( utest__protobuf_endianess___pack, {
    XIUTESTENDIANESS msg = XI__UTEST__ENDIANESS__INIT;

    uint8_t* packed_msg               = NULL;
    ProtobufCAllocator* nullAllocator = NULL;
    size_t predicted_packed_length;
    size_t packed_length;

    /* set the funciton type */
    msg.test_value = 0x90AB12CD;

    /* pack the data */
    predicted_packed_length = xi__utest__endianess__get_packed_size( &msg );
    tt_want_int_op( predicted_packed_length, ==,
                    xi_utest_protobuf_endianess_data_length );

    packed_msg = malloc( sizeof( uint8_t ) * predicted_packed_length );
    tt_want_ptr_op( nullAllocator, !=, packed_msg );

    packed_length = xi__utest__endianess__pack( &msg, packed_msg );
    tt_want_int_op( packed_length, ==, predicted_packed_length );

    size_t i = 0;
    for ( ; i < packed_length; ++i )
    {
        tt_want_uint_op( xi_utest_protobuf_packed_data[i], ==, packed_msg[i] );
    }

    free( packed_msg );
} )

XI_TT_TESTCASE( utest__protobuf_endianess___unpack, {
    XIUTESTENDIANESS* unpacked_msg    = NULL;
    ProtobufCAllocator* nullAllocator = NULL;

    /* unpack the data */
    unpacked_msg = xi__utest__endianess__unpack( nullAllocator,
                                                 xi_utest_protobuf_endianess_data_length,
                                                 xi_utest_protobuf_packed_data );

    tt_want_ptr_op( NULL, !=, unpacked_msg );

    tt_want_uint_op( unpacked_msg->test_value, ==, xi_utest_protobuf_uint32_value );

    xi__utest__endianess__free_unpacked( unpacked_msg, nullAllocator );

} )


XI_TT_TESTGROUP_END

#ifndef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#define XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#include __FILE__
#undef XI_TT_TESTCASE_ENUMERATION__SECONDPREPROCESSORRUN
#endif
