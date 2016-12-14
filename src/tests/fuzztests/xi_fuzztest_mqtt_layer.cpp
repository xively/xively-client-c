#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* This is the fuzzer signature, and we cannot change it */
extern "C" int LLVMFuzzerTestOneInput( const uint8_t* data, size_t size )
{
    /* not implemented */
    return 0;
}
