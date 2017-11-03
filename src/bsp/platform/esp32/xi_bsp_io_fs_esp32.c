/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#if 0
#include <xi_bsp_debug.h>
#else
#define xi_bsp_debug_format printf // TODO: delete me
#define xi_bsp_debug_logger printf // TODO: delete me
#endif
#include <xi_bsp_io_fs.h>
#include <xi_bsp_fwu.h>
#include <string.h>
#include <stdio.h>

#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_ota_ops.h"

#if 1
#include <xi_list.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>
#include <xi_bsp_mem.h>
/* The size of the buffer to be used for reads. */
const size_t xi_bsp_io_fs_buffer_size = 1024;

#define XI_BSP_IO_FS_CHECK_CND( cnd, e, s )                                              \
    if ( ( cnd ) )                                                                       \
    {                                                                                    \
        ( s ) = ( e );                                                                   \
        goto err_handling;                                                               \
    }

/**
 * @brief xi_bsp_io_fs_posix_file_handle_container_t
 *
 * Database type of file handles and allocated memory chunks.
 */
typedef struct xi_bsp_io_fs_posix_file_handle_container_s
{
    FILE* posix_fp;
    uint8_t* memory_buffer;
    struct xi_bsp_io_fs_posix_file_handle_container_s* __next;
} xi_bsp_io_fs_posix_file_handle_container_t;

/* local list of open files */
static xi_bsp_io_fs_posix_file_handle_container_t* 
    xi_bsp_io_fs_posix_files_container;

/* translates bsp errno errors to the xi_bsp_io_fs_state_t values */
xi_bsp_io_fs_state_t xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( int errno_value )
{
    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;

    printf( "\n****errno: %d", errno_value );
    switch ( errno_value )
    {
        case EBADF:
        case EACCES:
        case EFAULT:
        case ENOENT:
        case ENOTDIR:
            ret = XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE;
            break;
        case ELOOP:
        case EOVERFLOW:
            ret = XI_BSP_IO_FS_ERROR;
            break;
        case EINVAL:
            printf( "\n** Errno: EINVAL -- Invalid argument" );
        case ENAMETOOLONG:
            ret = XI_BSP_IO_FS_INVALID_PARAMETER;
            break;
        case ENOMEM:
            ret = XI_BSP_IO_FS_OUT_OF_MEMORY;
            break;
        default:
            ret = XI_BSP_IO_FS_ERROR;
    }

    return ret;
}

/* helper function that translates posix stat to xi stat */
static xi_bsp_io_fs_state_t xi_bsp_io_fs_posix_stat_2_xi_bsp_io_fs_stat( const struct stat* const posix_stat,
                                                                         xi_bsp_io_fs_stat_t* const xi_stat )
{
    assert( NULL != posix_stat );
    assert( NULL != xi_stat );

    xi_stat->resource_size = posix_stat->st_size;

    return XI_BSP_IO_FS_STATE_OK;
}

/**
 * @brief xi_bsp_io_fs_posix_file_list_cnd
 *
 * Functor for list find function.
 *
 * @param list
 * @param fp
 * @return 1 if list element is the one with the matching fp 0 otherwise
 */
static uint8_t
xi_bsp_io_fs_posix_file_list_cnd( xi_bsp_io_fs_posix_file_handle_container_t* list_element,
                                  FILE* fp )
{
    assert( NULL != list_element );
    assert( NULL != fp );

    return ( list_element->posix_fp == fp ) ? 1 : 0;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_stat( const char* const resource_name, xi_bsp_io_fs_stat_t* resource_stat )
{
    if ( NULL == resource_stat || NULL == resource_name )
    {
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;

    struct stat stat_struct;
    memset( &stat_struct, 0, sizeof( stat_struct ) );

    int res = stat( resource_name, &stat_struct );

    /* Verification of the os function result.
     * Jump to err_handling label in case of failure. */
    XI_BSP_IO_FS_CHECK_CND( 0 != res, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

    /* here we translate stat posix os stat structure to libxively version */
    ret = xi_bsp_io_fs_posix_stat_2_xi_bsp_io_fs_stat( &stat_struct, resource_stat );

err_handling:
    return ret;
}
#endif

#define ESP32_FS_BASE_PATH "/spiflash"
#define ESP32_FATFS_PARTITION "storage"
#define ESP32_FS_MAX_FILES 4

//#define ESP32_NVS_MAX_KEY_LEN 16 /* Including NULL terminator */
//#define ESP32_FW_INFO_STRSIZE 64
//static char open_esp32_nvs_key[ESP32_NVS_MAX_KEY_LEN];
//static char read_buffer[1024];

static esp_ota_handle_t open_firmware_bin_handle = 0;
/* ESP32 wear levelling library instance */
wl_handle_t flash_wl_handle = WL_INVALID_HANDLE;

/* These functions are implemented in the application layer to report status */
extern void esp32_xibsp_notify_update_started( const char* filename, size_t file_size );
extern void esp32_xibsp_notify_chunk_written( size_t chunk_size, size_t offset );
extern void esp32_xibsp_notify_update_applied();

static inline int8_t xi_esp32_flash_init( void )
{
    const esp_vfs_fat_mount_config_t mount_config = {.max_files = ESP32_FS_MAX_FILES,
                                                     .format_if_mount_failed = true};

    const esp_err_t retval = esp_vfs_fat_spiflash_mount(
        ESP32_FS_BASE_PATH, ESP32_FATFS_PARTITION, &mount_config, &flash_wl_handle );
    if ( retval != ESP_OK )
    {
        xi_bsp_debug_format( "Failed to mount FATFS [0x%x]", retval );
        return -1;
    }
    return 0;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_open( const char* const resource_name,
                   const size_t size,
                   const xi_bsp_io_fs_open_flags_t open_flags,
                   xi_bsp_io_fs_resource_handle_t* resource_handle_out )
{
    ( void )open_flags;
    esp_err_t esp_retv = ESP_OK;
    printf( "Opening of file [%s] requested", resource_name );
    if ( NULL == resource_name || NULL == resource_handle_out )
    {
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );

        if ( NULL == next_partition )
        {
            xi_bsp_debug_logger( "Failed to get next OTA update partition" );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        esp_retv = esp_ota_begin( next_partition, size,
                                  ( esp_ota_handle_t* )resource_handle_out );

        if ( ESP_OK != esp_retv )
        {
            xi_bsp_debug_format( "esp_ota_begin() failed with error %d", esp_retv );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }

        esp32_xibsp_notify_update_started( resource_name, size );
        open_firmware_bin_handle = *resource_handle_out;
    }
    else
    {
        /* Initialize flash filesystem if necessary */
        if ( WL_INVALID_HANDLE == flash_wl_handle )
        {
            if ( 0 > xi_esp32_flash_init() )
            {
                printf( "\n >>FLASH_INIT FS ERROR<< \n" );
                return XI_BSP_IO_FS_OPEN_ERROR;
            }
            else
            {
                printf( "\n $$FLASH_INIT OK$$ \n" );
            }
        }

        /* Create path to the mounted partition and fopen the file */
        char filepath[64] = "";
        snprintf( filepath, 64, "%s/%s", ESP32_FS_BASE_PATH, resource_name );
#if 1 // posix fs

        /* append not supported */
        if ( XI_BSP_IO_FS_OPEN_APPEND == ( open_flags & XI_BSP_IO_FS_OPEN_APPEND ) )
        {
            printf( "\n >>OPEN_APPEND FS ERROR<< \n" );
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        printf("\n[O 1]");
        xi_bsp_io_fs_posix_file_handle_container_t* new_entry = NULL;
        xi_bsp_io_fs_state_t ret                              = XI_BSP_IO_FS_STATE_OK;

        printf( "\n>>Opening filepath [%s] in mode [%s]", filepath, ( open_flags & XI_BSP_IO_FS_OPEN_READ ) ? "rb" : "wb" );
        FILE* fp =
            fopen( filepath, ( open_flags & XI_BSP_IO_FS_OPEN_READ ) ? "rb" : "wb" );

        printf("\n[O 2]");
        /* if error on fopen check the errno value */
        XI_BSP_IO_FS_CHECK_CND(
            NULL == fp, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        printf("\n[O 3]");
        /* allocate memory for the files database element */
        new_entry =
            xi_bsp_mem_alloc( sizeof( xi_bsp_io_fs_posix_file_handle_container_t ) );
        memset( new_entry, 0, sizeof( xi_bsp_io_fs_posix_file_handle_container_t ) );

        printf("\n[O 4]");
        /* store the posix file pointer */
        new_entry->posix_fp = fp;

        printf("\n[O 5]");
        /* add the entry to the database */
        XI_LIST_PUSH_BACK( xi_bsp_io_fs_posix_file_handle_container_t,
                           xi_bsp_io_fs_posix_files_container, new_entry );

        printf("\n[O 6]");
        /* make sure that the size is as expected. */
        assert( sizeof( fp ) == sizeof( xi_bsp_io_fs_resource_handle_t ) );

        printf("\n[O 7]");
        /* return fp as a resource handle */
        *resource_handle_out = ( xi_bsp_io_fs_resource_handle_t )fp;

        return ret;

    err_handling:
        printf( "\n >>OPEN_ERRHANDLING FS ERROR. Ret: [%d]<< \n", ret );
        if ( NULL != fp )
        {
            fclose( fp );
        }
        xi_bsp_mem_free( new_entry );
        *resource_handle_out = xi_bsp_io_fs_init_resource_handle();
        return ret;
#else
        resource_handle_out = ( xi_bsp_io_fs_resource_handle_t* )fopen( filepath, "wb" );
        if ( NULL == resource_handle_out )
        {
            xi_bsp_debug_logger( "Failed to fopen file [%s]", filepath );
            return XI_BSP_IO_FS_OPEN_ERROR;
        }
#endif
    }

    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_write( const xi_bsp_io_fs_resource_handle_t resource_handle,
                    const uint8_t* const buffer,
                    const size_t buffer_size,
                    const size_t offset,
                    size_t* const bytes_written )
{
    ( void )offset;
    esp_err_t retv = ESP_OK;
    printf( "Write of file [] requested" );
    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        retv = esp_ota_write( resource_handle, buffer, buffer_size );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_write() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        *bytes_written = buffer_size;
        esp32_xibsp_notify_chunk_written( buffer_size, offset );
    }
    else
    {
#if 1 // posix fs
        if ( NULL == buffer || 0 == buffer_size ||
             XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            printf( "\n >>WRITE FS ERROR<< \n" );
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
        FILE* fp                 = ( FILE* )resource_handle;

        xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* let's set the offset */
        const int fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        *bytes_written = fwrite( buffer, ( size_t )1, buffer_size, fp );

        /* if error on fwrite check errno */
        XI_BSP_IO_FS_CHECK_CND(
            buffer_size != *bytes_written,
            xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( ferror( fp ) ), ret );

    err_handling:
        printf( "\n >>WRITE_H FS ERROR<< \n" );

        return ret;
#else
        const size_t written =
            fwrite( buffer, 1, buffer_size, ( FILE* )&resource_handle );
        if ( buffer_size > written )
        {
            return XI_BSP_IO_FS_WRITE_ERROR;
        }
#endif
    }
    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_read( const xi_bsp_io_fs_resource_handle_t resource_handle,
                   const size_t offset,
                   const uint8_t** buffer,
                   size_t* const buffer_size )
{
    printf( "Read file [] requested" );
    if ( NULL == buffer || NULL != *buffer || NULL == buffer_size ||
         XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
    {
        printf( "\n >>READ FS ERROR<< \n" );
        return XI_BSP_IO_FS_INVALID_PARAMETER;
    }

    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        xi_bsp_debug_logger( "Error: No FS read of FW files in ESP32's OTA API" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
#if 1
        xi_bsp_io_fs_state_t ret = XI_BSP_IO_FS_STATE_OK;
        FILE* fp                 = ( FILE* )resource_handle;
        int fop_ret              = 0;

        xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* make an allocation for memory block */
        if ( NULL == elem->memory_buffer )
        {
            elem->memory_buffer =
                ( uint8_t* )xi_bsp_mem_alloc( xi_bsp_io_fs_buffer_size );
            memset( elem->memory_buffer, 0, xi_bsp_io_fs_buffer_size );
        }

        /* let's set an offset */
        fop_ret = fseek( fp, offset, SEEK_SET );

        /* if error on fseek check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret != 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* use the fread to read the file chunk */
        fop_ret = fread( elem->memory_buffer, ( size_t )1, xi_bsp_io_fs_buffer_size, fp );

        /* if error on fread check errno */
        XI_BSP_IO_FS_CHECK_CND(
            fop_ret == 0, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        /* return buffer, buffer_size */
        *buffer      = elem->memory_buffer;
        *buffer_size = fop_ret;

        return XI_BSP_IO_FS_STATE_OK;

    err_handling:

        printf( "\n >>READ_H FS ERROR<< \n" );
        *buffer      = NULL;
        *buffer_size = 0;
        xi_bsp_mem_free( elem->memory_buffer );

        return ret;
#else
        //const char* read_buffer[1024];
        fseek( ( FILE* )&resource_handle, offset, SEEK_SET );
        *buffer_size =
            fread( read_buffer, ( size_t )1, 1024, ( FILE* )&resource_handle );
        if ( ferror( ( FILE* )&resource_handle ) )
        {
            xi_bsp_debug_logger( "Failed to read file" );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }
        clearerr( ( FILE* )&resource_handle );
        *buffer = ( const uint8_t* )read_buffer;
#endif
    }
    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t
xi_bsp_io_fs_close( const xi_bsp_io_fs_resource_handle_t resource_handle )
{
    printf( "Close file [] requested" );
    if ( ( esp_ota_handle_t )resource_handle == open_firmware_bin_handle )
    {
        const esp_partition_t* next_partition = esp_ota_get_next_update_partition( NULL );
        esp_err_t retv                        = ESP_OK;

        xi_bsp_debug_format( "Finished FW download to partition [%p]", next_partition );

        retv = esp_ota_end( resource_handle );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_end() failed with error %d", retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        retv = esp_ota_set_boot_partition( next_partition );
        if ( ESP_OK != retv )
        {
            xi_bsp_debug_format( "esp_ota_set_boot_partition() failed with error %d",
                                 retv );
            return XI_BSP_IO_FS_WRITE_ERROR;
        }

        esp32_xibsp_notify_update_applied();
    }
    else
    {
#if 1 // posix fs
        if ( XI_BSP_IO_FS_INVALID_RESOURCE_HANDLE == resource_handle )
        {
            return XI_BSP_IO_FS_INVALID_PARAMETER;
        }

        xi_bsp_io_fs_state_t ret                         = XI_BSP_IO_FS_STATE_OK;
        FILE* fp                                         = ( FILE* )resource_handle;
        xi_bsp_io_fs_posix_file_handle_container_t* elem = NULL;
        int fop_ret                                      = 0;

        XI_LIST_FIND( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container,
                      xi_bsp_io_fs_posix_file_list_cnd, fp, elem );

        /* if element not on the list return resource not available error */
        XI_BSP_IO_FS_CHECK_CND( NULL == elem, XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE, ret );

        /* remove element from the list */
        XI_LIST_DROP( xi_bsp_io_fs_posix_file_handle_container_t,
                      xi_bsp_io_fs_posix_files_container, elem );

        fop_ret = fclose( fp );

        /* if error on fclose check errno */
        XI_BSP_IO_FS_CHECK_CND(
            0 != fop_ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

        xi_bsp_mem_free( elem->memory_buffer );
        xi_bsp_mem_free( elem );

        return XI_BSP_IO_FS_STATE_OK;

    err_handling:

        printf( "\n >>CLOSE FS ERROR<< \n" );
        if ( NULL != elem )
        {
            xi_bsp_mem_free( elem->memory_buffer );
            xi_bsp_mem_free( elem );
        }
        return ret;
#else
        fclose( resource_handle );
#endif
    }
    return XI_BSP_IO_FS_STATE_OK;
}

xi_bsp_io_fs_state_t xi_bsp_io_fs_remove( const char* const resource_name )
{
    ( void )resource_name;
    printf( "Remove file [%s] requested", resource_name );
    if ( !xi_bsp_fwu_is_this_firmware( resource_name ) )
    {
        xi_bsp_debug_logger( "FS Remove not implemented for FW images" );
        return XI_BSP_IO_FS_NOT_IMPLEMENTED;
    }
    else
    {
        char filepath[64] = "";
        snprintf( filepath, 64, "%s/%s", ESP32_FS_BASE_PATH, resource_name );
        int ret = remove( filepath );

        XI_BSP_IO_FS_CHECK_CND(
            0 != ret, xi_bsp_io_fs_posix_errno_2_xi_bsp_io_fs_state( errno ), ret );

    err_handling:

        printf( "\n >>RM FS ERROR<< \n" );
        return ret;
    }
    return XI_BSP_IO_FS_STATE_OK;
}
