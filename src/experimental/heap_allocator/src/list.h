/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __LIST_H__
#define __LIST_H__

/**
 * @file list.h
 * @brief Implementation of the single linked list for stack allocator purposes.
 *
 * This implementation contains standard list functionality such as push_front, push_back,
 *find, insert plus some functionality that is not standard but makes the implementation
 *of the allocator itself easier.
 **/

#include <stdint.h>

typedef enum list_errors_e {
    WRONG_ARGUMENTS   = -0x0001,
    INTERNAL_ERROR    = -0x0002,
    ELEMENT_NOT_FOUND = -0x0003
} list_errors_t;

typedef enum boolean_e {
    false = 0,
    true  = 1,
} boolean_t;

typedef struct list_node_s
{
    struct list_node_s* next;
    void* value;
} list_node_t;


typedef struct list_s
{
    struct list_node_s* head;
} list_t;

typedef boolean_t( list_node_value_pred_t )( const void* const v, void* arg );
typedef boolean_t( list_if_fit_pred_t )( const void* const prev_value,
                                         const void* const tmp_value,
                                         void* node_value,
                                         void* arg );
typedef void( list_node_dstr_t )( list_node_t* node );

/* API */

int list_create_list( list_t* list );
int list_destroy_list( list_t* list, list_node_dstr_t* dstr_fn );

int list_make_node( void* value, list_node_t* out_node );

int list_push_back( list_t* list, list_node_t* node );
int list_push_front( list_t* list, list_node_t* node );
int list_insert_after( list_t* list, list_node_t* node, list_node_t* element );
int list_find_spot_if( list_t* list,
                       list_node_t* node,
                       list_if_fit_pred_t* pred,
                       void* arg,
                       list_node_t** out_prev,
                       list_node_t** out_next );

int list_find( list_t* list,
               list_node_value_pred_t* cmp_fn,
               void* pred_arg,
               list_node_t** out_node );
int list_remove( list_t* list, list_node_t* node );

int list_relocate( list_t* list, list_node_t* node, intptr_t new_addr );

#endif
