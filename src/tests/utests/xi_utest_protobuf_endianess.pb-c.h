/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: xi_utest_protobuf_endianess.proto */

#ifndef PROTOBUF_C_xi_5futest_5fprotobuf_5fendianess_2eproto__INCLUDED
#define PROTOBUF_C_xi_5futest_5fprotobuf_5fendianess_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1001001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _XIUTESTENDIANESS XIUTESTENDIANESS;


/* --- enums --- */


/* --- messages --- */

struct  _XIUTESTENDIANESS
{
  ProtobufCMessage base;
  uint32_t test_value;
};
#define XI__UTEST__ENDIANESS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&xi__utest__endianess__descriptor) \
    , 0 }


/* XIUTESTENDIANESS methods */
void   xi__utest__endianess__init
                     (XIUTESTENDIANESS         *message);
size_t xi__utest__endianess__get_packed_size
                     (const XIUTESTENDIANESS   *message);
size_t xi__utest__endianess__pack
                     (const XIUTESTENDIANESS   *message,
                      uint8_t             *out);
size_t xi__utest__endianess__pack_to_buffer
                     (const XIUTESTENDIANESS   *message,
                      ProtobufCBuffer     *buffer);
XIUTESTENDIANESS *
       xi__utest__endianess__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   xi__utest__endianess__free_unpacked
                     (XIUTESTENDIANESS *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*XIUTESTENDIANESS_Closure)
                 (const XIUTESTENDIANESS *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor xi__utest__endianess__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_xi_5futest_5fprotobuf_5fendianess_2eproto__INCLUDED */
