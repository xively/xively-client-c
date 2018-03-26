/*

Please edit this file to configure the comm interface (UART/SPI) between Nucleo and X-Nucleo and also configure which module is used (SPWF01/04)
Each supported Nucleo platform will have different configurations of supported interfaces and modules.
Please see which Nucleo platform(Macro) you are using and configure accordingly.

You can also set debug print option from this file. (macro DEBUG_PRINT)

Meaning of the following macros:

CONSOLE_UART_ENABLED:   Enable or disable the UART/SPI interface for either SPWF01 or SPWF04. 
                        Enabling this macro means the UART interface is used. 
                        Disabling will mean SPI will be used as interface.
SPWF01:                 SPWF01SA module is used as underlying expansion board (X-NUCLEO-IDW01M1). 
                        Enabling this macro will build for SPWF01SA.
SPWF04:                 SPWF04SA module is used as underlying expansion board (X-NUCLEO-IDW04A1).
                        Enabling this macro will build for SPWF04SA.
*/

/*Set the debug print option
0 -> disable debug prints on console
1 -> enable debug prints on console
*/
#define DEBUG_PRINT 0
