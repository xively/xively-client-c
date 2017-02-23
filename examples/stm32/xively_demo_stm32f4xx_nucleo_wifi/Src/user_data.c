/*
Usage:
    1. user_data_init()
    2. user_data_read_flash( ... )
        2.1. Read entire sector from flash
        2.2. Parse user data from the read memory sector 
        2.3. Identify
*/
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"
#include "user_data.h"

/******************************************************************************
*                           CRC Peripheral Usage                              *
******************************************************************************/
#define CRC_REGISTER_BYTES 4

static int8_t calculate_checksum( user_data_t* user_data );

/******************************************************************************
*                                 Flash Usage                                 *
******************************************************************************/
#define FLASH_USER_DATA_SECTOR FLASH_SECTOR_TOTAL-1
#define FLASH_VOLTAGE_RANGE    FLASH_VOLTAGE_RANGE_1
#define FLASH_USER_DATA_PREFIX 0x55
#define FLASH_USER_DATA_SIZE 0xfff
#define FLASH_USER_DATA_BASE (FLASH_END - FLASH_USER_DATA_SIZE)

static int8_t erase_flash_sector( void );
static int8_t program_flash( user_data_t* user_data );

/******************************************************************************
*             Pointers to user data, one to RAM and one to Flash              *
******************************************************************************/
user_data_t* runtime_user_data_ptr = NULL;
user_data_t* flash_user_data_ptr = FLASH_USER_DATA_BASE;

/******************************************************************************
*                           Workflow Implementation                           *
******************************************************************************/

/**
  * @brief  Verify this device is compatible with the hardcoded flash
  *         sector/address specifications
  * @param   None
  * @retval 0 Success
  * @retval -1 User data storage in flash not supported for your device
  */
user_data_t* user_data_init( void )
{
    runtime_user_data_ptr = malloc( sizeof( user_data_t ) );
    memset( runtime_user_data_ptr, 0x00, sizeof( user_data_t ) );
    user_data_printf( runtime_user_data_ptr );
#if !(USE_FLASH_STORAGE)
    return runtime_user_data_ptr;
#endif

    /* TODO: Should we kill runtime_user_data_ptr completely if flash init fails? */
    //assert( 0 == DATA_STR_LEN % CRC_REGISTER_BYTES );
    if ( !IS_FLASH_SECTOR( FLASH_USER_DATA_SECTOR ) )
    {
        printf( "\r\n>> Flash init [ERROR] Sector [%d] is incompatible with this device",
                FLASH_USER_DATA_SECTOR );
        goto error_out;
    }
    if ( sizeof(user_data_t) > FLASH_USER_DATA_SIZE )
    {
        printf( "\r\n>> Flash init [ERROR] Allocated flash meomry not enough" );
        goto error_out;
    }

    return runtime_user_data_ptr;

error_out:
    if( NULL != runtime_user_data_ptr )
    {
        free( runtime_user_data_ptr );
    }
    runtime_user_data_ptr = NULL;
    return NULL;
}

void user_data_free( void )
{
    if ( runtime_user_data_ptr != NULL )
    {
        free( runtime_user_data_ptr );
    }
}

#if 0
user_data_t* user_data_get_ptr( void )
{
    return runtime_user_data_ptr;
}
#endif

/*
 *  1. check if FLASH_USER_DATA_PREFIX is in flash data. If not, return -1(not configured)
 *  2. verify data integrity by CRC checksum comparison. On error, return -2(corrupt data)
 *  3. On data available and verified, return 0. Assign to @param the memory
 *     location where the data is stored (mapped flash memory address)
 */
int8_t user_data_read_flash( void )
{
#if !( USE_FLASH_STORAGE )
    return -2;
#endif
    if ( NULL == runtime_user_data_ptr )
    {
        return -1;
    }
    memcpy( runtime_user_data_ptr, flash_user_data_ptr, sizeof( user_data_t ) );

    /* TODO: verify flash message integrity */
    return 0;
}

int8_t user_data_save_to_flash( void )
{
#if !( USE_FLASH_STORAGE )
    return -2;
#endif
    /* TODO: Calculate CRC checksum and set it in the runtime user data */

    /* Erase */
    if ( user_data_reset_flash() < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }

    /* Program */
    if ( program_flash( runtime_user_data_ptr ) < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }
    return 0;
}

/* TODO: This execution branch should run entirely from RAM */
int8_t user_data_reset_flash( void )
{
#if !( USE_FLASH_STORAGE )
    return -2;
#endif
    /* TODO: Save all non-user_data memory before the erase */
    if ( erase_flash_sector() < 0 )
    {
        return -1;
    }
    /* TODO: Rewrite all non-user_data to flash */
    return 0;
}

void user_data_printf( user_data_t* user_data )
{
    //TODO : Verify checksum ??
    printf( "\r\n\tSnapshot of the contents of the current user data\r\n\t" );
    for ( uint32_t i = 0; i < sizeof( user_data_t ) / sizeof( int32_t ); i++ )
    { //TODO: Get rid of this debug loop
        printf( "%08lx ", *( ( int32_t* )user_data + i ) );
    }
    printf( "\r\n\t  * WiFi Cipher: %ld", user_data->wifi_client_cipher );
    if ( user_data->wifi_client_cipher == 2 ) //TODO : Remove this if
    {
        printf( "\r\n\t  * WiFi SSID:   %s", user_data->wifi_client_ssid );
        printf( "\r\n\t  * WiFi Pwd:    %s", user_data->wifi_client_password );
        printf( "\r\n" );
        printf( "\r\n\t  * Xi Acc ID:  %s", user_data->xi_account_id );
        printf( "\r\n\t  * Xi Dev ID:  %s", user_data->xi_device_id );
        printf( "\r\n\t  * Xi Dev Pwd: %s", user_data->xi_device_password );
        printf( "\r\n" );
    }
    printf( "\r\n\t  * CRC Checksum: %ld", user_data->crc_checksum );
}

/******************************************************************************
*                            FLASH MANIPULATION                               *
*                               program/erase                                 *
******************************************************************************/

static int8_t erase_flash_sector( void )
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef erase_config;
    uint32_t error;

    printf( "\r\n>> Erasing FLASH user sector" );
    erase_config.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_config.Sector       = FLASH_USER_DATA_SECTOR;
    erase_config.VoltageRange = FLASH_VOLTAGE_RANGE;
    erase_config.NbSectors    = 1;

    /* Unlock flash */
    printf( "\r\n\tUnlocking Flash at HAL" );
    status = HAL_FLASH_Unlock();
    if ( HAL_OK != status )
    {
        printf( "\r\n\tFlash unlock [ERROR] CODE [%d]\r\n", status );
        return -1;
    }

    /* Erase Sector */
    printf( "\r\n\tRunning FLASHEx_Erase" );
    if ( HAL_OK != HAL_FLASHEx_Erase( &erase_config, &error ) )
    {
        printf( "\r\n\tFlash erase [ERROR] code %08lx", error );
        goto error_out;
    }
    if ( 0xFFFFFFFFU != error )
    {
        printf( "\r\n\tFlash erase [ERROR] code %08lx", error );
        goto error_out;
    }

    /* Lock flash */
    printf( "\r\n\tRe-locking FLASH at HAL" );
    status = HAL_FLASH_Lock();
    if ( HAL_OK != status )
    {
        printf( "\r\n\tFLASH LOCK ERROR CODE [%d]\r\n", status );
        return -1;
    }
    return 0;

error_out:
    HAL_FLASH_Lock();
    return -1;
}

static int8_t program_flash( user_data_t* user_data )
{
    HAL_StatusTypeDef status;
    int32_t* runtime_data_32t_head = NULL;
    int32_t* flash_data_32t_head = NULL;

    /* Unlock FLASH interface */
    printf( "\r\n>> Saving user data to flash" );
    if ( HAL_OK != HAL_FLASH_Unlock() )
    {
        printf( "\r\n\tFlash unlock [ERROR]" );
        return -1;
    }

    /* Program new user data to flash */
    for ( uint32_t i = 0; i < sizeof( user_data_t ); i += sizeof( int32_t ) )
    {
        runtime_data_32t_head = i + ( char* )user_data;
        flash_data_32t_head   = i + FLASH_USER_DATA_BASE;
        status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, flash_data_32t_head,
                                    ( uint64_t )*runtime_data_32t_head );
        if ( HAL_OK != status )
        {
            printf( "\r\n\tFlash program [ERROR] Status code %d", status );
            goto error_out;
        }
    }

    /* Lock FLASH interface */
    status = HAL_FLASH_Lock();
    if ( HAL_OK != status )
    {
        printf( "\r\nFLASH LOCK ERROR CODE [%d]\r\n", status );
        return -1;
    }
    return 0;

error_out:
    HAL_FLASH_Lock();
    return -1;
}

/******************************************************************************
*                                HAL Callbacks                                *
*                         TODO : NOT IN USE ( I THINK )                       *
******************************************************************************/

void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
    printf( "\r\n>> FLASH END OF OPERATION. Return value: %ld\r\n", ReturnValue );
}

void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    printf( "\r\n>> FLASH OPERATION ERROR. Return value: %ld\r\n", ReturnValue );
}

/******************************************************************************
*                 CRC Operations for Data Integrity Validation                *
******************************************************************************/

/**
  * @brief  Calculate a checksum of the user data stored in @param; set its
  *         'crc_checksum' field
  * @param  user_data structure to checksum and update
  * @retval  0 Success
  * @retval <0 Error
  */ 
static int8_t calculate_checksum( user_data_t* user_data )
{
    char* string_ptrs[] = {user_data->wifi_client_ssid,
                           user_data->wifi_client_password,
                           user_data->xi_account_id,
                           user_data->xi_device_id,
                           user_data->xi_device_password};
    uint32_t wifi_cipher_32t = ( uint32_t )user_data->wifi_client_cipher;
    uint32_t crc_code = 0x00000000;
    static CRC_HandleTypeDef crc_config;
    crc_config.Instance = CRC;

    /* Init */
    __HAL_RCC_CRC_CLK_ENABLE();
    if ( HAL_OK != HAL_CRC_Init( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }

    /* Reset CRC register and calculate the first code */
    crc_code = HAL_CRC_Calculate( &crc_config, &wifi_cipher_32t, 1 );

    /* Iterate through the relevant strings and accumulate their results */
    for ( uint32_t i = 0; i < sizeof( string_ptrs ) / sizeof( char* ); i++ )
    {
        assert( 0 == sizeof( string_ptrs[i] ) % CRC_REGISTER_BYTES );
        crc_code = HAL_CRC_Accumulate( &crc_config, ( uint32_t* )string_ptrs[i],
                                       DATA_STR_LEN / CRC_REGISTER_BYTES );
        printf( "\r\n\tString [%s] has CRC [%08lx]", string_ptrs[i], crc_code );
    }
    printf( "\r\n\tCalculated user data CRC code: %08lx", crc_code );
    user_data->crc_checksum = crc_code;

    /* DeInit */
    if ( HAL_OK != HAL_CRC_DeInit( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }
    __HAL_RCC_CRC_CLK_DISABLE();
    return 0;
}
