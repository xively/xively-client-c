/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_RESOURCE_MANAGER_H__
#define __XI_RESOURCE_MANAGER_H__

#include "xi_event_handle.h"
#include "xi_data_desc.h"
#include "xi_fs_api.h"
#include "xi_err.h"

/**
 * @struct xi_resource_manager_context_t
 */
typedef struct xi_resource_manager_context_s
{
    xi_data_desc_t* data_buffer; /* the storage for internal data buffer */
    /* pointer to a callback must be disposed at the end of any request processing this is
     * an invariant. If it's NULL it means that the request is finished. While it is set
     * means that one of the operation is pending( open, read, write, close )*/
    xi_event_handle_t callback;
    xi_fs_stat_t resource_stat; /* copy of the resource stat passed with open */
    xi_fs_resource_handle_t resource_handle; /* handle to the opened resource */
    xi_fs_open_flags_t open_flags;           /* copy of open flags passed with open */
    xi_memory_type_t memory_type;            /* data_buffer's ownership */
    size_t data_offset; /* accumuleted value of bytes read or written */
    uint16_t cs;        /* current operation coroutine state */
} xi_resource_manager_context_t;

/**
 * @brief xi_resource_manager_make_context creates fresh context
 *
 * Creates and initialises a context that can be later used for other resource manager
 * functions for reading and writing from/to resource. Context has to be destroyed via
 * xi_resource_manager_free_context function.
 *
 * Behaves like a memory malloc function.
 *
 * @param data_buffer optional parameter if an external memory suppose to be used
 * @param context return parameter, function will create a context at given pointer
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t
xi_resource_manager_make_context( xi_data_desc_t* data_buffer,
                                  xi_resource_manager_context_t** const context );

/**
 * @brief xi_resource_manager_free_context disposes the previously created context
 *
 * Behaves like a memory free function.
 *
 * @param context valid context or NULL
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t
xi_resource_manager_free_context( xi_resource_manager_context_t** const context );

/**
 * @brief xi_resource_manager_open begins the open sequence
 *
 * Starts the open coroutine. Open coroutine will keep calling filesystem's functions in
 * order to make sure that the file is available. In order to achieve that filesystem's
 * stat function will be called as a first function. Result of the stat function will be
 * kept in a xi_resource_manager_context_t structure. Immediately after stat function open
 * filesystem function will be called. After the resource manager finishes with the
 * processing it will call the callback with the result of an operation passed within the
 * callback's state parameter.
 *
 * @todo would be nice to think about a timeout for each function related to long-term
 * resource reading/writing related tasks.
 *
 * @param callback function that will be called upon a success or an error
 * @param resource_type one of the fs resource type
 * @param resource_name name of the resource
 * @param context previously created context
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t xi_resource_manager_open( xi_resource_manager_context_t* const context,
                                     xi_event_handle_t callback,
                                     const xi_fs_resource_type_t resource_type,
                                     const char* const resource_name,
                                     const xi_fs_open_flags_t open_flags,
                                     void* fs_context );

/**
 * @brief xi_resource_manager_read begins the asynchronous reading process
 *
 * Starts the reading coroutine. Reading coroutine will keep calling read fs function
 * untill the whole file is read or an error happen. After the coroutine is done it will
 * call the given callback with a state of an operation. The read buffer can be taken from
 * the xi_resource_manager_context_t's data_buffer.
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t xi_resource_manager_read( xi_resource_manager_context_t* const context,
                                     xi_event_handle_t callback,
                                     void* fs_context );

/**
 * @brief xi_resource_manager_write
 *
 * @warning not yet implemented!
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t xi_resource_manager_write( xi_resource_manager_context_t* const context,
                                      xi_event_handle_t callback,
                                      xi_data_desc_t* const data,
                                      void* fs_context );

/**
 * @brief xi_resource_manager_close begins the asynchronous close operation
 *
 * Starts the close coroutine which will call the filesystem's close function. After it's
 * done it will invoke the given callback with a status of an operation given through
 * state callback's parameter.
 *
 * @param callback function that will be called upon a success or an error
 * @param context previously created context
 * @return XI_STATE_OK if operation succeded, one of error code otherwise
 */
xi_state_t xi_resource_manager_close( xi_resource_manager_context_t* const context,
                                      xi_event_handle_t callback,
                                      void* fs_context );

#endif /* __XI_RESOURCE_MANAGER_H */
