/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <stdio.h>
#include <stdint.h>

#include <xi_coroutine.h>
#include <xi_macros.h>

/* CRC specific code */
/* the table that containes crc precalculated values */
uint32_t crc_table[256];

/* function that initialises the crc table */
void initialise_crc_table( uint32_t ( *table )[256] )
{
    uint32_t c;
    int32_t n, k;

    for ( n = 0; n < 256; ++n )
    {
        c = ( uint32_t )n;
        for ( k = 0; k < 8; ++k )
        {
            if ( c & 1 )
                c = 0xedb88320L ^ ( c >> 1 );
            else
                c = c >> 1;
        }
        ( *table )[n] = c;
    }
}

/**
 * @name update_crc
 * @brief updates calculated checksum
 */
uint32_t update_crc( const uint32_t crc, const uint8_t* buf, const size_t buf_len )
{
    uint32_t c = crc;
    size_t i   = 0;

    for ( ; i < buf_len; ++i )
    {
        c = crc_table[( c ^ buf[i] ) & 0xFF] ^ ( c << 8 );
    }

    return c;
}
/* end of CRC specific code */

/* CRC coroutine specific code */
typedef struct crc_coroutine_s
{
    uint16_t cr_state;    /* coroutine will save here it's own state */
    uint32_t crc;         /* keeps the calculated crc */
    size_t current_pos;   /* how many bytes has already been processed */
    const uint8_t* buf;   /* buffer with data to process */
    const size_t buf_len; /* length of the buffer with data */
} crc_coroutine_t;

typedef enum crc_coroutine_result {
    CRC_COROUTINE_FINISHED = 0,
    CRC_COROUTINE_WANTS_TO_CONTINUE,
    CRC_COROUTINE_ERROR
} crc_coroutine_result_t;

/**
 * It's kind of a SIMD approach to processing we using same instruction but for
 * multiple data.
 */
crc_coroutine_result_t process_crc( crc_coroutine_t* cr_data )
{
    /**
     * Because we are going to invoke this function many times for each
     * coroutine we have to re-calcualte values for each entry in separation.
     *
     * Golden rule for this type of coroutine is to keep the values outside.
     */
    const size_t calculations_per_run =
        32; /* how much of the computation done in one iteration */
    const size_t len_left =
        cr_data->buf_len - cr_data->current_pos; /* calculate how much of the data
                                                is still not processed */
    const size_t to_go =
        XI_CLAMP( len_left, 0, calculations_per_run ); /* calculate how much of the data
                                                          will be processed in this
                                                          execution */

    /* The biggest power of coroutine is that they allow us to re-structurise
     * state dependent code into a "natural" sequence of operations.
     *
     * In example cr_data->crc = 0xFFFFFFFFL line should only happen only once
     * at the beginning. Usually programmers check for a condition or a state in
     * order to make sure that this is the first invocation. Using coroutines we
     * can put the operations in natural order and they shall be invoked until
     * corotuine yield keyword. After re-entry coroutine will make the function
     * to continue it's execution from where it has yielded.
     */
    XI_CR_START( cr_data->cr_state );

    /* initialisation of the return value */
    cr_data->crc = 0xFFFFFFFFL;

    for ( ;; )
    {
        /* update the crc with the next portion of bytes */
        cr_data->crc =
            update_crc( cr_data->crc, cr_data->buf + cr_data->current_pos, to_go );
        cr_data->current_pos += to_go;

        /* we are going to keep yielding until all the data is processed */
        XI_CR_YIELD_UNTIL( cr_data->cr_state, cr_data->current_pos < cr_data->buf_len,
                           CRC_COROUTINE_WANTS_TO_CONTINUE );

        /* here is the place where coroutine function continues whenever the
         * condition from XI_CR_YIELD_UNTIL has not been met */
        break;
    }

    /* this is std CRC operation refer to the CRC implementation if you are
     * interested in details */
    cr_data->crc = cr_data->crc ^ 0xFFFFFFFFL;

    /* exiting the coroutine is like yielding but the coroutine state variable
     * is set to a special value which means that the coroutine is now done */
    XI_CR_EXIT( cr_data->cr_state, CRC_COROUTINE_FINISHED );

    XI_CR_END();

    return CRC_COROUTINE_ERROR;
}

/* end of CRC coroutine specific code */

int main( const int argc, const char* argv[] )
{
    XI_UNUSED( argc );
    XI_UNUSED( argv );

    initialise_crc_table( &crc_table );

    uint8_t buffer1[1024] = {'a'};
    uint8_t buffer2[512]  = {'b'};

    crc_coroutine_t task1 = {.cr_state    = 0,
                             .crc         = 0,
                             .current_pos = 0,
                             .buf         = buffer1,
                             .buf_len     = sizeof( buffer1 )};
    crc_coroutine_result_t task1_result = CRC_COROUTINE_WANTS_TO_CONTINUE;

    crc_coroutine_t task2 = {.cr_state    = 0,
                             .crc         = 0,
                             .current_pos = 0,
                             .buf         = buffer2,
                             .buf_len     = sizeof( buffer2 )};
    crc_coroutine_result_t task2_result = CRC_COROUTINE_WANTS_TO_CONTINUE;

    do
    {
        if ( task1_result == CRC_COROUTINE_WANTS_TO_CONTINUE )
        {
            task1_result = process_crc( &task1 );
        }

        if ( task2_result == CRC_COROUTINE_WANTS_TO_CONTINUE )
        {
            task2_result = process_crc( &task2 );
        }

        /**
         * Because we are only deals with a fraction of caluclation's per one
         * loop we can do some other things in so called meantime. This is very
         * simplified implementation of cooperative multitasking where we are
         * splitting the time of CPU between multiple functions. We are also
         * counting on their good manners so that they will not occupy processor
         * for too long.
         */
        printf( "Summary: task1: %.2f %%, task2: %.2f %%\n",
                ( task1.current_pos / ( float )task1.buf_len ) * 100.0,
                ( task2.current_pos / ( float )task2.buf_len ) * 100.0 );

    } while ( task1_result == CRC_COROUTINE_WANTS_TO_CONTINUE ||
              task2_result == CRC_COROUTINE_WANTS_TO_CONTINUE );

    printf( "All tasks finished processing: \n" );
    printf( "task1 crc: %#x: \n", task1.crc );
    printf( "task2 crc: %#x: \n", task2.crc );

    return 0;
}
