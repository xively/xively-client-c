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
                 char* account_id,
                 char* device_id );
void xi_publish_file_info( xi_context_handle_t in_context_handle );
