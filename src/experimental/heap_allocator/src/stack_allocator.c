/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "stack_allocator.h"

/* let's use 4 bytes as our default alignment */
const size_t extra_space_size = sizeof( stack_allocator_entry_t );

/**
 * memory blocks:
 * There are two types of memory blocks:
 *  - free - those which describes the space that is left in the memory pool
 *  - allocated - describes allocated space in the memory pool
 *
 * Both are kept in the lists. Lists are separated. Lists containing free elements is kept
 * sorted.
 *
 * external_aligned_address - allocated block available for normal usage
 * internal_aligned_address - used internally, limited to store only the stack allocator
 * help data can't be overwritten or changed outside, it shouldn't be accessible outside.
 */

/**
 * @brief calculates the aligned address of the given memory address. It takes into
 * account the size of the alignment.
 */
intptr_t align_address( intptr_t align, intptr_t offset )
{
    return ( offset + align - 1 ) & ~( align - 1 );
}

/**
 * converts the aligned offset to an aligned extra offset
 */
intptr_t get_internal_aligned_addr_from_external_aligned_addr( size_t extra_size,
                                                               intptr_t align,
                                                               intptr_t aligned_addr )
{
    assert( extra_size - 1 <= ( size_t )aligned_addr );
    const intptr_t internal_aligned_addr =
        align_address( align, aligned_addr - extra_size - 1 );

    return internal_aligned_addr;
}

/**
 * converts the aligned extra offset to a base offset
 */
intptr_t
get_external_aligned_addr_from_internal_aligned_addr( size_t extra_size,
                                                      intptr_t align,
                                                      intptr_t internal_aligned_addr )
{
    const intptr_t external_aligned_addr =
        align_address( align, internal_aligned_addr + extra_size );
    return external_aligned_addr;
}

/* initialises the stack allocator */
/**
 * @brief initialises the stack allocator main strucutre
 *
 * Takes the memory buffer and the size of it and uses this as a pool. It also alignes the
 * first memory block in order to make it invariant so that unocupied memory blocks are
 * already aligned
 *
 */
int stack_allocator_init( stack_allocator_t* stack_allocator,
                          uint8_t* memory_buffer,
                          size_t memory_buffer_size )
{
    /* pre-conditions */
    assert( memory_buffer != NULL );
    assert( memory_buffer_size > 1 );

    /* clean the global variable - watch-out for double initialisation etc */
    memset( stack_allocator, 0, sizeof( stack_allocator_t ) );

    /* assign memory buffer */
    stack_allocator->memory_buffer = memory_buffer;
    stack_allocator->memory_size   = memory_buffer_size;

    /* let's calculate the offset of the first free block */
    intptr_t aligned_addr = align_address( XI_ALIGN, ( intptr_t )memory_buffer );

    /* sanity check */
    assert( ( intptr_t )memory_buffer + memory_buffer_size - aligned_addr >
            extra_space_size );


    stack_allocator_entry_t* first_block = ( stack_allocator_entry_t* )aligned_addr;

    first_block->raw_address = ( intptr_t )memory_buffer;
    first_block->size        = memory_buffer_size;

    list_make_node( first_block, &first_block->list_node );

    int ret = list_push_back( &stack_allocator->free_blocks, &first_block->list_node );

    if ( ret < 0 )
    {
        return ret;
    }

    return 0;
}

static boolean_t matching_size_pred( const void* const value, void* arg )
{
    /* sanity checks */
    assert( NULL != value );
    assert( NULL != arg );

    size_t required_size           = ( size_t )arg;
    stack_allocator_entry_t* entry = ( stack_allocator_entry_t* )value;

    /* calculate first external aligned addr */
    const intptr_t external_aligned_addr =
        get_external_aligned_addr_from_internal_aligned_addr( extra_space_size, XI_ALIGN,
                                                              ( intptr_t )entry );

    /* calculate where the old block ends */
    const intptr_t end_of_the_block = external_aligned_addr + required_size;

    /* calculate where the new free block begins */
    const intptr_t new_free_block_beginning = align_address( XI_ALIGN, end_of_the_block );

    /* calculate where the new free block internal part ends */
    const intptr_t new_free_block_internal_part_ends =
        get_external_aligned_addr_from_internal_aligned_addr( extra_space_size, XI_ALIGN,
                                                              new_free_block_beginning );
    /* calculate where the old block ends */
    const intptr_t old_block_end = ( intptr_t )entry->raw_address + entry->size;

    /* check if there is enough space to fit all the data */
    if ( old_block_end > new_free_block_internal_part_ends )
    {
        return true;
    }

    return false;
}

void* stack_allocator_alloc( stack_allocator_t* stack_allocator, size_t size )
{
    /* look for a block of memory that will be able to store the required amounts of bytes
     */
    list_node_t* ret_node = NULL;

    {
        int ret = list_find( &stack_allocator->free_blocks, &matching_size_pred,
                             ( void* )size, &ret_node );

        if ( ret < 0 )
        {
            return NULL;
        }
    }

    /* sanity check */
    assert( NULL != ret_node );

    stack_allocator_entry_t* original_entry = ( stack_allocator_entry_t* )ret_node->value;

    /* now let's prepare the new block which is going to be returned */
    /* we are going to turn the free block we've found into a allocated block */

    const intptr_t external_aligned_addr =
        get_external_aligned_addr_from_internal_aligned_addr(
            extra_space_size, XI_ALIGN, ( intptr_t )original_entry );

    const intptr_t new_free_entry_internal_unaligned_addr = external_aligned_addr + size;
    const intptr_t new_free_entry_internal_aligned_addr =
        align_address( XI_ALIGN, new_free_entry_internal_unaligned_addr );

    stack_allocator_entry_t* new_free_entry =
        ( stack_allocator_entry_t* )new_free_entry_internal_aligned_addr;

    {
        int ret = list_relocate( &stack_allocator->free_blocks, ret_node,
                                 ( intptr_t )&new_free_entry->list_node );

        if ( ret < 0 )
        {
            return NULL;
        }
    }

    /* update the pointer to the parent structure */
    new_free_entry->list_node.value = new_free_entry;
    new_free_entry->size            = ( intptr_t )original_entry->raw_address +
                           original_entry->size - new_free_entry_internal_unaligned_addr;
    new_free_entry->raw_address = new_free_entry_internal_unaligned_addr;

    original_entry->size -= new_free_entry->size;

    assert( original_entry->raw_address + original_entry->size ==
            new_free_entry->raw_address );

    if ( NULL != new_free_entry->list_node.next )
    {
        stack_allocator_entry_t* next_entry =
            ( stack_allocator_entry_t* )new_free_entry->list_node.next->value;
        assert( new_free_entry->raw_address + new_free_entry->size ==
                next_entry->raw_address );
    }

    /* here the original entry is now ready to be taken */
    list_make_node( original_entry, &original_entry->list_node );

    {
        int ret = list_push_front( &stack_allocator->allocated_blocks,
                                   &original_entry->list_node );
        if ( ret < 0 )
        {
            return NULL;
        }
    }

    const intptr_t original_entry_external_aligned_addr =
        get_external_aligned_addr_from_internal_aligned_addr(
            extra_space_size, XI_ALIGN, ( intptr_t )original_entry );

    return ( void* )original_entry_external_aligned_addr;
}

static boolean_t list_insert_sort_pred( const void* const prev_value,
                                        const void* const tmp_value,
                                        void* node_value,
                                        void* arg )
{
    assert( NULL != tmp_value || NULL != prev_value );

    ( void )arg; /* not - used */

    const stack_allocator_entry_t* const prev_entry = prev_value;
    const stack_allocator_entry_t* const tmp_entry  = tmp_value;
    const stack_allocator_entry_t* const node_entry = node_value;

    if ( NULL != prev_entry && NULL != tmp_entry )
    {
        if ( prev_entry->raw_address < node_entry->raw_address &&
             node_entry->raw_address < tmp_entry->raw_address )
        {
            return true;
        }
    }
    else if ( NULL != prev_entry && prev_entry->raw_address < node_entry->raw_address )
    {
        return true;
    }
    else if ( NULL != tmp_entry && node_entry->raw_address < tmp_entry->raw_address )
    {
        return true;
    }

    return false;
}

/**
 *
 */
void stack_allocator_free( stack_allocator_t* stack_allocator, void* ptr )
{
    if ( NULL == ptr )
    {
        return;
    }

    /* deallocation requires to take the internal address */
    const intptr_t internal_aligned_addres =
        get_internal_aligned_addr_from_external_aligned_addr( extra_space_size, XI_ALIGN,
                                                              ( intptr_t )ptr );

    stack_allocator_entry_t* entry = ( stack_allocator_entry_t* )internal_aligned_addres;

    /* now we can detach this element from the busy blocks list */
    {
        int ret = list_remove( &stack_allocator->allocated_blocks, &entry->list_node );

        if ( ret < 0 )
        {
            debug_log( "%s", "Something went wrong. Given memory block not found on the "
                             "busy blocks list." );
            return;
        }
    }

    /* we have to insert the memory block to the free blocks list ( using insert sort )
     * and perform a merg */

    list_node_t* prev = NULL;
    list_node_t* next = NULL;

    {
        int res = list_find_spot_if( &stack_allocator->free_blocks, &entry->list_node,
                                     &list_insert_sort_pred, NULL, &prev, &next );
        if ( res < 0 )
        {
            debug_log( "Finding a spot in a free blocks failed! - result = %d", res );
            return;
        }
    }

    /* let's check if we can merge elements together */
    stack_allocator_entry_t* prev_entry = NULL == prev ? NULL : prev->value;
    stack_allocator_entry_t* next_entry = NULL == next ? NULL : next->value;

    /* insert the new element */
    {
        int res =
            list_insert_after( &stack_allocator->free_blocks, &entry->list_node, prev );

        if ( res < 0 )
        {
            debug_log( "Failed to insert node at the proper position reason = %d", res );
            return;
        }

        if ( prev )
        {
            assert( prev->next == &entry->list_node );
        }
    }

    /* check next against the entry */
    if ( NULL != next_entry )
    {
        if ( ( entry->raw_address + ( intptr_t )entry->size == next_entry->raw_address ) )
        {
            /* remove the next entry */
            int res = list_remove( &stack_allocator->free_blocks, next );

            if ( res < 0 )
            {
                debug_log( "Failed to remove the next from the list reason = %d", res );
                return;
            }

            entry->size += next_entry->size;
        }
    }

    if ( NULL != prev_entry )
    {
        if ( ( prev_entry->raw_address + ( intptr_t )prev_entry->size ==
               entry->raw_address ) )
        {
            int res = list_remove( &stack_allocator->free_blocks, &entry->list_node );

            if ( res < 0 )
            {
                debug_log( "Failed to remove the entry element from the list reason = %d",
                           res );
                return;
            }

            prev_entry->size += entry->size;
        }
    }
}
