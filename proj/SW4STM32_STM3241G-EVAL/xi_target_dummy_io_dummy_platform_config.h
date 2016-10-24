/*
 *  Copyright (c) 2003-2015, LogMeIn, Inc. All rights reserved.
 *  This is part of Xively C library.
 * 
 *  Configuration Build flags for building libxively, libcyassl, freeRTOS, 
 *  LwIP, tinytest utests and xively examples targeted for the STM3241G-EVAL
 *  demo board (Cortex-m4)
 *  
 *  It is intended that this file be "preincluded" in the AC6's System Workbench 
 *  (SW4) of all components.   
 *  
 *  Note: In some cases these flags may be redundant but are included to ensure
 *  all parts of the builds (libs, application etc.) use the same configuation
 *  of included header files.
 *  Flags with "CONFIG" comment are those that would be determined via the 
 *  makefiles (either directly or by ./configure in the case of cyassl) 
 *  if this were a gcc build.
 *  Flags with a "Target Limitation" comment are those that must be constrained
 *  for the target environment due to target or the related low-level libs.
 *  Flags with an "EWARM Limitation" comment are for constraints originally
 *  driven by IAR's EWARM environment but which have been retained here for
 *  consistency and convienience.
 *
 */    
 
#ifndef XI_TARGET_DUMMY_IO_DUMMY_PLATFORM_CONFIG_H
#define XI_TARGET_DUMMY_IO_DUMMY_PLATFORM_CONFIG_H

// mt-config xively build flags:
#define XI_CROSS_TARGET                 //CONFIG
#define HAVE_SNI                        //CONFIG
#define HAVE_OCSP                       //CONFIG
#define XI_DEBUG_OUTPUT  1              //CONFIG
#define XI_DEBUG_ASSERT  1              //CONFIG
#define XI_DEBUG_EXTRA_INFO  1          //CONFIG
#define XI_CONTROL_TOPIC_ENABLED  1     //CONFIG

#define XI_IO_LAYER  1                  //CONFIG
#define XI_PLATFORM_EVENT_LOOP  1       //CONFIG
#define XI_PLATFORM_BASE_DUMMY          //CONFIG 

#define XI_NO_EMPTY_PARAMS              //EWARM Limitation
#define XI_IO_LAYER_POSIX_COMPAT 1      //CONFIG
#define XI_FS_DUMMY                     //CONFIG
 
// cyassl build flags:  Note these values are generally driven by 
// target limitations
#define FREERTOS                        //CONFIG
#define SINGLE_THREADED                 //CONFIG
#define USE_CERT_BUFFERS_1024           //CONFIG
#define BENCH_EMBEDDED                  //CONFIG
#define NO_FILESYSTEM                   //CONFIG
#define NO_WRITEV                       //CONFIG
#define CYASSL_USER_IO                  //CONFIG
#define NO_DEV_RANDOM                   //CONFIG
#define STM32F4_RNG                     //CONFIG
#define HAVE_TLS_EXTENSIONS             //CONFIG
#define HAVE_SNI                        //CONFIG

// lwip build flags:   in lwipopts.h

// freertos build flags:  in FreeRTOSConfig.h

// tinytest build flags:
#define XI_EMBEDDED_TESTS               //CONFIG
#define NO_FORKING                      //CONFIG

// application build flags
#define USE_SERIAL                      //Target Limitation

// common build flags:
#define STM32F417xx                     //Target Limitation
#define USE_HAL_DRIVER                  //Target Limitation

#endif //XI_TARGET_DUMMY_IO_DUMMY_PLATFORM_CONFIG_H
