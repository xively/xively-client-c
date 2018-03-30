  /**
  ******************************************************************************
  * @file    stm32_xx_hal_msp.c
  * @author  Central LAB
  * @version V2.1.0
  * @date    17-May-2016
  * @brief   HAL MSP module
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
#include "main.h"
#include "wifi_module.h"
#include "wifi_globals.h"
#include "stm32_spwf_wifi.h"

/** @addtogroup STM32_xx_HAL_MSP
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Prescaler declaration */

/* TIM handle declaration */

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

/**
 * @brief  This function is used for low level initialization of the SPI 
 *         communication with the WIFI Expansion Board.
 * @param  Pointer to the handle of the STM32Cube HAL SPI interface.
 * @retval None
 */
#if defined (USE_STM32F4XX_NUCLEO) || defined (USE_STM32L4XX_NUCLEO)
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
  uint32_t peripheral_address;
  
  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==WIFI_SPI_INSTANCE)
  {
    /* Enable peripherals clock */
    
    /* Enable GPIO Ports Clock */ 
    WIFI_SPI_RESET_CLK_ENABLE();
    WIFI_SPI_SCLK_CLK_ENABLE();
    WIFI_SPI_MISO_CLK_ENABLE();
    WIFI_SPI_MOSI_CLK_ENABLE();
    WIFI_SPI_CS_CLK_ENABLE();
    WIFI_SPI_IRQ_CLK_ENABLE();
    
    /* Enable SPI clock */
    WIFI_SPI_CLK_ENABLE();    
    
    /* RESET */
    GPIO_InitStruct.Pin = WIFI_SPI_RESET_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_RESET_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_RESET_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_RESET_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_RESET_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_SCLK_PORT, &GPIO_InitStruct); 
    
    /* SCLK */
    GPIO_InitStruct.Pin = WIFI_SPI_SCLK_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_SCLK_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_SCLK_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_SCLK_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_SCLK_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_SCLK_PORT, &GPIO_InitStruct); 
    
    /* MISO */
    GPIO_InitStruct.Pin = WIFI_SPI_MISO_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_MISO_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_MISO_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_MISO_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_MISO_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_MISO_PORT, &GPIO_InitStruct);
    
    /* MOSI */
    GPIO_InitStruct.Pin = WIFI_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_MOSI_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_MOSI_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_MOSI_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_MOSI_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_MOSI_PORT, &GPIO_InitStruct);
    
    /* NSS/CSN/CS */
    GPIO_InitStruct.Pin = WIFI_SPI_CS_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_CS_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_CS_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_CS_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_CS_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_CS_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(WIFI_SPI_CS_PORT, WIFI_SPI_CS_PIN, GPIO_PIN_SET);
    
    /* IRQ -- INPUT */
    GPIO_InitStruct.Pin = WIFI_SPI_IRQ_PIN;
    GPIO_InitStruct.Mode = WIFI_SPI_IRQ_MODE;
    GPIO_InitStruct.Pull = WIFI_SPI_IRQ_PULL;
    GPIO_InitStruct.Speed = WIFI_SPI_IRQ_SPEED;
    GPIO_InitStruct.Alternate = WIFI_SPI_IRQ_ALTERNATE;
    HAL_GPIO_Init(WIFI_SPI_IRQ_PORT, &GPIO_InitStruct);
    
    /*##-3- Configure the DMA channel ##########################################*/ 
    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;
    
    /* Enable DMA2 clock */
    WIFI_DMA_CLK_ENABLE();   

#if defined (USE_STM32L4XX_NUCLEO) 
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Init.Request             = WIFI_SPI_TX_DMA_REQUEST;
    
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_tx.Instance                 = WIFI_SPI_TX_DMA_CHANNEL;
#endif
#if defined (USE_STM32F4XX_NUCLEO) 
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Init.Channel             = WIFI_SPI_TX_DMA_CHANNEL;
    
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_tx.Instance                 = WIFI_SPI_TX_DMA_STREAM;
#endif    
    HAL_DMA_Init(&hdma_tx);   
    peripheral_address = __HAL_WIFI_SPI_GET_TX_DATA_REGISTER_ADDRESS(hspi);
    __HAL_WIFI_DMA_SET_PERIPHERAL_ADDRESS(&hdma_tx, peripheral_address);
    
    /* Associate the initialized DMA handle to the SPI handle */
    __HAL_LINKDMA(hspi, hdmatx, hdma_tx);
    
#if defined (USE_STM32L4XX_NUCLEO)   
    /* Configure the DMA handler for Transmission process */
    hdma_rx.Init.Request             = WIFI_SPI_RX_DMA_REQUEST;
    
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Instance                 = WIFI_SPI_RX_DMA_CHANNEL;
#endif
#if defined (USE_STM32F4XX_NUCLEO)
    /* Configure the DMA handler for Transmission process */
    hdma_rx.Init.Channel             = WIFI_SPI_RX_DMA_CHANNEL;
    
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Instance                 = WIFI_SPI_RX_DMA_STREAM;
#endif
    
    HAL_DMA_Init(&hdma_rx);
    peripheral_address = __HAL_WIFI_SPI_GET_RX_DATA_REGISTER_ADDRESS(hspi);
    __HAL_WIFI_DMA_SET_PERIPHERAL_ADDRESS(&hdma_rx, peripheral_address);
    
    /* Associate the initialized DMA handle to the SPI handle */
    __HAL_LINKDMA(hspi, hdmarx, hdma_rx); 
    
    /* Configure the NVIC for SPI */  
    HAL_NVIC_SetPriority(WIFI_SPI_DMA_TX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(WIFI_SPI_DMA_TX_IRQn);
    
    HAL_NVIC_SetPriority(WIFI_SPI_DMA_RX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(WIFI_SPI_DMA_RX_IRQn);
    
    HAL_NVIC_SetPriority(SPIx_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_IRQn);
      
    /* Configure the NVIC for EXTI */  
    HAL_NVIC_SetPriority(WIFI_SPI_EXTI_IRQn, 2, 0);    
    //HAL_NVIC_EnableIRQ(WIFI_SPI_EXTI_IRQn);
  }
}
#endif

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{ 
  
#if defined(USE_STM32F4XX_NUCLEO) || defined(USE_STM32L4XX_NUCLEO)
  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;
#endif

  GPIO_InitTypeDef  GPIO_InitStruct;

  if (huart==&UartWiFiHandle)
  {
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();

#if defined(SPWF04) && defined(CONSOLE_UART_ENABLED)
  USARTx_DMAx_CLK_ENABLE();
#endif
  
  /* Enable USARTx clock */
  USARTx_CLK_ENABLE(); 
  __SYSCFG_CLK_ENABLE();

#ifdef USE_STM32F1xx_NUCLEO      
   __HAL_AFIO_REMAP_USART3_PARTIAL();
#endif
   
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = WiFi_USART_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
#if defined (USE_STM32L0XX_NUCLEO) || defined (USE_STM32F4XX_NUCLEO) || defined (USE_STM32L4XX_NUCLEO)
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = WiFi_USARTx_TX_AF;
#endif  
#ifdef USE_STM32F1xx_NUCLEO      
   GPIO_InitStruct.Pull     = GPIO_PULLUP;
#endif  
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  
  HAL_GPIO_Init(WiFi_USART_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = WiFi_USART_RX_PIN;
#ifdef USE_STM32F1xx_NUCLEO  
  GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;
#endif  
#if defined (USE_STM32L0XX_NUCLEO) || defined (USE_STM32F4XX_NUCLEO) || defined (USE_STM32L4XX_NUCLEO)
  GPIO_InitStruct.Alternate = WiFi_USARTx_RX_AF;
#endif
  
  HAL_GPIO_Init(WiFi_USART_RX_GPIO_PORT, &GPIO_InitStruct);
  
  
  /* UART RTS GPIO pin configuration  */
  GPIO_InitStruct.Pin = WiFi_USART_RTS_PIN;
#ifdef USE_STM32F1xx_NUCLEO  
  GPIO_InitStruct.Pull     = GPIO_PULLDOWN;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;//GPIO_MODE_AF_PP;
#endif  
#if defined (USE_STM32L0XX_NUCLEO) || defined (USE_STM32F4XX_NUCLEO) || defined (USE_STM32L4XX_NUCLEO)
  GPIO_InitStruct.Pull     = GPIO_PULLUP;
  GPIO_InitStruct.Alternate = WiFi_USARTx_RX_AF;
#endif
  
  HAL_GPIO_Init(WiFi_USART_RTS_GPIO_PORT, &GPIO_InitStruct);
  
#if defined (USE_STM32F4XX_NUCLEO) 
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = WIFI_USART_TX_DMA_STREAM;
    hdma_tx.Init.Channel             = WIFI_USART_TX_DMA_CHANNEL;
    
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_tx);  
    
    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(huart, hdmatx, hdma_tx);  
    
    /* Configure the DMA handler for reception process */
    hdma_rx.Instance                 = WIFI_USART_RX_DMA_STREAM;
    hdma_rx.Init.Channel             = WIFI_USART_RX_DMA_CHANNEL;
    
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_CIRCULAR;//DMA_NORMAL;//
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

    HAL_DMA_Init(&hdma_rx);
  
    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(huart, hdmarx, hdma_rx);
#endif   

#if defined (USE_STM32L4XX_NUCLEO) 
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Init.Request             = WIFI_USART_TX_DMA_REQUEST;

    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_tx.Instance                 = WIFI_USART_TX_DMA_CHANNEL;
    
    HAL_DMA_Init(&hdma_tx);  
    
    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(huart, hdmatx, hdma_tx);  
    
    /* Configure the DMA handler for Reception process */
    hdma_rx.Init.Request             = WIFI_USART_RX_DMA_REQUEST;
    
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_CIRCULAR;//DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Instance                 = WIFI_USART_RX_DMA_CHANNEL;
    
    HAL_DMA_Init(&hdma_rx);
  
    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(huart, hdmarx, hdma_rx);
    
#endif
    
#if defined(SPWF04) && defined(CONSOLE_UART_ENABLED)
  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
  HAL_NVIC_SetPriority(WIFI_USART_DMA_TX_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(WIFI_USART_DMA_TX_IRQn);
    
  /* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
  HAL_NVIC_SetPriority(WIFI_USART_DMA_RX_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(WIFI_USART_DMA_RX_IRQn);
#endif
  
  /*##-3- Configure the NVIC for UART ########################################*/
  /* NVIC for USART */
	#if defined (USE_STM32L0XX_NUCLEO) || defined (USE_STM32F4XX_NUCLEO) || defined (USE_STM32L4XX_NUCLEO)
          #if defined (SPWF04) && defined (CONSOLE_UART_ENABLED)
            HAL_NVIC_SetPriority(USARTx_IRQn, 0, 0);
          #else
            HAL_NVIC_SetPriority(USARTx_IRQn, 3, 0);
          #endif  //SPWF04
	#else
          #if defined (SPWF04) && defined (CONSOLE_UART_ENABLED)
            HAL_NVIC_SetPriority(USARTx_IRQn, 0, 0);
          #else
            HAL_NVIC_SetPriority(USARTx_IRQn, 1, 0);
          #endif  // SPWF04
	#endif    // STM32L0XX_NUCLEO

  HAL_NVIC_EnableIRQ(USARTx_IRQn);
  
    /* Enable the UART Idle Interrupt */
    #if defined(SPWF04) && defined(CONSOLE_UART_ENABLED)
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
    #endif
    
  }
}


/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart==&UartWiFiHandle)
  {
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(WiFi_USART_TX_GPIO_PORT, WiFi_USART_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(WiFi_USART_RX_GPIO_PORT, WiFi_USART_RX_PIN);
  
  /*##-3- Disable the NVIC for UART ##########################################*/
  HAL_NVIC_DisableIRQ(USARTx_IRQn);
  }
}


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim==&PushTimHandle)
  {
    /* TIMx Peripheral clock enable */
    TIMp_CLK_ENABLE();
    
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIMp_IRQn, 3, 0);
    
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMp_IRQn);
  }
  else
  {    
    /* TIMx Peripheral clock enable */
    TIMx_CLK_ENABLE();
    
    /*##-2- Configure the NVIC for TIMx ########################################*/
    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIMx_IRQn, 3, 0);
    
    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIMx_IRQn);
  }
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
