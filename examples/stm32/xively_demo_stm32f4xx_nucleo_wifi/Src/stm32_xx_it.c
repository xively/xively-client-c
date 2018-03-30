/**
******************************************************************************
* @file    stm32_xx_it.c
* @author  Central LAB
* @version V2.1.0
* @date    17-May-2016
* @brief   Main Interrupt Service Routines.
*          This file provides template for all exceptions handler and
*          peripherals interrupt service routine.
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32_xx_it.h"
#include "wifi_module.h"
#include "stm32_spwf_wifi.h"
#include "wifi_globals.h"
#include "demo_io.h"

/** @defgroup STM32xx_IT_Private_Variables
 * @{
 */

/** @addtogroup STM32xx_HAL_Examples
 * @{
 */

/** @addtogroup Templates
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void USARTx_IRQHandler( void );
void USARTx_PRINT_IRQHandler( void );
// void USARTx_EXTI_IRQHandler( void );
void TIMx_IRQHandler( void );
void TIMp_IRQHandler( void );

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*                Demo GPIO Interrupt implementation                         */
/******************************************************************************/
/**
 * @brief  This function handles External lines from 15 to 10 interrupt request
 * @param  None
 * @retval None
 */
void EXTI15_10_IRQHandler( void )
{
    HAL_GPIO_EXTI_IRQHandler( KEY_BUTTON_PIN );
}

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler( void )
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler( void )
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while ( 1 )
    {
        BSP_LED_On( LED2 );
    }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler( void )
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while ( 1 )
    {
        BSP_LED_On( LED2 );
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler( void )
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while ( 1 )
    {
        BSP_LED_On( LED2 );
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler( void )
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while ( 1 )
    {
        BSP_LED_On( LED2 );
    }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler( void )
{
    BSP_LED_On( LED2 );
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler( void )
{
    BSP_LED_On( LED2 );
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler( void )
{
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f1xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles TIM interrupt request.
 * @param  None
 * @retval None
 */
void TIMx_IRQHandler( void )
{
    HAL_TIM_IRQHandler( &TimHandle );
}

/**
 * @brief  This function handles TIM interrupt request.
 * @param  None
 * @retval None
 */
void TIMp_IRQHandler( void )
{
    HAL_TIM_IRQHandler( &PushTimHandle );
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler( void )
{
    HAL_IncTick();

#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    WiFi_AT_CMD_Timeout();
#endif

#if defined( SPWF01 ) && defined( CONSOLE_UART_ENABLED )
    Wifi_SysTick_Isr();
#endif
}

#if defined( SPWF01 ) && defined( CONSOLE_UART_ENABLED )
/**
 * @brief  HAL_UART_RxCpltCallback
 *         Rx Transfer completed callback
 * @param  UsartHandle: UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback( UART_HandleTypeDef* UartHandleArg )
{
    WiFi_HAL_UART_RxCpltCallback( UartHandleArg );
}
#endif

/**
 * @brief  Period elapsed callback in non blocking mode
 *         This timer is used for calling back User registered functions with information
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback( TIM_HandleTypeDef* htim )
{
    Wifi_TIM_Handler( htim );
}

/**
 * @brief  HAL_UART_TxCpltCallback
 *         Tx Transfer completed callback
 * @param  UsartHandle: UART handle
 * @retval None
 */
void HAL_UART_TxCpltCallback( UART_HandleTypeDef* UartHandleArg )
{
    WiFi_HAL_UART_TxCpltCallback( UartHandleArg );
}

/**
 * @brief  UART error callbacks
 * @param  UsartHandle: UART handle
 * @note   This example shows a simple way to report transfer error, and you can
 *         add your own implementation.
 * @retval None
 */
void HAL_UART_ErrorCallback( UART_HandleTypeDef* UartHandle )
{
    WiFi_HAL_UART_ErrorCallback( UartHandle );
}

#if defined( SPWF04 ) && defined( CONSOLE_UART_ENABLED )
/*Uart Idle Callback*/
void HAL_UART_IdleCallback( UART_HandleTypeDef* UartHandle )
{
    WiFi_HAL_UART_IdleCallback( UartHandle );
}
#endif

/******************************************************************************/
/*                 STM32 Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32xxx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles USARTx Handler.
 * @param  None
 * @retval None
 */
void USARTx_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UartWiFiHandle );
}

/**
 * @brief  This function handles USARTx vcom Handler.
 * @param  None
 * @retval None
 */
#ifdef USART_PRINT_MSG
void USARTx_PRINT_IRQHandler( void )
{
    HAL_UART_IRQHandler( UartMsgHandle );
}
#endif

#if defined( SPWF04 )
/**
 * @brief  EXTI line detection callback.
 * @param  uint16_t GPIO_Pin Specifies the pins connected EXTI line
 * @retval None
 */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    if ( GPIO_Pin == WIFI_SPI_EXTI_PIN )
    {
        WIFI_SPI_IRQ_Callback();
    }
    else
    {
        // nothing to do
    }
#endif
}

/**
 * @brief  WIFI_SPI_EXTI_IRQHandler This function handles External line
 *         interrupt request for BlueNRG.
 * @param  None
 * @retval None
 */
void WIFI_SPI_EXTI_IRQHandler( void )
{
    HAL_GPIO_EXTI_IRQHandler( WIFI_SPI_EXTI_PIN );
}


#define CS_PULSE_700NS_NBR_CYCLES_REQ2 152
#define CS_PULSE_LENGTH2 ( CS_PULSE_700NS_NBR_CYCLES_REQ2 / 4 )
#define DUMMY_RAM_ADDRESS_TO_READ2 ( 0x20000000 )

#ifdef STM32L476xx

/*SPI DMA Rx Callback*/
void DMA1_Channel2_IRQHandler( void )
{
#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    if ( __HAL_DMA_GET_IT_SOURCE( SpiHandle.hdmarx, DMA_IT_TC ) &&
         __HAL_DMA_GET_FLAG( SpiHandle.hdmarx, DMA_FLAG_TC2 ) )
    {
        WiFi_DMA_RxCallback();
    }
#endif
}

/*SPI DMA Tx Callback*/
void DMA1_Channel3_IRQHandler( void )
{
#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    if ( __HAL_DMA_GET_IT_SOURCE( SpiHandle.hdmatx, DMA_IT_TC ) &&
         __HAL_DMA_GET_FLAG( SpiHandle.hdmatx, DMA_FLAG_TC3 ) )
    {
        WiFi_DMA_TxCallback();
    }
#endif
}

#if defined( SPWF04 ) && defined( CONSOLE_UART_ENABLED )

/*UART DMA Tx Callback*/
void DMA1_Channel4_IRQHandler( void )
{
    HAL_DMA_IRQHandler( UartWiFiHandle.hdmatx );
}

/*UART DMA Rx Callback*/
void DMA1_Channel5_IRQHandler( void )
{
    // if(__HAL_DMA_GET_IT_SOURCE(UartWiFiHandle.hdmarx, DMA_IT_TC)  &&
    // __HAL_DMA_GET_FLAG(UartWiFiHandle.hdmarx, DMA_FLAG_TC5))
    {
        HAL_DMA_IRQHandler( UartWiFiHandle.hdmarx );
    }
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef* UartHandle )
{
    /* Disable the Idle Interrupt */
    __HAL_UART_DISABLE_IT( UartHandle, UART_IT_IDLE );

    Read_DMA_Buffer();

    /* Enable the Idle Interrupt */
    __HAL_UART_ENABLE_IT( UartHandle, UART_IT_IDLE );
}


void HAL_UART_RxHalfCpltCallback( UART_HandleTypeDef* UartHandle )
{
    /* Disable the Idle Interrupt */
    __HAL_UART_DISABLE_IT( UartHandle, UART_IT_IDLE );

    Read_DMA_Buffer();

    /* Enable the Idle Interrupt */
    __HAL_UART_ENABLE_IT( UartHandle, UART_IT_IDLE );
}

#endif

#endif /* STM32L476xx */

#ifdef STM32F401xE
void DMA2_Stream0_IRQHandler( void )
{
#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    if ( __HAL_DMA_GET_IT_SOURCE( SpiHandle.hdmarx, DMA_IT_TC ) &&
         __HAL_DMA_GET_FLAG( SpiHandle.hdmarx, DMA_FLAG_TCIF0_4 ) )
    {
        WiFi_DMA_RxCallback();
    }
#endif
}

void DMA2_Stream3_IRQHandler( void )
{
#if defined( SPWF04 ) && !defined( CONSOLE_UART_ENABLED )
    if ( __HAL_DMA_GET_IT_SOURCE( SpiHandle.hdmatx, DMA_IT_TC ) &&
         __HAL_DMA_GET_FLAG( SpiHandle.hdmatx, DMA_FLAG_TCIF3_7 ) )
    {
        WiFi_DMA_TxCallback();
    }
#endif
}

#if defined( SPWF04 ) && defined( CONSOLE_UART_ENABLED )
void DMA2_Stream7_IRQHandler( void )
{ // Tx USART1 DMA
    HAL_DMA_IRQHandler( UartWiFiHandle.hdmatx );
}

void DMA2_Stream2_IRQHandler( void )
{ // Rx USART1 DMA
    // printf("\r\ndma rx\r\n");
    HAL_DMA_IRQHandler( UartWiFiHandle.hdmarx );
}

// void USARTx_DMA_RX_IRQHandler(void)
//{
//    printf("\r\ndma rx\r\n");
//    HAL_DMA_IRQHandler(UartWiFiHandle.hdmarx);
//}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef* UartHandle )
{
    /* Disable the Idle Interrupt */
    __HAL_UART_DISABLE_IT( UartHandle, UART_IT_IDLE );

    Read_DMA_Buffer();

    /* Enable the Idle Interrupt */
    __HAL_UART_ENABLE_IT( UartHandle, UART_IT_IDLE );
}


void HAL_UART_RxHalfCpltCallback( UART_HandleTypeDef* UartHandle )
{
    /* Disable the Idle Interrupt */
    __HAL_UART_DISABLE_IT( UartHandle, UART_IT_IDLE );

    Read_DMA_Buffer();

    /* Enable the Idle Interrupt */
    __HAL_UART_ENABLE_IT( UartHandle, UART_IT_IDLE );
}

#endif

#endif /* STM32F401xE */
#endif /* SPWF04 */

       /**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE*****/
                                                                                      
