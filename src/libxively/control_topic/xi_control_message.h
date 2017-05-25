/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_MESSAGE_H__
#define __XI_CONTROL_MESSAGE_H__

#include <stdint.h>

typedef enum xi_control_message_type_e {
    XI_CONTROL_MESSAGE_DB_FILE_INFO = 0, /* DB = message goes from Device to Broker */
    XI_CONTROL_MESSAGE_BD_FILE_UPDATE_AVAILABLE, /* BD = Broker to Device */
    XI_CONTROL_MESSAGE_DB_FILE_GET_CHUNK,
    XI_CONTROL_MESSAGE_BD_FILE_CHUNK,
    XI_CONTROL_MESSAGE_DB_FILE_STATUS
} xi_control_message_type_t;

typedef struct xi_control_message_file_desc_s
{
    const char* name;
    const char* revision;
} xi_control_message_file_desc_t;

typedef struct xi_control_message_file_desc_ext_s
{
    char* name;
    char* revision;
    uint8_t file_operation;
    uint32_t size_in_bytes;
    char* fingerprint;
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

        const uint16_t list_len;
        const xi_control_message_file_desc_t* list;

    } file_info;

    struct
    {
        struct xi_control_message_common_s common;

        uint16_t list_len;
        xi_control_message_file_desc_ext_t* list;

    } file_update_available;

    struct file_get_chunk_s
    {
        struct xi_control_message_common_s common;

        const char* name;
        const char* revision;

        const uint32_t offset;
        const uint32_t length;

    } file_get_chunk;

    struct
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

        const char* name;
        const char* revision;

        const char* status_message;
        uint8_t status_code;

    } file_status;

} xi_control_message_t;

void xi_control_message_free( xi_control_message_t** control_message );

#endif /* __XI_CONTROL_MESSAGE_H__ */
