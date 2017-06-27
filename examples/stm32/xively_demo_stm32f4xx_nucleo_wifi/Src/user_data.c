/* PRODUCTION WARNING:
 *          This is a sample implementation on how to store user data to flash.
 *          For portability and simplicity reasons, we're storing the data in
 *          the last $FLASH_USER_DATA_ALLOCATED_SIZE bytes of the last sector in
 *          flash.
 *          We are NOT reserving this memory area in the linker script, which means
 *          if the codebase grew large enough, some of the program may end up
 *          in the same sector as the user data.
 *          In order to store anything to flash memory, the programmer needs
 *          to 'erase' the entire sector before writing to it.
 *          The combination of those 3 factors means that if the codebase grew big
 *          enough, and a user tried to set/update user data, part of the code
 *          would be erased and the device would probably be bricked.
 *
 *          For production, you should use the linker script to reserve the entire flash
 *          sector for user data.
 *          You can also share that sector with other code/data by reading the
 *          entire sector before erasing it, and re-writing all unrelated data after
 *          the fact.
 *          If there might be code in that sector (NOT recommended), you also need to make
 *          sure the functions that erase and rewrite it run entirely from volatile RAM;
 *          that way they can't accidentally overwrite themselves during the process.
 */
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
#include "stm32f4xx_hal_crc.h"
#include "user_data.h"

#define CRC_REGISTER_SIZE sizeof( int32_t )
static int8_t calculate_checksum( user_data_t* user_data, int32_t* out_checksum );

#define FLASH_USER_DATA_SECTOR FLASH_SECTOR_TOTAL - 1
#define FLASH_VOLTAGE_RANGE FLASH_VOLTAGE_RANGE_1
#define FLASH_USER_DATA_ALLOCATED_SIZE 0xfff
#define FLASH_USER_DATA_BASE ( FLASH_END - FLASH_USER_DATA_ALLOCATED_SIZE )
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
    assert( 0 == USER_DATA_STR_SIZE % CRC_REGISTER_SIZE );

    if ( !IS_FLASH_SECTOR( FLASH_USER_DATA_SECTOR ) )
    {
        printf( "\r\n>> Flash init [ERROR] Sector [%d] is incompatible with this device",
                FLASH_USER_DATA_SECTOR );
        return -1;
    }

    if ( sizeof( user_data_t ) > FLASH_USER_DATA_ALLOCATED_SIZE )
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
    if ( NULL == dst )
    {
        return -1;
    }

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
/* NOTE: This execution branch should run entirely from volatile RAM if possible */
int8_t user_data_save_to_flash( user_data_t* src )
{
    if ( NULL == src )
    {
        return -1;
    }

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
/* NOTE: This execution branch should run entirely from volatile RAM if possible */
int8_t user_data_reset_flash( void )
{
    /* NOTE: Ideally, now we'd save all non-user_data in the sector before the erase */
    if ( erase_flash_sector() < 0 )
    {
        return -1;
    }
    /* NOTE: Ideally, now we'd rewrite all non-user_data to flash */
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
    if ( NULL == user_data )
    {
        return -1;
    }

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
#if USER_DATA_WIFI_DEVICE
    char* string_ptrs[] = {user_data->wifi_client_ssid,
                           user_data->wifi_client_password,
                           user_data->xi_account_id,
                           user_data->xi_device_id,
                           user_data->xi_device_password};
    uint32_t wifi_encryption_u32t = ( uint32_t )user_data->wifi_client_encryption_mode;
#else
    char* string_ptrs[] = {user_data->xi_account_id,
                           user_data->xi_device_id,
                           user_data->xi_device_password};
#endif /* USER_DATA_WIFI_DEVICE */
    uint32_t crc_code = 0x00000000;
    static CRC_HandleTypeDef crc_config;
    uint32_t i = 0;

    if ( ( NULL == user_data ) || ( NULL == out_checksum ) )
    {
        return -1;
    }

    /* Init CRC peripheral */
    crc_config.Instance = CRC;
    __HAL_RCC_CRC_CLK_ENABLE();
    if ( HAL_OK != HAL_CRC_Init( &crc_config ) )
    {
        printf( "\r\n\tCRC HAL Driver initialization [ERROR]" );
        return -1;
    }

    /* Reset CRC register and calculate the first code */
    crc_code = HAL_CRC_Calculate( &crc_config, ( uint32_t* )string_ptrs[i],
                                  USER_DATA_STR_SIZE / CRC_REGISTER_SIZE );

    /* Iterate through the data strings and accumulate their results */
    printf( "\r\n>> Computing CRC checksum for user data: [0x%08lx]", crc_code );
    for ( ++i; i < sizeof( string_ptrs ) / sizeof( char* ); i++ )
    {
        crc_code = HAL_CRC_Accumulate( &crc_config, ( uint32_t* )string_ptrs[i],
                                       USER_DATA_STR_SIZE / CRC_REGISTER_SIZE );
        printf( " -> [0x%08lx]", crc_code );
    }

#if USER_DATA_WIFI_DEVICE
    /* Accumulate checksum of the WiFi encryption mode integer */
    crc_code = HAL_CRC_Accumulate( &crc_config, &wifi_encryption_u32t,
                                   sizeof( wifi_encryption_u32t ) / CRC_REGISTER_SIZE );
    printf( " -> [0x%08lx]", crc_code );
#endif /* USER_DATA_WIFI_DEVICE */

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
                user_data->crc_checksum, new_checksum );
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
#if 0
    /* This debug message dumps the given *user_data memory block. Very useful
    to find possible errors when modifying the user_data_t datastructure */
    for ( uint32_t i = 0; i < sizeof( user_data_t ) / sizeof( int32_t ); i++ )
    {
        if ( 0 == i % 4 )
        {
            printf( "\r\n\t\t" );
        }
        printf( "%08lx ", *( ( int32_t* )user_data + i ) );
    }
#endif

#if USER_DATA_WIFI_DEVICE
    printf( "\r\n\t* WiFi Security: [%ld]", user_data->wifi_client_encryption_mode );
    printf( "\r\n\t* WiFi SSID: [%.64s]", user_data->wifi_client_ssid );
    //printf( "\r\n\t* WiFi Pwd: [%.64s]", user_data->wifi_client_password );
    printf( "\r\n\t* WiFi Pwd: ( REDACTED )" );
#endif /* USER_DATA_WIFI_DEVICE */
    printf( "\r\n\t* Xi Acc ID: [%.64s]", user_data->xi_account_id );
    printf( "\r\n\t* Xi Dev ID: [%.64s]", user_data->xi_device_id );
    //printf( "\r\n\t* Xi Dev Pwd: [%.64s]", user_data->xi_device_password );
    printf( "\r\n\t* Xi Dev Pwd: ( REDACTED )" );
    printf( "\r\n\t* CRC Checksum: [0x%08lx]", user_data->crc_checksum );
}
