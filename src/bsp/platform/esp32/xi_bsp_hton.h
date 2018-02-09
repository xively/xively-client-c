#include <sys/types.h>
#include <stdint.h>

//#include <lwip/def.h>
/* This is copied from lwip/def.h so we don't need to include it, removing
 * libxively.a's dependency on sdkconfig.h, and making it app-agnostic.
 * Other BSPs for lwip-based SDKs can most likely just include lwip/def.h and remove
 * these lines: */
#define htons(x) lwip_htons(x)
#define ntohs(x) lwip_ntohs(x)
#define htonl(x) lwip_htonl(x)
#define ntohl(x) lwip_ntohl(x)
uint16_t lwip_htons(uint16_t x);
uint16_t lwip_ntohs(uint16_t x);
uint32_t lwip_htonl(uint32_t x);
uint32_t lwip_ntohl(uint32_t x);
