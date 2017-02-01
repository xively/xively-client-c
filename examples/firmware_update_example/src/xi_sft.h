#ifndef __XI_SFT_H__
#define __XI_SFT_H__

#include <stdint.h>
#include <xively.h>

/**
 * All of the different kinds of CBOR parser errors
 */
typedef enum xi_cbor_parser_errors {
    /* This set of errors relate to strange formatting errors */
    XI_CBOR_PARSER_OK = 0,
    XI_CBOR_MISSING_NEXT_ELEMENT,
    XI_CBOR_UNKNOWN_PACKET,
    XI_CBOR_MISSING_MAP,

    /* This set of errors relate to required elements for all message types */
    XI_CBOR_MISSING_MSGTYPE = 100,
    XI_CBOR_NOMATCH_MSGTYPE,
    XI_CBOR_MISSING_MSGTYPE_VALUE,
    XI_CBOR_MISSING_MSGVER,
    XI_CBOR_NOMATCH_MSGVER,
    XI_CBOR_MISSING_MSGVER_VALUE,
    XI_CBOR_INCOMPLETE_PACKET,

    /* This set of errors relate to required elements for the FILE_INFO packet */
    XI_CBOR_MISSING_FILE_INFO_LIST = 200,
    XI_CBOR_MISSING_FILE_INFO_ARRAY,
    XI_CBOR_MISSING_FILE_INFO_MAP,
    XI_CBOR_MISSING_FILE_INFO_NAME_KEY,
    XI_CBOR_MISSING_FILE_INFO_NAME_VALUE,
    XI_CBOR_MISSING_FILE_INFO_REVISION_KEY,
    XI_CBOR_MISSING_FILE_INFO_REVISION_VALUE,

    /* This set of errors relate to required elements for the FILE_UPDATE_AVAILABLE packet
       */
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_LIST = 300,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_ARRAY,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_MAP,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_NAME_KEY,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_NAME_VALUE,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_REVISION_KEY,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_REVISION_VALUE,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_IMAGESIZE_KEY,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_IMAGESIZE_VALUE,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_FINGERPRINT_KEY,
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_FINGERPRINT_VALUE,
    /* File Operation is optional, so no MISSING error */
    XI_CBOR_MISSING_FILE_UPDATE_AVAILABLE_FILEOPERATION_VALUE,

} xi_cbor_parser_errors;

typedef enum xi_packet_types {
    XI_FILE_INFO = 0,
    XI_FILE_UPDATE_AVAILABLE,
    XI_FILE_GET_CHUNK,
    XI_FILE_CHUNK,
    XI_FILE_STATUS
} xi_packet_types;

int xi_sft_init( xi_context_handle_t in_context_handle,
                 const char* const account_id,
                 const char* const device_id );
void xi_publish_file_info( xi_context_handle_t in_context_handle );

/* functions for platform specific behaviour */

/**
 * Reboot the MCU by requesting hibernate for a short duration
 */
extern void reboot();

/**
 * Opens the file of a given name for writing
 *
 * @param fileName
 * @param fileLength
 * @param fileHandle - return argument passes the pointer to the platform specific file
 * handle
 * @returns 0 if no errors <0 in case of error the negative number is an error code
 */
extern int openFileForWrite( const char* fileName,
                                       size_t fileLength,
                                       void** fileHandle );

/**
 * Closes previously opened file
 *
 * @param fileHandle
 * @return 0 in case of success <0 in case of an error
 */
extern int closeFile( void** fileHandle );

/**
 * Writes one chunk of data to the previously opened file using fileHandle.
 *
 * @param fileHandle - handle of the previously opened file
 * @param chunkOffset - offset in bytes from the beginning of the file points to where the bytes in opened file should be written
 * @param bytes - bytes to write in the file
 * @param bytesLength - number of bytes to write
 */
extern int writeChunk( void* fileHandle,
                                 size_t chunkOffset,
                                 const unsigned char* const bytes,
                                 size_t bytesLength );

/**
 * @enum xi_commit_firmware_flags_e
 * 
 * XI_FIRMWARE_COMMITED - means that the firmware was tested and can be used
 * XI_FIRMWARE_NOT_COMMITED - firmare should be rejected
 */
typedef enum xi_commit_firmware_flags_e {
    XI_FIRMWARE_COMMITED     = 1,
    XI_FIRMWARE_NOT_COMMITED = 2
} xi_commit_firmware_flags_t;

/**
 * If the firmware was sucesfully updated the MCU will be able to store it and use it as a main firmware
 */
extern int32_t commitFirmware( int32_t commitFlags );

/**
 * Set the MCU to use new firmware in order to test if it work and can be commited
 *
 * @brief May be followed by Reboot
 */
extern int32_t testFirmware( void );

#endif /* __XI_SFT_H__ */
