/* NOTE: If we'd like to save memory, there's no need to allocate a new user_data_t
 *       datasctructure when the data in flash is valid. A simple
 *       (user_data_t*)FLASH_USER_DATA_BASE will give you a pointer to the data in flash.
 *       It cannot be written to directly (flash erase and program are required for
 *       that), but it can be CRC-validated and accessed from anywhere in the codebase
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"
#include "user_data.h"

#define CRC_REGISTER_BYTES 4
static int8_t calculate_checksum( user_data_t* user_data, int32_t* out_checksum );

#define FLASH_USER_DATA_SECTOR FLASH_SECTOR_TOTAL-1
#define FLASH_VOLTAGE_RANGE    FLASH_VOLTAGE_RANGE_1
#define FLASH_USER_DATA_PREFIX 0x55
#define FLASH_USER_DATA_SIZE 0xfff
#define FLASH_USER_DATA_BASE (FLASH_END - FLASH_USER_DATA_SIZE)
static int8_t erase_flash_sector( void );
static int8_t program_flash( user_data_t* user_data );

/******************************************************************************
*                           Workflow Implementation                           *
******************************************************************************/

/**
 * @brief  Verify this device is compatible with the hardcoded flash
 *         sector/address specifications
 * @param   None
 * @retval 0 Success
 * @retval <0 Error: Something is probably misconfigured in the source code
 */
int8_t user_data_flash_init( void )
{
    assert( 0 == sizeof( user_data_t ) % sizeof( int32_t ) ); /* For program_flash() */
    assert( 0 == USER_DATA_STR_LEN % CRC_REGISTER_BYTES );
    if ( !IS_FLASH_SECTOR( FLASH_USER_DATA_SECTOR ) )
    {
        printf( "\r\n>> Flash init [ERROR] Sector [%d] is incompatible with this device",
                FLASH_USER_DATA_SECTOR );
        return -1;
    }
    if ( sizeof( user_data_t ) > FLASH_USER_DATA_SIZE )
    {
        printf( "\r\n>> Flash init [ERROR] Allocated flash meomry not enough" );
        return -1;
    }

    return 0;
}

/**
 * @brief  Copy the Flash area reserved for user data into *dst so it can be
 *         manipulated
 * @param  *dst is the pre-allocated pointer where we want to copy the user data
 * @retval <0: Error, >=0: OK
 */
int8_t user_data_copy_from_flash( user_data_t* dst )
{
    memset( dst, 0x00, sizeof( user_data_t ) );
    memcpy( dst, ( user_data_t* )FLASH_USER_DATA_BASE, sizeof( user_data_t ) );
    return 0;
}

/**
 * @brief  This function calculates and sets the CRC checksum of the user data
 *         we'd like to save, erases the relevant Fash sector and stores the new
 *         data
 * @param  *src is the datastructure we'd like to save to flash. src->crc_checksum
 *         will be overwritten
 * @retval <0: Error, >=0: OK
 */
/* TODO: This execution branch should run entirely from volatile RAM if possible */
int8_t user_data_save_to_flash( user_data_t* src )
{
    if ( calculate_checksum( src, &src->crc_checksum ) < 0 )
    {
        printf( "\r\n\tCRC checksum calculation [ERROR]" );
        return -1;
    }

    if ( user_data_reset_flash() < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }

    printf( "\r\n\tUser data being saved to flash:" );
    user_data_printf( src );

    if ( program_flash( src ) < 0 )
    {
        printf( "\r\n\tSaving user data to flash [ABORT]" );
        return -1;
    }
    return 0;
}

/**
 * @brief  Performs a Flash erase on the section reserved for user data
 * @retval <0: Error, >=0: OK
 */
/* TODO: This execution branch should run entirely from volatile RAM if possible */
int8_t user_data_reset_flash( void )
{
    /* TODO: Save all non-user_data in the sector before the erase */
    if ( erase_flash_sector() < 0 )
    {
        return -1;
    }
    /* TODO: Rewrite all non-user_data to flash */
    return 0;
}

/******************************************************************************
*                            FLASH MANIPULATION                               *
*                               program/erase                                 *
******************************************************************************/

/**
 * @brief  Erases the entire flash sector where the user data is stored. This
 *         will erase way more space than we need for user data; if the codebase
 *         grows too big and the linker script remains unchanged, machine code
 *         could end up allocated in this sector, and overwritten when this
 *         function is called.
 *         In order to avoid that, read the FLASH_USER_DATA_SECTOR before
 *         calling this function and re-write all inadvertently erased data
 * @retval <0: Error, >=0: OK
 */
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
        printf( "\r\n\tFlash lock [ERROR] Code [%d]\r\n", status );
        return -1;
    }
    return 0;

error_out:
    HAL_FLASH_Lock();
    return -1;
}

/**
 * @brief  Operate the Flash interface to copy *user_data into flash memory.
 *         It locks and unlocks the flash interface, and Programs the data 32-bits
 *         at a time
 * @param  *user_data is the datastructure we'd like to write to flash
 * @retval <0: Error, >=0: OK
 */
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
        runtime_data_32t_head = ( int32_t* )( i + ( char* )user_data );
        flash_data_32t_head   = ( int32_t* )( i + FLASH_USER_DATA_BASE );
        status =
            HAL_FLASH_Program( FLASH_TYPEPROGRAM_WORD, ( uint32_t )flash_data_32t_head,
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
        printf( "\r\nFlash lock [ERROR] Code [%d]\r\n", status );
        return -1;
    }
    return 0;

error_out:
    HAL_FLASH_Lock();
    return -1;
}

/******************************************************************************
*                 CRC Operations for Data Integrity Validation                *
******************************************************************************/
/**
 * @brief  Calculate a checksum of everything in *user_data but the CRC code itself.
 * @param  user_data will be read field by field (excluding crc_checksum) and its
 *         checksums will be accumulated
 * @param  *out_checksum will be set to the result of the CRC calculation
 * @retval <0: Error, >=0: OK
 */
static int8_t calculate_checksum( user_data_t* user_data, int32_t* out_checksum )
{
    char* string_ptrs[] = {user_data->wifi_client_ssid,
                           user_data->wifi_client_password,
                           user_data->xi_account_id,
                           user_data->xi_device_id,
                           user_data->xi_device_password};
    uint32_t wifi_encryption_32t = ( uint32_t )user_data->wifi_client_encryption_mode;
    uint32_t crc_code            = 0x00000000;
    static CRC_HandleTypeDef crc_config;

    /* Init CRC peripheral */
    crc_config.Instance = CRC;
    __HAL_RCC_CRC_CLK_ENABLE();
    if ( HAL_OK != HAL_CRC_Init( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }

    /* Reset CRC register and calculate the first code */
    crc_code = HAL_CRC_Calculate( &crc_config, &wifi_encryption_32t, 1 );

    /* Iterate through the relevant strings and accumulate their results */
    printf( "\r\n>> Computing CRC checksum for user data: [0x%08lx] ", crc_code );
    for ( uint32_t i = 0; i < sizeof( string_ptrs ) / sizeof( char* ); i++ )
    {
        crc_code = HAL_CRC_Accumulate( &crc_config, ( uint32_t* )string_ptrs[i],
                                       USER_DATA_STR_LEN / CRC_REGISTER_BYTES );
        printf( "+ [0x%08lx] ", crc_code );
    }
    printf( "= [0x%08lx]", crc_code );
    *out_checksum = crc_code;

    /* DeInit */
    if ( HAL_OK != HAL_CRC_DeInit( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }
    __HAL_RCC_CRC_CLK_DISABLE();
    return 0;
}

/**
 * @brief  Calculate the CRC checksum of everything in *user_data but its CRC
 *         checksum itself. If the CRC checksum we calculated matches the one
 *         in the datastructure, return 0. Otherwise, return <0
 * @param  *user_data is the structure whose integrity/usage is to be validated
 * @retval 0: OK
 * @retval -1: Failed to calculate checksum
 * @retval -2: Calculated checksum doesn't match the one in user_data
 */
int8_t user_data_validate_checksum( user_data_t* user_data )
{
    int32_t new_checksum = 0x00000000;
    if ( calculate_checksum( user_data, &new_checksum ) < 0 )
    {
        return -1;
    }
    if ( new_checksum != user_data->crc_checksum )
    {
        printf( "\r\n>> User data [CORRUPT] stored checksum %08lx doesn't match %08lx",
                new_checksum, user_data->crc_checksum );
        return -2;
    }
    printf( "\r\n>> User data integrity validation [OK]" );
    return 0;
}

/******************************************************************************
*                                     Helpers                                 *
******************************************************************************/
/**
 * @brief Print a debug message displaying the contents of user_data, both in
 *        binary format and ASCII.
 *        A memory dump of the data itself is performed as to make it obvious to
 *        the developer whether there was anything stored in flash, help debug
 *        issues when modifying the user_data_t datastructure, etc.
 * @param user_data is the structure we'd like to print to UART
 */
void user_data_printf( user_data_t* user_data )
{
    for ( uint32_t i = 0; i < sizeof( user_data_t ) / sizeof( int32_t ); i++ )
    {
        if ( 0 == i % 4 )
        {
            printf( "\r\n\t\t" );
        }
        printf( "%08lx ", *( ( int32_t* )user_data + i ) );
    }
    printf( "\r\n\t* WiFi Security: [%ld]", user_data->wifi_client_encryption_mode );
    printf( "\r\n\t* WiFi SSID: [%.64s]", user_data->wifi_client_ssid );
    printf( "\r\n\t* WiFi Pwd: [%.64s]", user_data->wifi_client_password );
    printf( "\r\n\t* Xi Acc ID: [%.64s]", user_data->xi_account_id );
    printf( "\r\n\t* Xi Dev ID: [%.64s]", user_data->xi_device_id );
    printf( "\r\n\t* Xi Dev Pwd: [%.64s]", user_data->xi_device_password );
    printf( "\r\n\t* CRC Checksum: [0x%08lx]", user_data->crc_checksum );
}
