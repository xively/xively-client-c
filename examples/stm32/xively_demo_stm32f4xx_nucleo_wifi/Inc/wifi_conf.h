/*

Please edit this file to configure the comm interface (UART/SPI) between Nucleo and X-Nucleo and also configure which module is used (SPWF01/04)
Each supported Nucleo platform will have different configurations of supported interfaces and modules.
Please see which Nucleo platform(Macro) you are using and configure accordingly.

You can also set debug print option from this file. (macro DEBUG_PRINT)

Meaning of the following macros:
DEBUG_PRINT:            Enable or disable debug prints on the serial console.
                        0 -> disable debug prints on serial console
                        1 -> enable debug prints on serial console
CONSOLE_UART_ENABLED:   Enable or disable the UART/SPI interface for either SPWF01 or SPWF04. 
                        Enabling this macro means the UART interface is used. 
                        Disabling will mean SPI will be used as interface.
SPWF01:                 SPWF01SA module is used as underlying expansion board (X-NUCLEO-IDW01M1). 
                        Enabling this macro will build for SPWF01SA. Must be disabled if SPWF04 is used.
SPWF04:                 SPWF04SA module is used as underlying expansion board (X-NUCLEO-IDW04A1).
                        Enabling this macro will build for SPWF04SA. Must be disabled if SPWF01 is used.
*/

#define DEBUG_PRINT 0

#ifdef USE_STM32F4XX_NUCLEO  
#define CONSOLE_UART_ENABLED
#define SPWF01
//#define SPWF04
#endif

#ifdef USE_STM32L4XX_NUCLEO
//#define CONSOLE_UART_ENABLED
//#define SPWF01
#define SPWF04
#endif

#ifdef USE_STM32L0XX_NUCLEO
/*SPWF04 is not supported on this board*/
#define CONSOLE_UART_ENABLED
#define SPWF01
#endif

#ifdef USE_STM32F1xx_NUCLEO
/*SPWF04 is not supported on this board*/
#define CONSOLE_UART_ENABLED
#define SPWF01
#endif
