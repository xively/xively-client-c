#include "include/wifi_conf.h"

/**
 * WHAT IS THIS FILE?
 * ------------------
 * Our BSP needs wifi_interface.h from the SDK.
 * wifi_interface.h includes stm32_spwf_wifi.h, which includes wifi_conf.h and
 * stm32f4xx_hal_conf.h.
 * We've found it only needs wifi_conf.h, and compiles just fine without the rest
 * of stm32_spwf_wifi.h.
 * By providing this file with the same name, and making sure that the makefile
 * includes our path before the SDK's, we can provide the SDK with just the data
 * it needs, while avoiding the inclusion of stm32f4xx_hal_conf.h.
 * By not having to provide the HAL configuration file when compiling libxively,
 * we can keep it agnostic from project-specific configurations.
 *
 * Important considerations:
 * 1. This header file must not be moved into the include/ directory. Otherwise it
 *    would be picked up by Eclipse when compiling the application code, and could
 *    cause problems.
 * 2. It's important the makefile (mt-stm32fx_nucleo_wifi.mk) includes the path to our BSP directory before
 *    including the path to the SDK's
 *
 * We don't like this solution, but it's better than the alternative. We've
 * reported it to ST so it can be fixed in the SDK and this hack can be removed.
 */
