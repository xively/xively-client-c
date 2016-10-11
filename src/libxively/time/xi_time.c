// Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
//
// This is part of the Xively C Client library,
// it is licensed under the BSD 3-Clause license.

#include <xi_bsp_time.h>

xi_time_t xi_getcurrenttime_seconds()
{
    return xi_bsp_time_getcurrenttime_milliseconds() / 1000;
}

xi_time_t xi_getcurrenttime_milliseconds()
{
    return xi_bsp_time_getcurrenttime_milliseconds();
}
