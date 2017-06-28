/******************************************************************************
 *                                                                            *
 * config_file.h                                                              *
 *                                                                            *
 ******************************************************************************/

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

/*
 *   Standard includes
 */
#include <stdint.h>


/******************************************************************************
 *                                                                            *
 * Macros                                                                     *
 *                                                                            *
 ******************************************************************************/

/*
 * Initial buffer length when reading lines from a config file.
 */
#define CONFIG_LINE_DEFAULT_LEN 64

#define CONFIG_FILE_MAXSIZE ( 63 * 1024 )


/******************************************************************************
 *                                                                            *
 * Definitions                                                                *
 *                                                                            *
 ******************************************************************************/

/******************************************************************************
 *                                                                            *
 * config_status_e                                                            *
 *                                                                            *
 * Config specific status values. Zero indicates success.                     *
 *                                                                            *
 * Note: SimpleLink error values are negative, and are returned by "config"   *
 * calls.                                                                     *
 *                                                                            *
 ******************************************************************************/

typedef enum {
    CONFIG_SUCCESS     = 0,
    CONFIG_EOF         = 1,
    CONFIG_EMPTY       = 2,
    CONFIG_PENDING     = 3,
    CONFIG_ALLOC_ERROR = 4,
    CONFIG_BAD_KEY     = 5,
    CONFIG_BAD_VALUE   = 6,
    CONFIG_BAD_CHAR    = 7,
    CONFIG_BAD_ESCAPE  = 8,
    CONFIG_NO_COLON    = 9
} config_status_e;


/******************************************************************************
 *                                                                            *
 * config_entry_t                                                             *
 *                                                                            *
 * Maintains lists of key/value config pairs.                                 *
 *                                                                            *
 ******************************************************************************/

struct config_entry_s
{
    char* key;
    char* value;
    struct config_entry_s* next;
};

typedef struct config_entry_s config_entry_t;


/******************************************************************************
 *                                                                            *
 * Externs                                                                    *
 *                                                                            *
 ******************************************************************************/

int read_config_file( const unsigned char* fname, config_entry_t** ce_pp );
int set_config_value( config_entry_t** ce_pp, const char* key, char* value );
char* get_config_value( config_entry_t* ce_p, const char* key );
void free_config_entries( config_entry_t* ce_p );

#endif
