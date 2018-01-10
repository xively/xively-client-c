/* Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include "xi_internals.h"

/* by default set the function pointers to the enabled type/system/storage
 * if none set use empty function pointer table
 */
#include "xi_fs_header.h"

#if defined( XI_FS_MEMORY ) || defined( XI_FS_DUMMY ) || defined( XI_FS_POSIX )
xi_internals_t xi_internals = {{sizeof( xi_fs_functions_t ), &xi_fs_stat, &xi_fs_open,
                                &xi_fs_read, &xi_fs_write, &xi_fs_close, &xi_fs_remove}};
#else
#error No filesystem defined!!!
#endif
