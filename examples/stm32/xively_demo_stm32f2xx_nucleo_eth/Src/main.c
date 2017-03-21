/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/main.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    01-July-2016
  * @brief   This sample code implements a http server application based on
  *          Netconn API of LwIP stack and FreeRTOS. This application uses
  *          STM32F2xx the ETH HAL API to transmit and receive data.
  *          The communication is done with a web browser of a remote PC.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright � 2016 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "cmsis_os.h"
#include "ethernetif.h"
#include "app_ethernet.h"
#include "httpserver-netconn.h"
#include <xively_client.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif gnetif; /* network interface structure */
/* UART handler declaration */
UART_HandleTypeDef UartHandle;

/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar( int ch )
#else
#define PUTCHAR_PROTOTYPE int fputc( int ch, FILE* f )
#endif /* __GNUC__ */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config( void );
static void StartThread( void const* argument );
static void Netif_Config( void );
static void BSP_Config( void );

/* Private functions ---------------------------------------------------------*/

static void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while (1)
  {
  }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main( void )
{
    /* STM32F2xx HAL library initialization:
         - Configure the Flash ART accelerator on ITCM interface
         - Configure the Systick to generate an interrupt each 1 msec
         - Set NVIC Group Priority to 4
         - Global MSP (MCU Support Package) initialization
       */
    HAL_Init();

    /* Configure the system clock to 120 MHz */
    SystemClock_Config();

    BSP_LED_Init(LED3);

    /*##-1- Configure the UART peripheral ######################################*/
    /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
    /* UART configured as follows:
        - Word Length = 9 Bits (8 data bit + 1 parity bit) :
        - Stop Bit    = One Stop bit
        - Parity      = ODD parity
        - BaudRate    = 115200 baud
        - Hardware flow control disabled (RTS and CTS signals) */
    UartHandle.Instance        = USARTx;
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_ODD;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX_RX;
    if ( HAL_UART_Init( &UartHandle ) != HAL_OK )
    {
        /* Initialization Error */
        Error_Handler();
    }

/* Init thread */
#if defined( __GNUC__ )
    osThreadDef( Start, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 5 );
#else
    osThreadDef( Start, StartThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE * 2 );
#endif

    osThreadCreate( osThread( Start ), NULL );

    /* Start scheduler */
    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for ( ;; )
        ;
}

PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

/**
  * @brief  Start Thread
  * @param  argument not used
  * @retval None
  */
static void StartThread( void const* argument )
{
    /* Initialize LEDs */
    BSP_Config();

    /* Create tcp_ip stack thread */
    tcpip_init( NULL, NULL );

    /* Initialize the LwIP stack */
    Netif_Config();

    /* Initialize webserver demo */
    // http_server_netconn_init();

    /* Notify user about the network interface config */
    User_notification( &gnetif );

#ifdef USE_DHCP
/* Start DHCPClient */
#if defined( __GNUC__ )
    osThreadDef( DHCP, DHCP_thread, osPriorityBelowNormal, 0,
                 configMINIMAL_STACK_SIZE * 5 );
#else
    osThreadDef( DHCP, DHCP_thread, osPriorityBelowNormal, 0,
                 configMINIMAL_STACK_SIZE * 2 );
#endif

    osThreadCreate( osThread( DHCP ), &gnetif );
#endif

    xively_client_start();

    for ( ;; )
    {
        /* Delete the Init Thread */
        osThreadTerminate( NULL );
    }
}

/**
  * @brief  Initializes the lwIP stack
  * @param  None
  * @retval None
  */
static void Netif_Config( void )
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

#ifdef USE_DHCP
    ipaddr.addr  = 0;
    netmask.addr = 0;
    gw.addr      = 0;
#else
    /* IP address default setting */
    IP4_ADDR( &ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3 );
    IP4_ADDR( &netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3 );
    IP4_ADDR( &gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3 );
#endif

    /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
    struct ip_addr *netmask, struct ip_addr *gw,
    void *state, err_t (* init)(struct netif *netif),
    err_t (* input)(struct pbuf *p, struct netif *netif))

    Adds your network interface to the netif_list. Allocate a struct
    netif and pass a pointer to this structure as the first argument.
    Give pointers to cleared ip_addr structures when using DHCP,
    or fill them with sane numbers otherwise. The state pointer may be NULL.

    The init function pointer must point to a initialization function for
    your ethernet netif interface. The following code illustrates it's use.*/

    netif_add( &gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input );

    /*  Registers the default network interface. */
    netif_set_default( &gnetif );

    if ( netif_is_link_up( &gnetif ) )
    {
        /* When the netif is fully configured this function must be called.*/
        netif_set_up( &gnetif );
    }
    else
    {
        /* When the netif link is down this function must be called */
        netif_set_down( &gnetif );
    }
}

/**
  * @brief  Initializes the LEDs resources.
  * @param  None
  * @retval None
  */
static void BSP_Config( void )
{
    /* Configure LED1, LED2, LED3 */
    BSP_LED_Init( LED1 );
    BSP_LED_Init( LED2 );
    BSP_LED_Init( LED3 );
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 120000000
  *            HCLK(Hz)                       = 120000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 240
  *            PLL_P                          = 2
  *            PLL_Q                          = 5
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 3
  * @param  None
  * @retval None
  */
void SystemClock_Config( void )
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 8;
    RCC_OscInitStruct.PLL.PLLN       = 240;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 5;
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 );
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_3 );
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while ( 1 )
    {
    }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
