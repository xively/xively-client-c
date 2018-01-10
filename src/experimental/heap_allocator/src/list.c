/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "list.h"
#include <memory.h>

#define CHECK_CND( ret, ... )                                                            \
    if ( __VA_ARGS__ )                                                                   \
    {                                                                                    \
        return ret;                                                                      \
    }

/**
 * @brief helper function to find the last element of the list
 *
 **/
static int get_last_node( list_t* list, list_node_t** ret_node )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == ret_node || NULL != *ret_node );

    list_node_t* head = list->head;
    list_node_t* prev = head;

    while ( NULL != head )
    {
        prev = head;
        head = ( list_node_t* )head->next;
    }

    if ( NULL != prev )
    {
        /* assign return argument */
        *ret_node = prev;
        return 0;
    }

    return ELEMENT_NOT_FOUND;
}

int list_create_list( list_t* list )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list );

    memset( list, 0, sizeof( list_t ) );

    return 0;
}

int list_destroy_list( list_t* list, list_node_dstr_t* dstr_fn )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == dstr_fn );

    list_node_t* head = list->head;

    while ( NULL != head )
    {
        dstr_fn( head );
        list_node_t* const prev = head;
        head                    = head->next;
        prev->next              = NULL;
    }

    return 0;
}

int list_make_node( void* value, list_node_t* node )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == value || NULL == node );

    memset( node, 0, sizeof( list_node_t ) );

    node->value = value;

    return 0;
}

int list_push_back( list_t* list, list_node_t* node )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == node );

    list_node_t* last_node = NULL;
    int ret                = get_last_node( list, &last_node );

    if ( 0 == ret ) /* last element exist */
    {
        last_node->next = node;
    }
    else /* if there was no last element */
    {
        list->head = node;
    }

    return 0;
}

int list_push_front( list_t* list, list_node_t* node )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == node );

    node->next = list->head;
    list->head = node;

    return 0;
}

int list_insert_after( list_t* list, list_node_t* node, list_node_t* element )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == node );

    if ( NULL == element )
    {
        return list_push_front( list, node );
    }

    node->next    = element->next;
    element->next = node;

    return 0;
}

int list_find_spot_if( list_t* list,
                       list_node_t* node,
                       list_if_fit_pred_t* pred,
                       void* arg,
                       list_node_t** out_prev,
                       list_node_t** out_next )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == node || NULL == pred );

    list_node_t* next = list->head;
    list_node_t* prev = NULL;

    /* each slot has to be tested so we are trying to cmp the node value with the two
     * surrounding elements, this way we can find a spot to place the new element */
    while ( NULL != prev || NULL != next )
    {
        /* these values can be null, the predicate implementation has to be prepared for
         * it */
        const void* tmp_value  = NULL == next ? NULL : next->value;
        const void* prev_value = NULL == prev ? NULL : prev->value;

        /* predicate invocation if the result is true stop the search loop we've found the
         * place */
        const boolean_t res = pred( prev_value, tmp_value, node->value, arg );

        if ( res == true )
        {
            break;
        }

        /* advance */
        prev = next;
        next = next->next;
    }

    /* here the new position must checked */
    if ( NULL == prev && NULL == next )
    {
        return ELEMENT_NOT_FOUND;
    }

    /* set return arguments */
    *out_prev = prev;
    *out_next = next;

    return 0;
}

int list_find( list_t* list,
               list_node_value_pred_t* cmp_fn,
               void* pred_arg,
               list_node_t** out_node )
{
    CHECK_CND( WRONG_ARGUMENTS,
               NULL == list || NULL == cmp_fn || NULL == out_node || NULL != *out_node );

    list_node_t* tmp = list->head;

    while ( NULL != tmp )
    {
        boolean_t ret = cmp_fn( tmp->value, pred_arg );

        if ( ret == true )
        {
            /* assign value to an out argument */
            *out_node = tmp;
            return 0;
        }

        tmp = tmp->next;
    }

    return ELEMENT_NOT_FOUND;
}

int find_node_prev( list_t* list, list_node_t* node, list_node_t** out_prev )
{
    CHECK_CND( WRONG_ARGUMENTS,
               NULL == list || NULL == node || NULL == out_prev || NULL != *out_prev );

    list_node_t* tmp  = list->head;
    list_node_t* prev = NULL;

    while ( NULL != tmp )
    {
        if ( tmp == node )
        {
            break;
        }

        prev = tmp;
        tmp  = tmp->next;
    }

    if ( NULL == tmp )
    {
        return ELEMENT_NOT_FOUND;
    }

    *out_prev = prev;

    return 0;
}

int list_remove( list_t* list, list_node_t* node )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == list || NULL == node );

    list_node_t* prev = NULL;

    int ret = find_node_prev( list, node, &prev );

    if ( ret < 0 )
    {
        return ret;
    }

    if ( NULL == prev )
    {
        list->head = node->next;
        node->next = NULL;
        return 0;
    }

    prev->next = node->next;
    node->next = NULL;

    return 0;
}

int list_relocate( list_t* list, list_node_t* node, intptr_t new_addr )
{
    CHECK_CND( WRONG_ARGUMENTS, NULL == node );

    list_node_t* new_spot = ( list_node_t* )new_addr;

    CHECK_CND( WRONG_ARGUMENTS, new_spot == node );

    list_node_t* prev = NULL;

    int ret = find_node_prev( list, node, &prev );

    if ( ret < 0 )
    {
        return ret;
    }

    new_spot->value = node->value;
    new_spot->next  = node->next;

    if ( NULL != prev )
    {
        prev->next = new_spot;
    }
    else
    {
        list->head = new_spot;
    }

    return 0;
}
