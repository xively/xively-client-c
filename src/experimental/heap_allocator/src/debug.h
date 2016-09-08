#include <stdio.h>

#define debug_log( msg, ... )                                                            \
    printf( msg "\r\n", __VA_ARGS__ );                                                   \
    fflush( stdout )
