/*
Usage:
    1. flash_init()
    2. flash_get_user_data( ... )
        2.1. Read entire sector from flash
        2.2. Parse user data from the read memory sector 
        2.3. Identify
*/
/*
void writeFlash(void)
{
    HAL_StatusTypeDef status;
    status =    HAL_FLASH_Unlock();
    status= HAL_FLASH_Program(TYPEPROGRAM_HALFWORD,(uint32_t) userConfig; 
    status = HAL_FLASH_Lock();
}

void readFlash(void)
{
    tempvalue = *(uint8_t *)(userConfig);
}
*/
#include <assert.h>
#include <stdint.h>

#include "stm32f4xx.h"
#include "user_data.h"

#define FLASH_VOLTAGE_RANGE FLASH_VOLTAGE_RANGE_1
#define FLASH_USER_DATA_PREFIX 0x55
#define CRC_REGISTER_SIZE_BYTES 4
/**
  * FLASH_USER_DATA_SECTOR:
  * If your device doesn't have these many sectors, add an #ifdef for it here
  * (or move to demo_bsp.h) and use your device's last sector. The SDK doesn't
  * provide a more portable way of finding out the last available sector
  */
#define FLASH_USER_DATA_SECTOR FLASH_SECTOR_TOTAL-1

#define FLASH_USER_DATA_SIZE 0xfff
#define FLASH_USER_DATA_BASE (FLASH_END - FLASH_USER_DATA_SIZE)

int8_t erase_sector( void );
/**
  * @brief  Get the memory are we're going to erase so we can re-use most of
  * @param
  * @retval  0 Success
  * @retval <0 Error
  */
//static int8_t serialize_data_into_sector_conents( char* read_sector,
//                                                  char* user_data_str,
//                                                  char* checksum );

/**
  * @brief  Calculate a checksum of the user data stored in @param; set its
  *         'flash_data_checksum' field
  * @param  user_data structure to checksum and update
  * @retval  0 Success
  * @retval <0 Error
  */ 
//static int8_t calculate_checksum( user_data_t* user_data );

/**
  * @brief  Verify this device is compatible with the hardcoded flash
  *         sector/address specifications
  * @param   None
  * @retval 0 Success
  * @retval -1 User data storage in flash not supported for your device
  */
int8_t flash_init( void )
{
    //assert( 0 == DATA_STR_LEN % CRC_REGISTER_SIZE_BYTES );
    if ( !IS_FLASH_SECTOR( FLASH_USER_DATA_SECTOR ) )
    {
        printf( "\r\n>> Flash init [ERROR] Sector [%d] is incompatible with this device",
                FLASH_USER_DATA_SECTOR );
        return -1;
    }
    return 0;
}

/*
 *  1. check if FLASH_USER_DATA_PREFIX is in flash data. If not, return -1(not configured)
 *  2. verify data integrity by CRC checksum comparison. On error, return -2(corrupt data)
 *  3. On data available and verified, return 0. Assign to @param the memory
 *     location where the data is stored (mapped flash memory address)
 */
int8_t flash_get_user_data( user_data_t* user_data )
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
    printf( "\r\nFLAH READ DONE\r\n" );
    return 0;
}

int8_t flash_delete_user_data( void )
{
    erase_sector();
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

int8_t flash_set_user_data( user_data_t* user_data )
{
    HAL_StatusTypeDef status;
    uint32_t addr = 0;

    /* Erase */
    erase_sector();

    /* Unlock FLASH interface */
    printf( "\r\nUnlocking Flash at HAL" );
    status = HAL_FLASH_Unlock();
    if ( HAL_OK != status )
    {
        printf( "\r\n>> Flash unlock [ERROR] CODE [%d]\r\n", status );
    }

    /* Write - TODO: Move to a separate function */
    printf( "\r\n/************ FLASH OVERWRITE ************/\r\n" );
    for ( addr = FLASH_USER_DATA_BASE; addr < FLASH_END; addr++ )
    {
        status = HAL_FLASH_Program( TYPEPROGRAM_BYTE, ( uint32_t )addr, 0x5555555555555555 );
        if ( HAL_OK != status )
        {
            printf( "\r\nFLAH PROGRAM ERROR CODE [%d]\r\n", status );
        }
        else// if ( addr % 128 == 0 )
        {
            printf( "[0x%04lx] ", addr );
        }
    }

    /* Lock FLASH interface */
    status = HAL_FLASH_Lock();
    if ( HAL_OK != status )
    {
        printf( "\r\nFLAH LOCK ERROR CODE [%d]\r\n", status );
    }
    return 0;
}

int8_t erase_sector( void )
{
    printf( "\r\n>> Erasing FLASH user sector" );
    FLASH_EraseInitTypeDef erase_config;
    erase_config.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_config.Sector       = FLASH_USER_DATA_SECTOR;
    erase_config.VoltageRange = FLASH_VOLTAGE_RANGE;
    erase_config.NbSectors    = 1;
    uint32_t error;

    HAL_FLASH_Unlock();
    if ( HAL_OK != HAL_FLASHEx_Erase( &erase_config, &error ) )
    {
        goto error_out;
    }
    if ( 0xFFFFFFFFU != error )
    {
        goto error_out;
    }
    HAL_FLASH_Lock();
    return 0;

error_out:
    HAL_FLASH_Lock();
    printf( "\r\n\tFlash erase [ERROR] code %08lx", error );
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

int8_t crc_accumulate_data_str( char* str, uint32_t str_len )
{
#if 0
    assert( 0 == str_len % CRC_REGISTER_SIZE_BYTES );
    for(uint32_t i=0; i<str_len; i+= CRC_REGISTER_SIZE_BYTES)
    {
    }
#endif
    return 0;
}

#if 0
static int8_t calculate_checksum( user_data_t* user_data )
{
    uint32_t i = 0;
    const size_t data_size = sizeof( user_data_t );
    const CRC_HandleTypeDef crc_config = 0xff; //TODO: ??
    uint32_t wifi_cipher_32t = (uint32_t)user_data->wifi_client_cipher;
    char* verifiable_data_strings = 

    /* Init */
    __HAL_RCC_CRC_CLK_ENABLE();

    /* Reset CRC register and calculate the first code */
    HAL_CRC_Calculate( &crc_config, &wifi_cipher_32t, 1 );

    /* Iterate through the relevant strings and accumulate their results */
    if ( HAL_OK != HAL_CRC_Init( &crc_config ) )
    {
        printf( "\r\n>> CRC HAL Driver initialization [ERROR]" );
        return -1
    }
    for ( ; i < data_size; i += 32 )
    {
        if( (data_size - i) < 32 )
    }

    /* DeInit */
    __HAL_RCC_CRC_CLK_DISABLE();
    return 0;
}
#endif
