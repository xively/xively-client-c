/* Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_CONTROL_MESSAGE_H__
#define __XI_CONTROL_MESSAGE_H__

#include <stdint.h>

typedef enum xi_control_message_type_e {
    XI_CONTROL_MESSAGE_CS_FILE_INFO = 0, /* CS = message goes from Client to Service */
    XI_CONTROL_MESSAGE_SC_FILE_UPDATE_AVAILABLE, /* SC = Service to Client */
    XI_CONTROL_MESSAGE_CS_FILE_GET_CHUNK,
    XI_CONTROL_MESSAGE_SC_FILE_CHUNK,
    XI_CONTROL_MESSAGE_CS_FILE_STATUS
} xi_control_message_type_t;

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

        char* name;
        char* revision;

        uint8_t phase;
        uint8_t code;

    } file_status;

} xi_control_message_t;

xi_control_message_t* xi_control_message_create_file_info( const char** filenames,
                                                           const char** revisions,
                                                           uint16_t count );

xi_control_message_t* xi_control_message_create_file_get_chunk( const char* filename,
                                                                const char* revision,
                                                                uint32_t offset,
                                                                uint32_t length );

xi_control_message_t* xi_control_message_create_file_status( const char* filename,
                                                             const char* revision,
                                                             uint8_t phase,
                                                             uint8_t code );

const xi_control_message_file_desc_ext_t*
xi_control_message_file_update_available_get_next_file_desc_ext(
    const struct file_update_available_s* message_fua, const char* filename );

void xi_control_message_free( xi_control_message_t** control_message );

#if 1 // XI_DEBUG_OUTPUT
void xi_debug_control_message_dump( const xi_control_message_t* control_message,
                                    const char* custom_label );
#else
#define xi_debug_control_message_dump( ... )
#endif

#endif /* __XI_CONTROL_MESSAGE_H__ */
