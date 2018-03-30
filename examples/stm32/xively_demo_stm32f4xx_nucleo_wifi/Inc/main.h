  /**
  ******************************************************************************
  * @file    main.h
  * @author  Central LAB
  * @version V2.0.1
  * @date    17-May-2016
  * @brief   Header for main.c module
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#ifdef USE_STM32L0XX_NUCLEO
#include "stm32l0xx_hal.h"
#include "stm32l0xx_nucleo.h"
#endif
#ifdef USE_STM32F1xx_NUCLEO
#include "stm32f1xx_hal.h"
#include "stm32f1xx_nucleo.h"
#endif
#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"
#endif  
#ifdef USE_STM32L4XX_NUCLEO
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo.h"
#endif 
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

typedef enum {
  wifi_state_reset = 0,
  wifi_state_ready,
  wifi_state_idle,
  wifi_state_connected,
  wifi_state_connecting,
  wifi_state_socket,
  wifi_state_socket_write,
  wifi_state_disconnected,
  wifi_state_activity,
  wifi_state_inter,
  wifi_state_print_data,
  wifi_state_input_buffer,
  wifi_state_error,
  wifi_undefine_state       = 0xFF,
} wifi_state_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* Private macro ------------------------------------------------------------ */
#ifdef USART_PRINT_MSG

#ifdef USE_STM32L0XX_NUCLEO  

#define WIFI_UART_MSG                           USART2
#define USARTx_PRINT_CLK_ENABLE()              __USART2_CLK_ENABLE()
#define USARTx_PRINT_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_PRINT_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_PRINT_FORCE_RESET()             __USART2_FORCE_RESET()
#define USARTx_PRINT_RELEASE_RESET()           __USART2_RELEASE_RESET()

#define PRINTMSG_USARTx_TX_AF                       GPIO_AF4_USART2
#define PRINTMSG_USARTx_RX_AF                       GPIO_AF4_USART2

#endif //USE_STM32L0XX_NUCLEO

#if defined(USE_STM32F1xx_NUCLEO) || defined(USE_STM32F4XX_NUCLEO) || defined(USE_STM32L4XX_NUCLEO)

#define WIFI_UART_MSG                           USART2
#define USARTx_PRINT_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE()
#define USARTx_PRINT_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USARTx_PRINT_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USARTx_PRINT_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_PRINT_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()

#define PRINTMSG_USARTx_TX_AF                       GPIO_AF7_USART2
#define PRINTMSG_USARTx_RX_AF                       GPIO_AF7_USART2

#endif //(USE_STM32F1xx_NUCLEO) || (USE_STM32F4XX_NUCLEO)

#define WiFi_USART_PRINT_TX_PIN                    GPIO_PIN_2
#define WiFi_USART_PRINT_TX_GPIO_PORT              GPIOA
#define WiFi_USART_PRINT_RX_PIN                    GPIO_PIN_3
#define WiFi_USART_PRINT_RX_GPIO_PORT              GPIOA


/* Definition for USARTx's NVIC */
#define USARTx_PRINT_IRQn                      USART2_IRQn
#define USARTx_PRINT_IRQHandler                USART2_IRQHandler

#endif //USART_PRINT_MSG


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
