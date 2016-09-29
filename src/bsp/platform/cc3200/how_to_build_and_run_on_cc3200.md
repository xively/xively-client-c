# How to build and run Xively C Client on TI CC3200

This document describes a method which includes compilation and linkage of Xively C Client sources resulting in a CC3200 compatible static library which can be linked agains a Texas Instruments Code Composer Studio (CCS) example application. The result can be uploaded to the RAM and executed and debugged with the help of the CCS. If everything goes well the result is capable to connect to Xively Services from a CC3200 board.

This method requires OSX development platform although Windows and Linux methods should be very similar.

#### Prerequisites:

- OSX as development platform
- a [TI CC3200](http://www.ti.com/tool/cc3200-launchxl?keyMatch=launchpad%20cc3200&tisearch=Search-EN-Everything) board
- to download the Code Composer Studio and the SDK you'll need to create a TI account, then these tools are free to download.
- [TI Code Composer Studio](http://www.ti.com/tool/ccstudio) installed on OSX
- Install the TI-RTOS CC32xx add-on from the CCS App Center (View-->CCS App Center)
- [TI CC3200 SDK](http://www.ti.com/tool/cc3200sdk)
    - note: the SDK is downloadable in a Windows .exe compressed package format.
    - after extracting it on a Windows machine you should place it somewhere in your OSX directory structure, e.g. to ~/ti/CC3200SDK

## Table of Contents
1. [Building the Xively C Client library](#building-the-xively-c-client-library)
2. [Building the wolfSSL library](#building-the-wolfssl-library)
3. [Building CC3200 application: CCS ent_wlan example](#building-cc3200-application-ccs-ent_wlan-example)


## Building the Xively C Client library

:exclamation: **Under construction notes**:

- in file mt-cc3200 set XI_CC3200_PATH_SDK path according to your SDK install location and revise the CC and AR variables pointing on compiler and archiver binaries
- add XI_COMPILER_FLAGS += -DNO_OCSP line to file mt-cc3200, some problems were not solved with OCSP
- currently BSP TIME has unsolved issues around returning elapsed milliseconds since 01.01.1970. since it does not fit into 4 bytes return value. So please change function in file `xively-client-c/src/libxively/time/xi_time.c` to:

        xi_time_t xi_getcurrenttime_seconds()
        {
            return xi_bsp_time_getcurrenttime_milliseconds();
        }

    This is correct for the time being since BSP get milliseconds function returns seconds.

:exclamation:

This step results in a static library suitable for CC3200. The command

    make PRESET=CC3200_REL_MIN

should do it, result should be the file xively-client-c/obj/cc3200/libxively.a
This preset includes TLS connection. To disable TLS edit the preset in file mt-presets.mk

    make PRESET=CC3200_REL_MIN clean

cleans the output of the previous build.

## Building the wolfSSL library

The wolfSSL supports TI-RTOS builds. Follow the steps written on [Using wolfSSL with TI-RTOS](http://processors.wiki.ti.com/index.php/Using_wolfSSL_with_TI-RTOS) to generate wolfSSL static library for CC3200.

Before starting apply the following customizations made for a Xively wolfSSL build:

- In file wolfssl/wolfcrypt/settings.h add a new platform macro WOLFSSL_NOOS_XIVELY with content:

        #ifdef WOLFSSL_NOOS_XIVELY

            /*
             *  Wolf cypher settings
             */
            #undef   WOLFSSL_STATIC_RSA
            // #undef   NO_DH
            #define  NO_DH
            #define  HAVE_ECC

            #define  NO_DES
            #define  NO_DES3
            #define  NO_DSA
            #define  NO_HC128
            #define  NO_MD4
            #define  NO_OLD_TLS
            #define  NO_PSK
            #define  NO_PWDBASED
            #define  NO_RC4
            #define  NO_RABBIT
            #define  NO_SHA512

            #define SINGLE_THREADED

            #define CUSTOM_RAND_GENERATE xively_ssl_rand_generate
            #define CUSTOM_XTIME xively_ssl_time

            #define HAVE_SNI
            #define HAVE_OCSP
            #define HAVE_CERTIFICATE_STATUS_REQUEST

            #define SMALL_SESSION_CACHE
            #define NO_CLIENT_CACHE
            #define WOLFSSL_SMALL_STACK
            #define WOLFSSL_USER_IO
            #define TARGET_IS_CC3200

            #define SIZEOF_LONG_LONG 8
            #define NO_WRITEV
            #define NO_WOLFSSL_DIR
            #define USE_FAST_MATH
            #define TFM_TIMING_RESISTANT
            #define NO_DEV_RANDOM
            #define NO_FILESYSTEM
            #define USE_CERT_BUFFERS_2048
            // #define NO_ERROR_STRINGS
            #define USER_TIME
            #define HAVE_ECC
            // #define HAVE_ALPN
            #define HAVE_TLS_EXTENSIONS
            #define HAVE_AESGCM
            // #define HAVE_SUPPORTED_CURVES
            #define ALT_ECC_SIZE

            #ifdef __IAR_SYSTEMS_ICC__
                #pragma diag_suppress=Pa089
            #elif !defined(__GNUC__)
                /* Suppress the sslpro warning */
                #pragma diag_suppress=11
            #endif

        #endif

    This will disable a bunch of features in wolfSSL seemingly not required for a Xively connection but drastically deflate size of the result static library. Currently OCSP is disabled as well.

- To compile in the above settings replace the

        -DWOLFSSL_TIRTOS

    to

        -DWOLFSSL_NOOS_XIVELY

    in file `wolfssl/tirtos/wolfssl.bld`.


- In file `wolfssl/tirtos/packages/ti/net/wolfssl/package.bld` comment out the last lines for building hwLib:

        /*
        var hwLibptions = {incs: wolfsslPathInclude, defs: " -DWOLFSSL_TI_HASH "
               + "-DWOLFSSL_TI_CRYPT -DTARGET_IS_SNOWFLAKE_RA2"};

        var hwLib = Pkg.addLibrary("lib/wolfssl_tm4c_hw", targ, hwLibptions);
        hwLib.addObjects(wolfSSLObjList);
        */

    This is not available for CC3200 and not needed at all.


## Building CC3200 application: CCS ent_wlan example

The end_wlan is a non-os example application providing a wifi connection. This serves as a good light-weight starting point.

Steps to take:

#### Code Coposer Studio + ent_wlan example:

- create a new workspace for Code Composer Studio
- import the ent_wlan example from the CC3200 SDK's example directory
- build it
- debug it on the device
    - connect the device to the Mac
    - hit the green bug button on the top in the CCS

#### Adding Xively Client code to the ent_wlan example:

- locate the successful wifi connection point in the main.c of the ent_wlan example (comment: "wait for few moments")
- call Xively C Client initialize code taken from one of the examples in from the Client repo, e.g.

        void on_connected( xi_context_handle_t in_context_handle, void* data, xi_state_t state ) { }

        void ConnectToXively()
        {
            xi_initialize( "account_id", "xi_username", 0 );

            xi_context_handle_t xi_context = xi_create_context();

            if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
            {
                printf( " xi failed to create context, error: %d\n", -xi_context );
            }

            xi_state_t connect_result = xi_connect_to(
                    xi_context,
                    "broker.xively.com", 8883,
                    "35f9a1ba-2f71-4084-9b68-995eac71ef6b", "9Xusd8+jjTghQcggpvEPhu5AFY1GlVnuV9WYwxp8ZT8=",
                    10, 20,
                    XI_SESSION_CLEAN, &on_connected );

            xi_events_process_blocking();

            xi_delete_context( xi_context );

            xi_shutdown();
        }

- to make it build you'll need to
    - include xively.h
    - link CC3200 static library: xively-client-c/obj/cc3200/libxively.a
    - link the wolfSSL static library: xively-client-c/src/import/tls/wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f
    - and implement two functions

The functions and example implementations are:

    #include <time.h>
    #include <xi_bsp_rng.h>
    #include <xi_bsp_time.h>

    time_t XTIME(time_t * timer)
    {
        /* note: this is fine for the time beeing since BSP get milliseconds returns seconds */
        return xi_bsp_time_getcurrenttime_milliseconds();
    }

    uint32_t xively_ssl_rand_generate()
    {
        return xi_bsp_rng_get();
    }
