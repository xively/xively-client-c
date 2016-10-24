/*
 *  Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 *  This is part of Xively C library.
 * 
 *  Configuration build flag includes for building libxively etc. and 
 *  the cmocka tests. 
 *  
 *  It is intended that this file be "preincluded" in the EWARM build of 
 *  the wrapper code that calls the cmocka tests   
 *  
 *
 */

#ifndef DUMMY_DUMMY_CMOCKA_H
#define DUMMY_DUMMY_CMOCKA_H
 
#include "IAR_STM3241G-EVAL/xi_target_dummy_io_dummy_platform_config.h"
#include "IAR_STM3241G-EVAL/config.h"  // for cmocka, can't just use config.h 
                                       // as cyassl has one too...
 
#endif //DUMMY_DUMMY_CMOCKA_H  
