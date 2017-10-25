/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

/*
 * These functions are declared as extern in the ESP32's BSP:
 *     xively-client-c/src/bsp/platform/esp32/xi_bsp_io_fs_esp32.c
 * It's a custom solution to showcase how the application can be made aware
 * of the download status.
 * Useful to act on peripherals, display download progress, etc.
 * If you decide to remove these functions for your app, you'll also have to
 * remove their calls and declarations in xi_bsp_io_fs_esp32.c
 */

#include <stdio.h>

static size_t download_file_size = 0;
static int download_progress = 0;

void esp32_xibsp_notify_update_started( const char* filename, size_t file_size )
{
    printf( "\nDownload of file [%s] started. Size: [%d]", filename, file_size );
    download_file_size = file_size;
}

void esp32_xibsp_notify_chunk_written( size_t chunk_size, size_t offset )
{
    if ( download_file_size <= 0 )
    {
        return;
    }
    download_progress = ( ( chunk_size + offset ) * 100 ) / download_file_size;
    printf( "\n[%d%%] Firmware chunk of size [%d] written at offset [%d]",
            download_progress, chunk_size, offset );
}

void esp32_xibsp_notify_update_applied()
{
    printf( "\nFirmware update successfully completed - Proceeding with reboot" );
}
