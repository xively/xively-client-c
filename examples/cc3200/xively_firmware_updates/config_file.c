/******************************************************************************
 *                                                                            *
 * config_file.c                                                              *
 *                                                                            *
 ******************************************************************************/

/*
 *   Standard includes
 */
#include <stdlib.h>
#include <ctype.h>

/*
 *   TI includes
 */
#include "simplelink.h"

/*
 *   Local includes
 */
#include "config_file.h"


/******************************************************************************
 *                                                                            *
 * fio_fd                                                                     *
 *                                                                            *
 * Combines SimpleLink file descriptor with an offset to support streaming    *
 * reads/writes.                                                              *
 *                                                                            *
 ******************************************************************************/

typedef enum {
    FIO_SUCCESS     = 0,
    FIO_ALLOC_ERROR = 1,
} fio_status_e;


typedef struct
{
    int32_t sl_fh;
    uint32_t offset;
} fio_fd;


/******************************************************************************
 *                                                                            *
 * fio_open ()                                                                *
 *                                                                            *
 ******************************************************************************/

static int fio_open( const unsigned char* fname, fio_fd** fd_pp, unsigned long* ulToken )
{
    fio_fd* fd;
    int rval = -1;


    if ( ( fd = malloc( sizeof( fio_fd ) ) ) == NULL )
    {
        rval = FIO_ALLOC_ERROR;
    }
    else
    {
        rval = sl_FsOpen( ( const _u8* )fname, FS_MODE_OPEN_READ, NULL,
                          ( _i32* )&fd->sl_fh );
    }

    if ( rval == FIO_SUCCESS )
    {
        fd->offset = 0;
        *fd_pp     = fd;
    }
    else
    {
        if ( fd )
        {
            free( fd );
        }
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 * fio_close ()                                                               *
 *                                                                            *
 ******************************************************************************/

static void fio_close( fio_fd* fd )
{
    if ( fd )
    {
        sl_FsClose( fd->sl_fh, NULL, NULL, 0 );
        free( fd );
    }
}

/******************************************************************************
 *                                                                            *
 * fio_read_char ()                                                           *
 *                                                                            *
 ******************************************************************************/

static char fio_read_char( fio_fd* fd )
{
    char c;
    if ( sl_FsRead( fd->sl_fh, fd->offset, ( _u8* )&c, 1 ) == 1 )
    {
        fd->offset++;
    }
    else
    {
        c = 0;
    }

    return c;
}


/******************************************************************************
 *                                                                            *
 * free_ce ()                                                                 *
 *                                                                            *
 ******************************************************************************/

static void free_ce( config_entry_t* ce )
{
    if ( ce )
    {
        if ( ce->key )
        {
            free( ce->key );
        }

        if ( ce->value )
        {
            free( ce->value );
        }
    }
}


/******************************************************************************
 *                                                                            *
 * create_ce ()                                                               *
 *                                                                            *
 ******************************************************************************/

static config_entry_t* create_ce( char* key, char* value )
{
    config_entry_t* ce;

    if ( ( ce = malloc( sizeof( config_entry_t ) ) ) )
    {
        if ( ( ce->key = strdup( key ) ) && ( ce->value = strdup( value ) ) )
        {
            ce->next = NULL;
        }
        else
        {
            free_ce( ce );
            ce = NULL;
        }
    }

    return ce;
}

/******************************************************************************
 *                                                                            *
 * check_key                                                                  *
 *                                                                            *
 ******************************************************************************/

static int check_key( const char* key )
{
    int rval = CONFIG_SUCCESS;

    if ( !key || !*key )
    {
        rval = CONFIG_BAD_KEY;
    }
    else
    {
        while ( *key )
        {
            if ( ( ( *key >= 'A' ) && ( *key <= 'Z' ) ) ||
                 ( ( *key >= 'a' ) && ( *key <= 'z' ) ) ||
                 ( ( *key >= '0' ) && ( *key <= '9' ) ) || ( *key == '_' ) )
            {
                key++;
            }
            else
            {
                rval = CONFIG_BAD_KEY;
                break;
            }
        }
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 * check_value                                                                *
 *                                                                            *
 ******************************************************************************/

static int check_value( const char* value )
{
    int rval = CONFIG_SUCCESS;

    if ( !value )
    {
        rval = CONFIG_BAD_VALUE;
    }
    else
    {
        while ( *value )
        {
            if ( ( *value >= ' ' ) && ( *value <= '~' ) )
            {
                value++;
            }
            else
            {
                rval = CONFIG_BAD_VALUE;
                break;
            }
        }
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 * read_line_key_value ()                                                     *
 *                                                                            *
 * Read a key/value from single line in file. The returned key and value must *
 * be freed.                                                                  *
 *                                                                            *
 ******************************************************************************/

#define STRING_LEN_INIT 32


typedef enum {
    KV_PRE_KEY = 0,
    KV_IN_KEY,
    KV_PRE_COLON,
    KV_PRE_VALUE,
    KV_IN_VALUE,
    KV_POST_VALUE,
    KV_EOL
} kv_state_e;


static int read_line_key_value( fio_fd* fd, char** key_pp, char** value_pp )
{
    kv_state_e state     = KV_PRE_KEY;
    kv_state_e eol_state = KV_PRE_KEY;
    int buf_len          = 0;
    int str_len          = 0;
    char* str_buf        = NULL;
    int escape           = 0;
    char* key            = NULL;
    char* value          = NULL;
    int rval             = CONFIG_SUCCESS;
    char c;

    while ( ( c = fio_read_char( fd ) ) && ( c != '\n' ) )
    {
        switch ( state )
        {
            case KV_PRE_KEY:
            case KV_PRE_COLON:
            case KV_PRE_VALUE:
            case KV_POST_VALUE:
            {
                if ( isspace( c ) ) // Skip Space
                {
                    break;
                }

                if ( c == '#' ) // Comment
                {
                    eol_state = state;
                    state     = KV_EOL;
                    break;
                }

                if ( ( c == ':' ) && ( state == KV_PRE_COLON ) )
                {
                    state++;
                    break;
                }

                if ( ( c == '"' ) &&
                     ( ( state == KV_PRE_KEY ) || ( state == KV_PRE_VALUE ) ) )
                {
                    state++;
                    break;
                }

                rval      = CONFIG_BAD_CHAR;
                eol_state = state;
                state     = KV_EOL;
                break;
            }


            case KV_IN_KEY:
            case KV_IN_VALUE:
            {
                if ( str_len == buf_len )
                {
                    buf_len += STRING_LEN_INIT;

                    if ( ( str_buf = realloc( str_buf, buf_len + 1 ) ) == NULL )
                    {
                        rval      = CONFIG_ALLOC_ERROR;
                        eol_state = state;
                        state     = KV_EOL;
                        break;
                    }
                }

                if ( escape )
                {
                    if ( ( c == '"' ) || ( c == '\\' ) )
                    {
                        str_buf[str_len++] = c;
                    }
                    else
                    {
                        rval      = CONFIG_BAD_ESCAPE;
                        eol_state = state;
                        state     = KV_EOL;
                        break;
                    }

                    escape = 0;
                }

                if ( c == '"' )
                {
                    str_buf[str_len] = 0;

                    if ( state == KV_IN_KEY )
                    {
                        key = str_buf;
                    }
                    else
                    {
                        value     = str_buf;
                        eol_state = KV_POST_VALUE;
                    }

                    buf_len = 0;
                    str_len = 0;
                    str_buf = NULL;
                    state++;
                }

                else if ( c == '\\' )
                {
                    escape = 1;
                }
                else
                {
                    str_buf[str_len++] = c;
                }

                break;
            }

            case KV_EOL:
            default:
                break;

        } /* switch */

    } /* while */

    if ( rval == CONFIG_SUCCESS )
    {
        switch ( eol_state )
        {
            case KV_PRE_KEY:
            {
                if ( c )
                {
                    rval = CONFIG_EMPTY;
                }
                else
                {
                    rval = CONFIG_EOF;
                }
                break;
            }

            case KV_IN_KEY:
            {
                rval = CONFIG_BAD_KEY;
                break;
            }

            case KV_PRE_VALUE:
            case KV_IN_VALUE:
            {
                rval = CONFIG_BAD_VALUE;
                break;
            }

            case KV_PRE_COLON:
            {
                rval = CONFIG_NO_COLON;
                break;
            }
        }
    }

    if ( rval == CONFIG_SUCCESS )
    {
        *key_pp   = key;
        *value_pp = value;
    }
    else
    {
        if ( key )
        {
            free( key );
        }

        if ( value )
        {
            free( value );
        }
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 * read_config_file ()                                                        *
 *                                                                            *
 ******************************************************************************/

int read_config_file( const unsigned char* fname, config_entry_t** ce_pp )
{
    fio_fd* fd;
    config_entry_t* ce;
    char* key;
    char* value;
    int rval;

    ce = NULL;
    fd = NULL;

    do
    {
        unsigned long ulToken = 0;
        rval                  = fio_open( fname, &fd, &ulToken );
        if ( rval < 0 )
        {
            break;
        }

        while ( 1 )
        {
            rval = read_line_key_value( fd, &key, &value );

            if ( rval == CONFIG_SUCCESS )
            {
                rval = set_config_value( &ce, key, value );

                if ( rval )
                {
                    break;
                }
            }
            else if ( rval == CONFIG_EMPTY ) // Blank Line
            {
                continue;
            }
            else if ( rval == CONFIG_EOF )
            {
                rval = CONFIG_SUCCESS;
                break;
            }
            else
            {
                break;
            }
        }

        if ( rval )
        {
            free_config_entries( ce );
            break;
        }

        if ( ce == NULL )
        {
            rval = CONFIG_EMPTY;
            break;
        }

        *ce_pp = ce;

    } while ( 0 );

    if ( fd )
    {
        fio_close( fd );
    }

    return rval;
}

/******************************************************************************
 *                                                                            *
 * set_config_value ()                                                        *
 *                                                                            *
 ******************************************************************************/

int set_config_value( config_entry_t** ce_pp, const char* key, char* value )
{
    config_entry_t* ce;
    config_entry_t** end_pp;
    int rval;
    char* dup_value;

    do
    {
        if ( ( rval = check_key( key ) ) )
        {
            break;
        }

        if ( value )
        {
            if ( ( rval = check_value( value ) ) )
            {
                break;
            }
        }

        ce     = *ce_pp;
        end_pp = ce_pp;
        rval   = CONFIG_PENDING;

        while ( ce )
        {
            if ( strcmp( key, ce->key ) == 0 ) // KEY exists
            {
                if ( value == NULL ) // Delete entry
                {
                    *end_pp = ce->next;
                    free_ce( ce );
                    rval = CONFIG_SUCCESS;
                }
                else
                {
                    if ( strcmp( value, ce->value ) == 0 ) // VALUE unchanged
                    {
                        rval = CONFIG_SUCCESS;
                    }
                    else // New VALUE
                    {
                        if ( ( dup_value = strdup( value ) ) )
                        {
                            free( ce->value ); // Free old VALUE
                            ce->value = dup_value;
                            rval      = CONFIG_SUCCESS;
                        }
                        else
                        {
                            rval = CONFIG_ALLOC_ERROR;
                        }
                    }
                }

                break;
            }

            end_pp = &ce->next;
            ce     = ce->next;
        }

        if ( rval == CONFIG_PENDING ) // New KEY and VALUE
        {
            if ( ( ce = create_ce( ( char* )key, value ) ) )
            {
                rval    = CONFIG_SUCCESS;
                *end_pp = ce;
            }
            else
            {
                rval = CONFIG_ALLOC_ERROR;
            }
        }

    } while ( 0 );

    return rval;
}


/******************************************************************************
 *                                                                            *
 * get_config_value ()                                                        *
 *                                                                            *
 ******************************************************************************/

char* get_config_value( config_entry_t* ce, const char* key )
{
    char* rval = NULL;

    while ( ce )
    {
        if ( strcmp( key, ce->key ) == 0 )
        {
            rval = ce->value;
            break;
        }
        ce = ce->next;
    }

    return rval;
}


/******************************************************************************
 *                                                                            *
 * free_config_entries ()                                                     *
 *                                                                            *
 ******************************************************************************/

void free_config_entries( config_entry_t* ce )
{
    config_entry_t* next;

    while ( ce )
    {
        next = ce->next;
        free_ce( ce );
        ce = next;
    }
}
