# How to build and run Xively C Client on TI CC3200

This document describes a method which includes compilation and linkage of Xively C Client sources resulting in a CC3200 compatible static library which can be linked agains a Texas Instruments Code Composer Studio (CCS) example application. The result can be uploaded to the RAM and executed and debugged with the help of the CCS. If everything goes well the result is capable to connect to Xively Services from a CC3200 board.

This method requires OSX development platform although Windows and Linux methods should be very similar.

#### Prerequisites:

- OSX as development platform
- a [TI CC3200](http://www.ti.com/tool/cc3200-launchxl?keyMatch=launchpad%20cc3200&tisearch=Search-EN-Everything) board
- to download the Code Composer Studio and the SDK you'll need to create a TI account, then these tools are free to download.
- [TI Code Composer Studio](http://www.ti.com/tool/ccstudio) installed on OSX
- [TI CC3200 SDK](http://www.ti.com/tool/cc3200sdk)
    - note: the SDK is downloadable in a Windows .exe compressed package format.
    - after extracting it on a Windows machine you should place it somewhere in your OSX directory structure, e.g. to ~/ti/CC3200SDK

## Table of Contents
1. [Building the Xively C Client library](#building-the-xively-c-client-library)
2. [Building the wolfSSL library](#building-the-wolfssl-library)
3. [Building CC3200 application: CCS ent_wlan example](#building-cc3200-application-ccs-ent_wlan-example)


## Building the Xively C Client library

:exclamation: **Under construction notes**:

- currently auto-tests and Xively Client examples are not buildable for CC3200 thus in Makefile leave only $(XI) in the target *all*
- add XI_COMPILER_FLAGS += -DNO_OCSP line to file mt-cc3200, some problems were not solved with OCSP
- since current time implementation is very "artificial" the scheduling of events seems not to work as it should. Thus in xively.c in function xi_connect the xi_evtd_execute_in scheduled execution should be changed to xi_evtd_execute immediate execution. This latter does not need the last "delay" parameter and its return value differs as well.

This step results in a static library suitable for CC3200. The command

    make PRESET=CC3200_REL_MIN

should do it, result should be the file xively-client-c/obj/cc3200/libxively.a
This preset includes TLS connection. To disable TLS edit the preset in file mt-presets.mk

    make PRESET=CC3200_REL_MIN clean

cleans the output of the previous build.

## Building the wolfSSL library

The wolfSSL supports TI-RTOS builds. Follow the steps written on [Using wolfSSL with TI-RTOS](http://processors.wiki.ti.com/index.php/Using_wolfSSL_with_TI-RTOS) to generate wolfSSL static library for CC3200.

Before starting apply the following customizations made for a custom wolfSSL build:

In file wolfcrypt/settings.h find the WOLFSSL_TIRTOS section and update as follows:

    #ifdef WOLFSSL_TIRTOS

        #define SINGLE_THREADED

        #define NO_OCSP
        #define NO_DES3
        #define NO_OLD_TLS
        #define NO_PSK
        #define NO_PWDBASED
        #define CUSTOM_RAND_GENERATE xively_ssl_rand_generate
        #define CUSTOM_XTIME xively_ssl_time
        #define HAVE_SNI

        // #define HAVE_OCSP
        // #define HAVE_CERTIFICATE_STATUS_REQUEST

        #define SMALL_SESSION_CACHE
        #define NO_CLIENT_CACHE
        #define WOLFSSL_SMALL_STACK
        #define WOLFSSL_USER_IO
        #define TARGET_IS_CC3200

        #define NO_RABBIT
        #define NO_MD4
        #define NO_RC4
        #define NO_DH
        #define NO_DSA
        #define NO_SHA
        #define NO_HC128

        #define SIZEOF_LONG_LONG 8
        #define NO_WRITEV
        #define NO_WOLFSSL_DIR
        #define USE_FAST_MATH
        #define TFM_TIMING_RESISTANT
        #define NO_DEV_RANDOM
        #define NO_FILESYSTEM
        #define USE_CERT_BUFFERS_2048
        #define NO_ERROR_STRINGS
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

        #include <ti/sysbios/hal/Seconds.h>

    #endif

This will disable a bunch of features in wolfSSL seemingly not required for a Xively connection but drastically deflate size of the result static library.

In file wolfcrypt/random.c find the WOLFSSL_TIRTOS section and comment it out to force wolfSSL to use custom random function in the

    #elif defined(CUSTOM_RAND_GENERATE)

section.

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
- copy Xively C Client initialize code from one of the examples in from the Client repo
- to make it build you'll need to
    - include xively.h
    - link CC3200 static library: xively-client-c/obj/cc3200/libxively.a
    - link the wolfSSL static library: xively-client-c/src/import/tls/wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f
    - and implement two functions

The functions are:

    uint_least32_t ti_sysbios_hal_Seconds_get__E( void );

    uint32_t xively_ssl_rand_generate();
