/*
Usage:
    1. user_data_init()
    2. user_data_read_flash( ... )
        2.1. Read entire sector from flash
        2.2. Parse user data from the read memory sector 
        2.3. Identify
*/
#include <assert.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"

#include "user_data.h"

/**
  * FLASH_USER_DATA_SECTOR:
  * If your device doesn't have these many sectors, add an #ifdef for it here
  * (or move to demo_bsp.h) and use your device's last sector. The SDK doesn't
  * provide a more portable way of finding out the last available sector
  */
#define FLASH_USER_DATA_SECTOR FLASH_SECTOR_TOTAL-1
#define FLASH_VOLTAGE_RANGE    FLASH_VOLTAGE_RANGE_1
#define FLASH_USER_DATA_PREFIX 0x55
#define CRC_REGISTER_SIZE_BYTES 4

#define FLASH_USER_DATA_SIZE 0xfff
#define FLASH_USER_DATA_BASE (FLASH_END - FLASH_USER_DATA_SIZE)

static int8_t erase_flash_sector( void );
static int8_t program_flash( user_data_t* user_data );

user_data_t* demo_user_data = NULL;

/**
  * @brief  Calculate a checksum of the user data stored in @param; set its
  *         'flash_data_checksum' field
  * @param  user_data structure to checksum and update
  * @retval  0 Success
  * @retval <0 Error
  */ 
int8_t calculate_checksum( user_data_t* user_data );

/**
  * @brief  Verify this device is compatible with the hardcoded flash
  *         sector/address specifications
  * @param   None
  * @retval 0 Success
  * @retval -1 User data storage in flash not supported for your device
  */
int8_t user_data_init( void )
{
#if USE_FLASH_STORAGE
    //assert( 0 == DATA_STR_LEN % CRC_REGISTER_SIZE_BYTES );
    if ( !IS_FLASH_SECTOR( FLASH_USER_DATA_SECTOR ) )
    {
        printf( "\r\n>> Flash init [ERROR] Sector [%d] is incompatible with this device",
                FLASH_USER_DATA_SECTOR );
        return -1;
    }
    if ( sizeof(user_data_t) > FLASH_USER_DATA_SIZE )
    {
        printf( "\r\n>> Flash init [ERROR] Allocated flash meomry not enough" );
        return -1;
    }
    demo_user_data = FLASH_USER_DATA_BASE;
#else
    demo_user_data = malloc( sizeof( user_data_t ) );
#endif
    return 0;
}

/*
 *  1. check if FLASH_USER_DATA_PREFIX is in flash data. If not, return -1(not configured)
 *  2. verify data integrity by CRC checksum comparison. On error, return -2(corrupt data)
 *  3. On data available and verified, return 0. Assign to @param the memory
 *     location where the data is stored (mapped flash memory address)
 */
int8_t user_data_read_flash( user_data_t* user_data )
{
    uint32_t addr = 0;

    printf( "\r\n/************ FLASH DUMP ************/\r\n" );
    for ( addr = FLASH_USER_DATA_BASE; addr < FLASH_END; addr++ )
    {
        printf( "0x%02x ", *( uint8_t* )( addr ) );
        if ( addr % 32 == 0 )
        {
            printf( "\r\n" );
        }
    }
    printf( "\r\nFLASH READ DONE\r\n" );
    return 0;
}

int8_t user_data_reset_flash( void )
{
    erase_flash_sector();
    return 0;
}

/**
  * @brief
  * @param  user data structure we'd like to get into flash
  * @retval  0 Success
  * @retval <0 Error
  */
int8_t flash_user_data_saved_status( user_data_t* user_data )
{
    //if(stored_flash_data) memcmp( user_data, $stored_flash_data )
    return 0;
}

/******************************************************************************
*                            FLASH MANIPULATION                               *
*                               program/erase                                 *
******************************************************************************/

int8_t user_data_write_flash( user_data_t* user_data )
{
    /* Erase */
    if ( erase_flash_sector() < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }

    /* TODO: Re-program all non-user data read before the erase, in case
             any part of the program is located at the last flash page */

    /* Program */
    if ( program_flash( user_data ) < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }
    return 0;
}

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
    status = HAL_FLASH_Lock();
    if ( HAL_OK != status )
    {
        printf( "\r\n\tFLASH LOCK ERROR CODE [%d]\r\n", status );
        goto error_out;
    }
    return 0;

error_out:
    HAL_FLASH_Lock();
    return -1;
}

static int8_t program_flash( user_data_t* user_data )
{
    HAL_StatusTypeDef status;
    uint32_t addr = 0;

    /* Unlock FLASH interface */
    printf( "\r\n>> Saving user data to flash" );
    if ( HAL_OK != HAL_FLASH_Unlock() )
    {
        printf( "\r\n\tFlash unlock [ERROR]" );
        return -1;
    }

    /* Write - TODO: Move to a separate function */
    for ( addr = FLASH_USER_DATA_BASE; addr < FLASH_END; addr++ )
    {
        status = HAL_FLASH_Program( TYPEPROGRAM_BYTE, ( uint32_t )addr, 0x5555555555555555 );
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
        goto error_out;
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

int32_t crc_accumulate_data_str( CRC_HandleTypeDef crc_handle, char* str, uint32_t str_len )
{
    uint32_t crc_code = 0x00;
    assert( 0 == str_len % CRC_REGISTER_SIZE_BYTES );
    //for ( uint32_t i = 0; i < str_len; i += CRC_REGISTER_SIZE_BYTES )
    //{
    //    crc_code = HAL_CRC_Accumulate( &crc_handle, ( uint32_t* )&str[i],
    //                                   str_len / CRC_REGISTER_SIZE_BYTES );
    //}
    crc_code = HAL_CRC_Accumulate( &crc_handle, ( uint32_t* )str,
                                   str_len / CRC_REGISTER_SIZE_BYTES );
    return crc_code;
}

int8_t calculate_checksum( user_data_t* user_data )
{
    char* string_ptrs[] = {user_data->wifi_client_ssid,
                           user_data->wifi_client_password,
                           user_data->xi_account_id,
                           user_data->xi_device_id,
                           user_data->xi_device_password};
    uint32_t wifi_cipher_32t = ( uint32_t )user_data->wifi_client_cipher;
    uint32_t crc_code = 0x00;
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
        crc_code = crc_accumulate_data_str( crc_config, string_ptrs[i], DATA_STR_LEN );
        printf( "\r\n\tString [%s] has CRC [%08lx]", string_ptrs[i], crc_code );
    }
    printf( "\r\n\tCalculated user data CRC code: %08lx", crc_code );

    /* DeInit */
    if ( HAL_OK != HAL_CRC_DeInit( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }
    __HAL_RCC_CRC_CLK_DISABLE();
    return 0;
}
