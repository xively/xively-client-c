#ifndef __XI_BSP_HTON_H__
#define __XI_BSP_HTON_H__

#include <sys/types.h>

uint16_t htons( uint16_t n )
{
    return ( ( n & 0xff ) << 8 ) | ( ( n & 0xff00 ) >> 8 );
}

uint16_t ntohs( uint16_t n )
{
    return htons( n );
}

uint32_t htonl( uint32_t n )
{
    return ( ( n & 0xff ) << 24 ) | ( ( n & 0xff00 ) << 8 ) |
           ( ( n & 0xff0000UL ) >> 8 ) | ( ( n & 0xff000000UL ) >> 24 );
}

uint32_t ntohl( uint32_t n )
{
    return htonl( n );
}

#endif /* __XI_BSP_HTON_H__ */
