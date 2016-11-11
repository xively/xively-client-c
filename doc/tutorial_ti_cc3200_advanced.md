## Software you will install during the tutorial.

- wolfSSL embedded SSL library

## Step 3 of 9: Download the Xively C Client library.
## Step 3.5: Download and configure the WolfSSL library

WolfSSL is used to create secure TLS connections.  There is a version of WolfSSL provided on-chip when using the CC3200, but it does not provide Online Certificate Status Protocol ([OCSP](https://en.wikipedia.org/wiki/Online_Certificate_Status_Protocol)) support. OCSP support is crucial in detecting compromised and revoked Certificates, and therefore we have provided instructions on building and linking against a newer version of the WolfSSL library so that OCSP can be leveraged by your project.

### Download WolfSSL library source

- Download WolfSSL library source code from [wolfssl](https://github.com/wolfSSL/wolfssl/releases/tag/v3.9.6)
- Put the WolfSSL main directory under the PATH_TO_XIVELY_LIBRARY_MAIN_FOLDER/xively-client-c/src/import/tls/
- **Important: Rename the folder so it is just `wolfssl`. It should not include the version number.**

### Configure WolfSSL library source

The wolfSSL supports TI-RTOS builds. Below we've provided the needed `{..}/wolfssl/tirtos/products.mak` variable settings for Windows and MacOS to make it easier to get going:

_Alternatively you can follow the steps written on [Using wolfSSL with TI-RTOS](http://processors.wiki.ti.com/index.php/Using_wolfSSL_with_TI-RTOS) to generate wolfSSL static library for CC3200._

#### Windows:

    XDC_INSTALL_DIR        =c:/ti/xdctools_3_32_01_22_core
    BIOS_INSTALL_DIR       =c:/ti/tirex-content/tirtos_cc32xx_2_16_00_08/products/bios_6_45_01_29
    NDK_INSTALL_DIR        =
    TIVAWARE_INSTALL_DIR   =

    export XDCTOOLS_JAVA_HOME=c:/Program Files (x86)/Java/jre1.8.0_51

    ti.targets.arm.elf.M4F =c:/ti/ccsv6/tools/compiler/arm_15.12.3.LTS
    iar.targets.arm.M4F    =
    gnu.targets.arm.M4F    =

#### MacOS:

    XDC_INSTALL_DIR        =/Applications/ti/xdctools_3_32_01_22_core
    BIOS_INSTALL_DIR       =$(HOME)/ti/tirex-content/tirtos_cc32xx_2_16_00_08/products/bios_6_45_01_29
    NDK_INSTALL_DIR        =
    TIVAWARE_INSTALL_DIR   =

    export XDCTOOLS_JAVA_HOME=/Applications/ti/ccsv6/eclipse/jre/Contents/Home

    ti.targets.arm.elf.M4F =/Applications/ti/ccsv6/tools/compiler/arm_15.12.3.LTS
    iar.targets.arm.M4F    =
    gnu.targets.arm.M4F    =

**Important Note**
- Depending on the version of the packages installed, the folder of `BIOS_INSTALL_DIR` may be different. Please check inside the `~/ti/tirex-content` folder to ensure the variable references the correct folder.

#### Further wolfSSL build customizations:

- In the file `{..}/wolfssl/wolfssl/wolfcrypt/settings.h` add a new platform macro `WOLFSSL_NOOS_XIVELY` with the following content. This will configure the wolfSSL features needed for connecting to the Xively service.

**Important Note**
- The position of this macro matters. Please put this new section just before the line `#ifdef WOLFSSL_TIRTOS`.


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

- To compile the above settings please change the following variable in the file `wolfssl/tirtos/wolfssl.bld`:

        -DWOLFSSL_TIRTOS

    to

        -DWOLFSSL_NOOS_XIVELY


- In the file `wolfssl/tirtos/packages/ti/net/wolfssl/package.bld` comment out the last lines for building hwLib:

        /*
        var hwLibptions = {incs: wolfsslPathInclude, defs: " -DWOLFSSL_TI_HASH "
               + "-DWOLFSSL_TI_CRYPT -DTARGET_IS_SNOWFLAKE_RA2"};

        var hwLib = Pkg.addLibrary("lib/wolfssl_tm4c_hw", targ, hwLibptions);
        hwLib.addObjects(wolfSSLObjList);
        */

    _The above can be removed because is not available for CC3200 and not needed for a Xively C Client TLS lib._

- Also in the file `wolfssl/tirtos/packages/ti/net/wolfssl/package.bld`, to add OCSP support add the `"src/ocsp.c"` source file to the wolfSSLObjList variable.

## Step 5 of 9: Build the Xively C Client library.
## Step 6 of 9: Build the wolfSSL embedded SSL library.

- MacOS:
    _From the `{..}/wolfssl/tirtos/` folder:_

        make -f wolfssl.mak all

- Windows:

        PATH=%PATH%;c:\ti\ccsv6\utils\bin
        gmake -f wolfssl.mak all

The resulting file is ```wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f```. This is the WolfSSL library that will provide TLS support to the example application below.

## Step 6 of 9: Build the wolfSSL embedded SSL library.

- MacOS:
    _From the `{..}/wolfssl/tirtos/` folder:_

        make -f wolfssl.mak all

- Windows:

        PATH=%PATH%;c:\ti\ccsv6\utils\bin
        gmake -f wolfssl.mak all

The resulting file is ```wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f```. This is the WolfSSL library that will provide TLS support to the example application below.
