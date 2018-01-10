/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#ifndef __XI_THREAD_IDS_H__
#define __XI_THREAD_IDS_H__

/**
 * @brief global libraray thread IDs
 *
 * With one of these IDs can one create event with a target thread.
 */
enum
{
    /* specific thread IDs must start with 0, they specify position of thread
     * in main threadpool */
    XI_THREADID_THREAD_0 = 0,
    XI_THREADID_THREAD_1,
    XI_THREADID_THREAD_2,

    XI_THREADID_ANYTHREAD = 250,
    XI_THREADID_MAINTHREAD
};

#endif /* __XI_THREAD_IDS_H__ */
