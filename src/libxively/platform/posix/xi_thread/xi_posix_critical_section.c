/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_err.h"
#include "xi_critical_section_def.h"
#include "xi_debug.h"
#include "xi_allocator.h"
#include "xi_macros.h"

#ifdef __cplusplus
extern "C" {
#endif

xi_state_t xi_init_critical_section( struct xi_critical_section_s** cs )
{
    assert( cs != NULL );

    xi_state_t ret_state = XI_STATE_OK;

    XI_ALLOC_AT( struct xi_critical_section_s, *cs, ret_state );

err_handling:
    return ret_state;
}

void xi_lock_critical_section( struct xi_critical_section_s* cs )
{
    assert( cs != 0 );

    while ( !__sync_bool_compare_and_swap( &cs->cs_state, 0, 1 ) )
        ;
}

void xi_unlock_critical_section( struct xi_critical_section_s* cs )
{
    assert( cs != 0 );

    while ( !__sync_bool_compare_and_swap( &cs->cs_state, 1, 0 ) )
        ;
}

void xi_destroy_critical_section( struct xi_critical_section_s** cs )
{
    assert( cs != 0 );

    XI_SAFE_FREE( *cs );
}

#ifdef __cplusplus
}
#endif
