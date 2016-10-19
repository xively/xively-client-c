# How to build and run Xively C Client on TI CC3200

This document describes a method which includes compilation and linkage of Xively C Client sources resulting in a CC3200 compatible static library which can be linked agains a Texas Instruments Code Composer Studio (CCS) example application. The result can be uploaded to the RAM and executed and debugged with the help of the CCS. If everything goes well the result is capable to connect to Xively Services from a CC3200 board.

This method requires OSX development platform although Windows and Linux methods should be very similar.

## Table of Contents
1. [Software Installation](#software-installation)
2. [Building the Xively C Client library](#building-the-xively-c-client-library)
3. [Building the wolfSSL library](#building-the-wolfssl-library)
4. [Building CC3200 application: CCS ent_wlan example](#building-cc3200-application-ccs-ent_wlan-example)

## Software Installation

### TI Code Composer Studio

Download [TI Code Composer Studio](http://www.ti.com/tool/ccstudio) and start installation process.

- choose your ccs install folder, leave the default value "c:\ti" and continue
- select two options from "SimpleLink Wireless MCUs" ->
              \[ "CC3200xx Device Support"
              and "TI ARM Compiler" \] continue
- finalise the installation process

### CC3200 Simplelink WiFi SDK

To install the CC3200 Simplelink WiFi SDK you have two options:

- 1st ( Works on Windows, MacOSX and Linux ) with CCS v6.2.0 or heigher - let the CCS download and install v1.1.0 of the SDK for you
    - open "View"->"Resource Explorer" from the top bar menu
    - select "CC3200 Simplelink WiFi" from the list of available development tools
    - click "Install on Desktop" -> "Make Available Offline" and confirm "Yes" on the popup window
- 2nd ( Only Windows ) - If you need latest version of the SDK ( 1.2.0 )
    - download [CC3200 Simplelink WiFi SDK 1.2.0](http://software-dl.ti.com/dsps/forms/self_cert_export.html?prod_no=CC3200SDK-1.2.0-windows-installer.exe&ref_url=http://software-dl.ti.com/ecs) and install using default settings

## Building the Xively C Client library

:exclamation: **Under construction notes**:

- in file make/mt-os/mt-cc3200 set XI_CC3200_PATH_CCS_TOOLS and XI_CC3200_PATH_SDK variables according to your CCS and SDK install paths and revise the CC and AR variables pointing on compiler and archiver binaries
- currently BSP TIME has unsolved issue around returning elapsed milliseconds from 01/01/1970 since it does not fit into 4 bytes return value. So please change function in file `xively-client-c/src/libxively/time/xi_time.c` to:

        xi_time_t xi_getcurrenttime_seconds()
        {
            return xi_bsp_time_getcurrenttime_milliseconds();
        }

    This is correct for the time being since BSP get milliseconds function returns seconds.

:exclamation:

- Windows:

    Set paths for ```gmake``` and ```mkdir```

        PATH=%PATH%;c:\ti\ccsv6\utils\bin
        PATH=%PATH%;c:\ti\ccsv6\utils\cygwin

    Build and clean the library:

        gmake PRESET=CC3200_REL_MIN clean
        gmake PRESET=CC3200_REL_MIN

- MacOS:

    Build and clean the library:

        make PRESET=CC3200_REL_MIN clean
        make PRESET=CC3200_REL_MIN

For both platforms the PRESET=CC3200_REL_MIN_UNSECURE results in a Xively C Client version withouth secure connection. Primarily non-secure library is for development purposes.

## Building the wolfSSL library

The wolfSSL supports TI-RTOS builds. Follow the steps written on [Using wolfSSL with TI-RTOS](http://processors.wiki.ti.com/index.php/Using_wolfSSL_with_TI-RTOS) to generate wolfSSL static library for CC3200.

Example tirtos/products.mak variable settings for Windows and MacOS:

- Windows:

        XDC_INSTALL_DIR        =c:/ti/xdctools_3_32_01_22_core
        BIOS_INSTALL_DIR       =c:/ti/tirex-content/tirtos_cc32xx_2_16_00_08/products/bios_6_45_01_29
        NDK_INSTALL_DIR        =
        TIVAWARE_INSTALL_DIR   =

        export XDCTOOLS_JAVA_HOME=c:/Program Files (x86)/Java/jre1.8.0_51

        ti.targets.arm.elf.M4F =c:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS
        iar.targets.arm.M4F    =
        gnu.targets.arm.M4F    =

- MacOS:

        XDC_INSTALL_DIR        =/Applications/ti/xdctools_3_31_03_43_core
        BIOS_INSTALL_DIR       =/Applications/ti/tirtos_cc32xx_2_16_01_14/products/bios_6_45_02_31
        NDK_INSTALL_DIR        =
        TIVAWARE_INSTALL_DIR   =

        export XDCTOOLS_JAVA_HOME=/Applications/ti/ccsv6/eclipse/jre/Contents/Home

        ti.targets.arm.elf.M4F = /Applications/ti/ccsv6/tools/compiler/ti-cgt-arm_15.12.1.LTS
        iar.targets.arm.M4F    =
        gnu.targets.arm.M4F    =

Further wolfSSL build customizations:

- In file wolfssl/wolfcrypt/settings.h add a new platform macro WOLFSSL_NOOS_XIVELY with content:

        #ifdef WOLFSSL_NOOS_XIVELY

        /*
         *  Wolf cypher settings
         */
        #undef   WOLFSSL_STATIC_RSA
        // #undef   NO_DH
        #define  NO_DH

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

    This will configure wolfSSL features needed for connecting to Xively services.

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

    This is not available for CC3200 and not needed for a Xively C Client TLS lib.

- In file `wolfssl/tirtos/packages/ti/net/wolfssl/package.bld` to add OCSP support add "src/ocsp.c" source file to wolfSSLObjList variable.

- to build wolfSSL static library type

    - MacOS:

            make -f wolfssl.mak all

    - Windows:

            PATH=%PATH%;c:\ti\ccsv6\utils\bin
            gmake -f wolfssl.mak all

    under directory `wolfssl/tirtos. The result file is ```wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f``` this is the library one should link to an example application to provide wolfSSL symbols.


## Building CC3200 application: CCS ent_wlan example

The end_wlan is a non-os example application providing a wifi connection. This serves as a good light-weight starting point.

Steps to take:

#### Code Coposer Studio + ent_wlan example:

- import the ent_wlan example from the CC3200 SDK's example directory
    - File->Import->Code Composer Studio->CCS Projects->Select search-directory:
    - ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/ent_wlan/ccs
    - click Finish to open the project
- build the project: Project->Build Project
- debug it on the CC3200 device:
    - connect the device to your PC or Mac with USB cable
    - hit the green bug button on the top in the CCS
        This should upload your program to RAM and end up with a debugger standing at the first line of main function in main.c. Reaching this point means you are able to produce and execute CC3200 compatible binary on the device itself.

#### Adding Xively Client code to the ent_wlan example:

- locate the successful wifi connection point in the main.c of the ent_wlan example (arond line 647, comment: "//wait for few moments")
- here put a call on the  ConnectToXively(); function. Its implementation is based on the examples in the Client repo, e.g. xively-client-c/examples/mqtt_logic_producer/src/mqtt_logic_producer.c:

        #include <xively.h>
        #include <stdio.h>

        void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                          void* data,
                                          xi_state_t state )
        {
            printf( "Hello Xively World!, state: %d\n", state );
        }

        void ConnectToXively()
        {
            xi_initialize( "account_id", "xi_username", 0 );

            xi_context_handle_t xi_context = xi_create_context();

            if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
            {
                printf( " xi failed to create context, error: %d\n", xi_context );
            }

            xi_state_t connect_result = xi_connect_to(
                    xi_context,
                    "broker.xively.com", 8883,
                    "35f9a1ba-2f71-4084-9b68-995eac71ef6b",
                    "9Xusd8+jjTghQcggpvEPhu5AFY1GlVnuV9WYwxp8ZT8=",
                    10, 20,
                    XI_SESSION_CLEAN, &on_connection_state_changed );

            xi_events_process_blocking();

            xi_delete_context( xi_context );

            xi_shutdown();
        }

- to make aboves buildable you'll need to
    - add two include paths to your project to help compiler find xively.h and friends: Project->Properties->Build->ARM Compiler->Include Options:
        - xively-client-c/include
        - xively-client-c/include/bsp
    - add two libraries Xively C Client and wolfSSL: Project->Properties->Build->ARM Linker->File Search Path:
        - xively-client-c/bin/cc3200/libxively.a
        - xively-client-c/src/import/tls/wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f
    - add timer_if.h and .c files to the project from directory: ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/common
    - extend the memory map in file cc3200v1p32.cmd inside end_wlan project. This should do it:

            MEMORY
            {
                /* Application uses internal RAM for program and data */
                SRAM_CODE (RWX) : origin = 0x20004000, length = 0x3C000
                //SRAM_DATA (RWX) : origin = 0x20017000, length = 0x19000
            }

            /* Section allocation in memory */

            SECTIONS
            {
                .intvecs:   > RAM_BASE
                .init_array : > SRAM_CODE
                .vtable :   > SRAM_CODE
                .text   :   > SRAM_CODE
                .const  :   > SRAM_CODE
                .cinit  :   > SRAM_CODE
                .pinit  :   > SRAM_CODE
                .data   :   > SRAM_CODE
                .bss    :   > SRAM_CODE
                .sysmem :   > SRAM_CODE
                .stack  :   > SRAM_CODE(HIGH)
            }

    - implement two functions as follows:

            #include <time.h>
            #include <xi_bsp_rng.h>
            #include <xi_bsp_time.h>

            time_t XTIME(time_t * timer)
            {
                return xi_bsp_time_getcurrenttime_milliseconds();
            }

            uint32_t xively_ssl_rand_generate()
            {
                return xi_bsp_rng_get();
            }

    - update wifi settings in main.c

        - update AP name and password defines according to your wifi settings:

                #define ENT_NAME    "AccessPointName"
                #define USER_NAME   "UsernameIfAny"
                #define PASSWORD    "Password"

        - select security type according to your wifi settings, in case of WPA2 set

                g_SecParams.Type = SL_SEC_TYPE_WPA_WPA2;

            and delete variable eapParams and pass NULL as last attribute to connect function:

                lRetVal = sl_WlanConnect(ENT_NAME,strlen(ENT_NAME),NULL,&g_SecParams,NULL);
