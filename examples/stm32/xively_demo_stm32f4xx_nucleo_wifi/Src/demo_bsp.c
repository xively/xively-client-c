#include "demo_bsp.h"
#include "stm32_spwf_wifi.h"
#include "wifi_globals.h"

/******************************************************************************
*                 UART Initialization implementation for                      *
*  STM32F1xx_NUCLEO, STM32F4XX_NUCLEO, STM32L0XX_NUCLEO, STM32L4XX_NUCLEO     *
******************************************************************************/

#ifdef USART_PRINT_MSG
UART_HandleTypeDef UART_MsgHandle;

void UART_Msg_Gpio_Init()
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    USARTx_PRINT_TX_GPIO_CLK_ENABLE();
    USARTx_PRINT_RX_GPIO_CLK_ENABLE();

    /* Enable USARTx clock */
    USARTx_PRINT_CLK_ENABLE();
    __SYSCFG_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin   = WiFi_USART_PRINT_TX_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
#if defined( USE_STM32L0XX_NUCLEO ) || defined( USE_STM32F4XX_NUCLEO ) || \
    defined( USE_STM32L4XX_NUCLEO )
    GPIO_InitStruct.Alternate = PRINTMSG_USARTx_TX_AF;
#endif /* defined(...)||defined(...)||defined(...) */
    HAL_GPIO_Init( WiFi_USART_PRINT_TX_GPIO_PORT, &GPIO_InitStruct );

    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin  = WiFi_USART_PRINT_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#if defined( USE_STM32L0XX_NUCLEO ) || defined( USE_STM32F4XX_NUCLEO ) || \
    defined( USE_STM32L4XX_NUCLEO )
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = PRINTMSG_USARTx_RX_AF;
#endif /* defined(...)||defined(...)||defined(...) */

    HAL_GPIO_Init( WiFi_USART_PRINT_RX_GPIO_PORT, &GPIO_InitStruct );

#ifdef WIFI_USE_VCOM
    /*##-3- Configure the NVIC for UART ########################################*/
    /* NVIC for USART */
    HAL_NVIC_SetPriority( USARTx_PRINT_IRQn, 0, 1 );
    HAL_NVIC_EnableIRQ( USARTx_PRINT_IRQn );
#endif /* WIFI_USE_VCOM */
}

int8_t USART_PRINT_MSG_Configuration( uint32_t baud_rate )
{
    UART_MsgHandle.Instance        = WIFI_UART_MSG;
    UART_MsgHandle.Init.BaudRate   = baud_rate;
    UART_MsgHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UART_MsgHandle.Init.StopBits   = UART_STOPBITS_1;
    UART_MsgHandle.Init.Parity     = UART_PARITY_NONE;
    UART_MsgHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UART_MsgHandle.Init.Mode       = UART_MODE_TX_RX;

    if ( HAL_UART_DeInit( &UART_MsgHandle ) != HAL_OK )
    {
        return -1;
    }
    if ( HAL_UART_Init( &UART_MsgHandle ) != HAL_OK )
    {
        return -1;
    }

    Set_UartMsgHandle( &UART_MsgHandle );
    return 0;
}

#endif /* USART_PRINT_MSG */

/******************************************************************************
*                 SystemClock_Config() implementations for                    *
*  STM32F1xx_NUCLEO, STM32F4XX_NUCLEO, STM32L0XX_NUCLEO, STM32L4XX_NUCLEO     *
******************************************************************************/

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSI)
 *            SYSCLK(Hz)                     = 64000000
 *            HCLK(Hz)                       = 64000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 2
 *            APB2 Prescaler                 = 1
 *            PLLMUL                         = 16
 *            Flash Latency(WS)              = 2
 * @param  None
 * @retval None
 */

#ifdef USE_STM32F1xx_NUCLEO

void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef clkinitstruct = {0};
    RCC_OscInitTypeDef oscinitstruct = {0};

    /* Configure PLL ------------------------------------------------------*/
    /* PLL configuration: PLLCLK = (HSI / 2) * PLLMUL = (8 / 2) * 16 = 64 MHz */
    /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 64 / 1 = 64 MHz */
    /* Enable HSI and activate PLL with HSi_DIV2 as source */
    oscinitstruct.OscillatorType      = RCC_OSCILLATORTYPE_HSE;
    oscinitstruct.HSEState            = RCC_HSE_ON;
    oscinitstruct.LSEState            = RCC_LSE_OFF;
    oscinitstruct.HSIState            = RCC_HSI_OFF;
    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    oscinitstruct.HSEPredivValue      = RCC_HSE_PREDIV_DIV1;
    oscinitstruct.PLL.PLLState        = RCC_PLL_ON;
    oscinitstruct.PLL.PLLSource       = RCC_PLLSOURCE_HSE;
    oscinitstruct.PLL.PLLMUL          = RCC_PLL_MUL9;
    if ( HAL_RCC_OscConfig( &oscinitstruct ) != HAL_OK )
    {
        /* Initialization Error */
        while ( 1 )
            ;
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    clkinitstruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    clkinitstruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clkinitstruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
    clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
    if ( HAL_RCC_ClockConfig( &clkinitstruct, FLASH_LATENCY_2 ) != HAL_OK )
    {
        /* Initialization Error */
        while ( 1 )
            ;
    }
}

#elif defined(USE_STM32F4XX_NUCLEO)

void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE2 );

    /* Enable HSI Oscillator and activate PLL with HSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 16;
    RCC_OscInitStruct.PLL.PLLN            = 336;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = 7;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );
}

#elif defined(USE_STM32L0XX_NUCLEO)

/**
 * @brief  System Clock Configuration
 * @param  None
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 0x10;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_2;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    RCC_ClkInitStruct.ClockType =
        ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
          RCC_CLOCKTYPE_PCLK2 );                             // RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI; // RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_1 );

    __SYSCFG_CLK_ENABLE();
}

#elif defined(USE_STM32L4XX_NUCLEO)

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (MSI)
 *            SYSCLK(Hz)                     = 80000000
 *            HCLK(Hz)                       = 80000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            APB2 Prescaler                 = 1
 *            MSI Frequency(Hz)              = 4000000
 *            PLL_M                          = 1
 *            PLL_N                          = 40
 *            PLL_R                          = 2
 *            PLL_P                          = 7
 *            PLL_Q                          = 4
 *            Flash Latency(WS)              = 4
 * @param  None
 * @retval None
 */
void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState            = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange       = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM            = 1;
    RCC_OscInitStruct.PLL.PLLN            = 40;
    RCC_OscInitStruct.PLL.PLLR            = 2;
    RCC_OscInitStruct.PLL.PLLP            = 7;
    RCC_OscInitStruct.PLL.PLLQ            = 4;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_4 );
}
#endif


