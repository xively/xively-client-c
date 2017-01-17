  /**
  ******************************************************************************
  * @file    main.c
  * @author  Central LAB
  * @version V2.1.0
  * @date    17-May-2016
  * @brief   Main program body
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
#include "wifi_interface.h"
#include "stdio.h"
#include "string.h"
#include "xively.h"

/**
   * @mainpage Documentation for X-CUBE-WIFI Software for STM32, Expansion for STM32Cube 
   * <b>Introduction</b> <br>
   * X-CUBE-WIFI1 is an expansion software package for STM32Cube. 
   * The software runs on STM32 and it can be used for building Wi-Fi applications using the SPWF01Sx device. 
   * It is built on top of STM32Cube software technology that eases portability across different STM32 microcontrollers.
   *
   *
   * \htmlinclude extra.html
*/

/** @defgroup WIFI_Examples
  * @{
  */

/** @defgroup WIFI_Example_Client_Socket
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define WIFI_SCAN_BUFFER_LIST           15

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
WiFi_Status_t wifi_get_AP_settings(void);

wifi_state_t wifi_state;
wifi_config config;
wifi_scan net_scan[WIFI_SCAN_BUFFER_LIST];

char console_ssid[40];
char console_psk[20];
char console_host[20];

char * ssid = "LMI-GUEST";
char * seckey = "21SimplyPossible!";
WiFi_Priv_Mode mode = WPA_Personal;
char *hostname = "index.hu";
uint8_t socket_id;

xi_context_handle_t gXivelyContextHandle = -1;


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

void user_callback_t ( xi_context_handle_t in_context_handle,
                                    void* data,
                                    xi_state_t state )
									{
printf( "\r\npublished" );
									}

void user_subscription_callback_t( xi_context_handle_t in_context_handle,
                                                 xi_sub_call_type_t call_type,
                                                 const xi_sub_call_params_t* const params,
                                                 xi_state_t state,
                                                 void* user_data )
												 {
	printf( "\r\nsubscribed, state : %i" , state );

	xi_publish( gXivelyContextHandle,
			"xi/blue/v1/15710fc1-7840-4a40-bbfa-ee80fbc38f78/d/f9f8dee6-0bcc-4127-8665-04c9b7afe056/_log",
	                              "HEHE",
	                              0,
								  XI_MQTT_RETAIN_FALSE,
	                              &user_callback_t,
	                              NULL );												 }


void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state )
{
    xi_connection_data_t* conn_data = ( xi_connection_data_t* )data;

    printf( "\r\nconnected, state : %i" , conn_data->connection_state  );

    xi_subscribe(gXivelyContextHandle,"xi/blue/v1/15710fc1-7840-4a40-bbfa-ee80fbc38f78/d/f9f8dee6-0bcc-4127-8665-04c9b7afe056/_log",0,&user_subscription_callback_t,NULL);
}


int main(void)
{
  uint8_t i=0;
  uint8_t socket_open = 0;
  uint32_t portnumber = 80;
  uint16_t len; /*Take care to change the length of the text we are sending*/
  char *protocol = "t";//t -> tcp , s-> secure tcp
  char *data = "Hello World!\r\n";
   wifi_bool SSID_found = WIFI_FALSE;

  len = strlen(data);

  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  __GPIOA_CLK_ENABLE();
  HAL_Init();

  /* Configure the system clock to 64 MHz */
  SystemClock_Config();
  SystemCoreClockUpdate();

  /* configure the timers  */
  Timer_Config( );

  UART_Configuration(115200);
#ifdef USART_PRINT_MSG
  UART_Msg_Gpio_Init();
  USART_PRINT_MSG_Configuration(115200);
#endif

  config.power=wifi_sleep;
  config.power_level=high;
  config.dhcp=on;//use DHCP IP address
  config.web_server=WIFI_TRUE;

  wifi_state = wifi_state_idle;

  status = wifi_get_AP_settings();
  if(status!=WiFi_MODULE_SUCCESS)
  {
    printf("\r\nError in AP Settings");
    return 0;
  }

  printf("\r\n\nInitializing the wifi module...");

  /* Init the wi-fi module */  
  status = wifi_init(&config);
  if(status!=WiFi_MODULE_SUCCESS)
  {
    printf("Error in Config");
    return 0;
  }  

  while (1)
  {
  	xi_events_process_tick( );
	printf("\r\nwifi state %i",wifi_state);
    switch (wifi_state) 
    {
      case wifi_state_reset:
        break;

      case wifi_state_ready:

        printf("\r\n >>running WiFi Scan...\r\n");

        status = wifi_network_scan(net_scan, WIFI_SCAN_BUFFER_LIST);

        if(status==WiFi_MODULE_SUCCESS)
        {
          for (i=0; i < WIFI_SCAN_BUFFER_LIST; i++)
            {
                //printf(net_scan[i].ssid);   
                //printf("\r\n");
                    if(( (char *) strstr((const char *)net_scan[i].ssid,(const char *)console_ssid)) !=NULL)
                    {
                        printf("\r\n >>network present...connecting to AP...\r\n");
                        SSID_found = WIFI_TRUE;
                        wifi_connect(console_ssid,console_psk, mode);
                        break;
                    }
            }
            if(!SSID_found) 
            {
              printf("\r\nGiven SSID not found!\r\n");
            }
            memset(net_scan, 0x00, sizeof(net_scan));
        }
        wifi_state = wifi_state_idle;

        break;

      case wifi_state_connected:

        printf("\r\n >>connected...\r\n");

        wifi_state = wifi_state_idle;

        HAL_Delay(2000);//Let module go to sleep

        wifi_wakeup(WIFI_TRUE);/*wakeup from sleep if module went to sleep*/        

        break;

      case wifi_state_disconnected:
        wifi_state = wifi_state_reset;
        break;

      case wifi_state_socket:
        printf("\r\n >>Connecting to socket\r\n");

        if(socket_open == 0)
          {
            /* Read Write Socket data */
            xi_state_t ret_state = xi_initialize( "15710fc1-7840-4a40-bbfa-ee80fbc38f78", "mtoth@logmein.com", 0 );

            if ( XI_STATE_OK != ret_state )
            {
                printf( "\r\n xi failed to initialise\n" );
                return -1;
            }

            gXivelyContextHandle = xi_create_context();

            if ( XI_INVALID_CONTEXT_HANDLE == gXivelyContextHandle )
            {
                printf( "\r\n xi failed to create context, error: %d\n",
                                   -gXivelyContextHandle );
                return -1;
            }

            xi_state_t connect_result = xi_connect( gXivelyContextHandle, "f9f8dee6-0bcc-4127-8665-04c9b7afe056", "K9N14DhRtHOJFtRfoJoEJTQWq26XQzs3bUY7tc/ZCKE=", 10, 0, XI_SESSION_CLEAN, &on_connected );

            printf("\r\n >>connected result %i\r\n", connect_result );

//            WiFi_Status_t status = WiFi_MODULE_SUCCESS;
//            status = wifi_socket_client_open((uint8_t *)console_host, portnumber, (uint8_t *)protocol, &socket_id);
//
//            if(status == WiFi_MODULE_SUCCESS)
//              {
//                printf("\r\n >>Socket Open OK\r\n");
//                printf("\r\n >>Socket ID: %d",socket_id);
//                socket_open = 1;
//                status = wifi_socket_client_write(socket_id, len, data);
//
//                if(status == WiFi_MODULE_SUCCESS)
//                  {
//                    printf("\r\n >>Socket Write OK\r\n");
//                  }
//              }
//            else
//              {
//                printf("Socket connection Error");
//              }
          }
        else
          {
            printf("Socket not opened!");
          }

        wifi_state = wifi_state_idle;
        break;

      case wifi_state_idle:
        printf(".");
        fflush(stdout);
        HAL_Delay(500);
          break;    

      default:
          break;
    }
  }
}

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

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Configure PLL ------------------------------------------------------*/
  /* PLL configuration: PLLCLK = (HSI / 2) * PLLMUL = (8 / 2) * 16 = 64 MHz */
  /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 64 / 1 = 64 MHz */
  /* Enable HSI and activate PLL with HSi_DIV2 as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_OFF;
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscinitstruct.HSEPredivValue    = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
    {
      /* Initialization Error */
      while(1); 
    }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
    {
      /* Initialization Error */
      while(1); 
    }
}
#endif

#ifdef USE_STM32F4XX_NUCLEO

void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSI Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
   
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}
#endif

#ifdef USE_STM32L0XX_NUCLEO

/**
 * @brief  System Clock Configuration
 * @param  None
 * @retval None
 */
  void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 0x10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);//RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;//RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  __SYSCFG_CLK_ENABLE(); 
}
#endif

#ifdef USE_STM32L4XX_NUCLEO
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
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
}
#endif

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @brief  Query the User for SSID, password, encryption mode and hostname
  * @param  None
  * @retval WiFi_Status_t
  */
WiFi_Status_t wifi_get_AP_settings(void)
{
  uint8_t console_input[1], console_count=0;
//  wifi_bool set_AP_config = WIFI_FALSE;
  WiFi_Status_t status = WiFi_MODULE_SUCCESS;
  printf("\r\n\n/********************************************************\n");
  printf("\r *                                                      *\n");
  printf("\r * X-CUBE-WIFI1 Expansion Software v2.1.1               *\n");
  printf("\r * X-NUCLEO-IDW01M1 Wi-Fi Configuration.                *\n");
  printf("\r * Client-Socket Example                                *\n");
  printf("\r *                                                      *\n");
  printf("\r *******************************************************/\n");
  printf("\r\nDo you want to setup SSID?(y/n):");
  fflush(stdout);
  scanf("%s",console_input);
  printf("\r\n");

  //HAL_UART_Receive(&UartMsgHandle, (uint8_t *)console_input, 1, 100000);
  if(console_input[0]=='y') 
        {
//              set_AP_config = WIFI_TRUE;  
              printf("Enter the SSID:");
              fflush(stdout);

              console_count=0;
              console_count=scanf("%s",console_ssid);
              printf("\r\n");

                if(console_count==39) 
                    {
                        printf("Exceeded number of ssid characters permitted");
                        return WiFi_NOT_SUPPORTED;
                    }    

              //printf("entered =%s\r\n",console_ssid);
              printf("Enter the password:");
              fflush(stdout);
              console_count=0;
              
              console_count=scanf("%s",console_psk);
              printf("\r\n");
              //printf("entered =%s\r\n",console_psk);
                if(console_count==19) 
                    {
                        printf("Exceeded number of psk characters permitted");
                        return WiFi_NOT_SUPPORTED;
                    }    
              printf("Enter the encryption mode(0:Open, 1:WEP, 2:WPA2/WPA2-Personal):"); 
              fflush(stdout);
              scanf("%s",console_input);
              printf("\r\n");
              //printf("entered =%s\r\n",console_input);
              switch(console_input[0])
              {
                case '0':
                  mode = None;
                  break;
                case '1':
                  mode = WEP;
                  break;
                case '2':
                  mode = WPA_Personal;
                  break;
                default:
                  printf("\r\nWrong Entry. Priv Mode is not compatible\n");
                  return WiFi_NOT_SUPPORTED;              
              }

              printf("Enter the Hostname (Apache Server): ");
              fflush(stdout);
              console_count=0;
              
              console_count=scanf("%s",console_host);
              printf("\r\n");
              //printf("entered =%s\r\n",console_host);
                if(console_count==19) 
                    {
                        printf("Exceeded number of host characters permitted");
                        return WiFi_NOT_SUPPORTED;
                    }
        } else 
            {
                printf("\r\n\n Module will connect with default settings.");
                memcpy(console_ssid, (const char*)ssid, strlen((char*)ssid));
                memcpy(console_psk,  (const char*)seckey, strlen((char*)seckey));
                memcpy(console_host, (const char*)hostname, strlen((char*)hostname));
            }

  printf("\r\n\n/**************************************************************\n");
  printf("\r * Configuration Complete                                     *\n");
  printf("\r * Port Number:32000, Protocol: TCP/IP                        *\n");
  printf("\r * Please make sure a server is listening at given hostname   *\n");
  printf("\r *************************************************************/\n");
  
  return status;
}

/******** Wi-Fi Indication User Callback *********/


//void ind_wifi_socket_data_received(uint8_t socket_id, uint8_t * data_ptr, uint32_t message_size, uint32_t chunk_size)
//{
//  printf("\r\nData Receive Callback...\r\n");
//  printf((const char*)data_ptr);
//  printf("\r\nsocket ID: %d\r\n",socket_id);
//  printf("msg size: %lu\r\n",(unsigned long)message_size);
//  printf("chunk size: %lu\r\n",(unsigned long)chunk_size);
//
//
//  uint8_t* chunk = malloc( message_size );
//  memcpy( chunk , data_ptr, message_size );
//
//  fflush(stdout);
//}

//void ind_wifi_socket_client_remote_server_closed(uint8_t * socket_closed_id)
//{
//  uint8_t id = *socket_closed_id;
//  printf("\r\n>>User Callback>>remote server socket closed\r\n");
//  printf("Socket ID closed: %d",id);//this will actually print the character/string, not the number
//  fflush(stdout);
//
//}

void ind_wifi_on()
{
  printf("\r\nWiFi Initialised and Ready..\r\n");
  wifi_state = wifi_state_ready;
}

void ind_wifi_connected()
{
  wifi_state = wifi_state_connected;
}

void ind_wifi_resuming()
{
  printf("\r\nwifi resuming from sleep user callback... \r\n");
  //Change the state to connect to socket if not connected
  wifi_state = wifi_state_socket;
}

/**
  * @}
  */
  
/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
