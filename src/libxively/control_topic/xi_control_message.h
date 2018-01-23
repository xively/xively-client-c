/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_MESSAGE_H__
#define __XI_CONTROL_MESSAGE_H__

#include <stdint.h>

typedef enum xi_control_message_type_e {
    XI_CONTROL_MESSAGE_CS__SFT_FILE_INFO =
        0, /* CS = message goes from Client to Service */
    XI_CONTROL_MESSAGE_SC__SFT_FILE_UPDATE_AVAILABLE, /* SC = Service to Client */
    XI_CONTROL_MESSAGE_CS__SFT_FILE_GET_CHUNK,
    XI_CONTROL_MESSAGE_SC__SFT_FILE_CHUNK,
    XI_CONTROL_MESSAGE_CS__SFT_FILE_STATUS,
    XI_CONTROL_MESSAGE_COUNT
} xi_control_message_type_t;

typedef enum xi_control_message__sft_file_status_phase_e {
    /* 0-1 - Reserved, set by Xively Broker itself, ignored when received from clients
       (Unreported, Reported) */
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADING = 2, /* HTTP reports this */
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_DOWNLOADED  = 3,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_PROCESSING  = 4,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_PHASE_FINISHED    = 5
} xi_control_message__sft_file_status_phase_t;

typedef enum xi_control_message__sft_file_status_code_e {
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__ADD_MORE_ERRORS_HERE       = -100,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_UNEXPECTED_FILE_NAME = -8,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_FAILED               = -7,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__URLDL_REJECTED             = -6,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CHECKSUM_MISMATCH     = -5,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_CLOSE                 = -4,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_WRITE                 = -3,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__FILE_OPEN                  = -2,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_ERROR__UNEXPECTED_FILE_CHUNK      = -1,
    XI_CONTROL_MESSAGE__SFT_FILE_STATUS_CODE_SUCCESS                           = 0,

} xi_control_message__sft_file_status_code_t;

typedef struct xi_control_message_file_desc_s
{
    char* name;
    char* revision;
} xi_control_message_file_desc_t;

typedef struct xi_control_message_file_desc_ext_s
{
    char* name;
    char* revision;

    uint8_t file_operation;
    uint32_t size_in_bytes;

    uint8_t* fingerprint;
    uint16_t fingerprint_len;

    char* download_link;
    uint8_t flag_mqtt_download_also_supported;

} xi_control_message_file_desc_ext_t;


typedef union xi_control_message_u {
    struct xi_control_message_common_s
    {
        xi_control_message_type_t msgtype;
        uint32_t msgver;
    } common;


    struct
    {
        struct xi_control_message_common_s common;

        uint16_t list_len;
        xi_control_message_file_desc_t* list;

        uint8_t flag_accept_download_link;
    } file_info;

    struct file_update_available_s
    {
        struct xi_control_message_common_s common;

        uint16_t list_len;
        xi_control_message_file_desc_ext_t* list;

    } file_update_available;

    struct file_get_chunk_s
    {
        struct xi_control_message_common_s common;

        char* name;
        char* revision;

        uint32_t offset;
        uint32_t length;

    } file_get_chunk;

    struct file_chunk_s
    {
        struct xi_control_message_common_s common;

        char* name;
        char* revision;

        uint32_t offset;
        uint32_t length;

        uint8_t status;
        uint8_t* chunk;

    } file_chunk;

    struct
    {
        struct xi_control_message_common_s common;

        char* name;
        char* revision;

        uint8_t phase;
        int8_t code; /* negative means error */

    } file_status;

} xi_control_message_t;

void xi_control_message_free( xi_control_message_t** control_message );

#if XI_DEBUG_OUTPUT
void xi_debug_control_message_dump( const xi_control_message_t* control_message,
                                    const char* debug_custom_label );
#else
#define xi_debug_control_message_dump( ... )
#endif

#endif /* __XI_CONTROL_MESSAGE_H__ */
