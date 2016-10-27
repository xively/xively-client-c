# How to connect your CC3200 to Xively

Please scan the whole tutorial to get familiar with it. Then complete each step in a sequential manner; each step builds upon the previous one. 

This tutorial supports mainly MacOS and Windows, though the Linux flow should be somewhat similar to MacOS.


## What you will learn.

This tutorial will teach you how to build, link and deploy a Xively C Client onto the CC3200 using the Code Composer Studio™. Then you will learn how to connect your device to Xively.


## Hardware you will need.

Texas Instruments [SimpleLink™ Wi-Fi® CC3200 LaunchPad™](http://www.ti.com/tool/cc3200-launchxl) development kit. 


## Software you will install during the tutorial.

- Code Composer Studio™
- CC3200 Simplelink™ WiFi SDK
- wolfSSL embedded SSL library
- Xively C Client library
- CC3200 Uniflash


## Step 1 of 8: Install the Code Composer Studio™.

Code Composer Studio™ includes the toolchain (compiler) you'll need to build for the CC3200 and a java-based IDE.

[Download](http://www.ti.com/tool/ccstudio) the Code Composer Studio™ appropriate for your operating system (Windows, Linux or MacOS). 

1. Complete the free registration.
2. Validate your email address.
3. Complete the brief export approval form and click ```Submit```.
4. Upon approval, click ```Download``` to proceed. Monitor the download process to completion.
5. Once download is complete, start the installation. 
6. Accept the license agreement and click ```Next >```.
7. Choose the default install folder and click ```Next >```. Or, if you install into a custom directory, then please note its path as you will need to refer to it later.

	By default the path should be ```c:\ti``` on Windows and ```/Applications/ti``` on MacOS.

8. Enable the following two options under ```SimpleLink Wireless MCUs```:
	1. ```CC3200xx Device Support```
	2. ```TI ARM Compiler```

9. Click ```Next >``` twice more, and click ```Finish``` when the button becomes enabled.
10. Once installation completes, click ```Finish``` to leave the installer.


## Step 2 of 8: Install the CC3200 Simplelink™ WiFi SDK.

These are the platform libraries that you'll need to compile and link against when writing software for the CC3200.

1. Launch Code Composer Studio™.
2. If prompted to ```Select a Workspace```, click ```OK``` to select the default path.
3. 	Select  ```View```->```Resource Explorer``` from the top bar menu.
4. Select ```CC3200 Simplelink WiFi``` from the list of available development tools.
5. On the right side of the screen, click the ```Install on Desktop``` down-arrow icon and select ```Make Available Offline```. Confirm ```Yes``` on the popup window.
6. A ```Dependencies``` popup may appear.  Click ```OK``` to download any software dependencies.

*NOTE*: Windows users may download the [CC3200 Simplelink™ WiFi SDK](http://www.ti.com/tool/cc3200sdk) directly outside of the Code Composer Studio™ if you wish. Once downloaded, please install using the default settings.


## Step 3 of 8: Install the Xively C Client library.

### Download the xively-client-c library source
1. Download the library source code from [xively-client-c](https://github.com/xively/xively-client-c).  Git [clone](https://help.github.com/articles/set-up-git/) the repository or download the source archive from the right side of the github page.

### Prebuild Configuration of the Xively C Client

#### Configure Make Target file mt-cc3200
1. Open the file ```make/mt-os/mt-cc3200``` in your favorite friendly text editor.
2. Scroll the HOSTS section devoted to your host platform: ```MAC HOST OS```, ```WINDOWS HOST OS```, or ```LINUX HOST OS```.
2. In your newly identified host's section, set ```XI_CC3200_PATH_CCS_TOOLS``` and ```XI_CC3200_PATH_SDK``` to your Code Composer Studio™ and SDK install paths, respectively.  **If you chose the default installation paths for these installations then these values should already be valid and you shouldn't need to change anything.**
3. The toolchain that Code Composer Studio™ downloaded might differ from the default that's configured in this ```mt-cc3200``` file.
	1. Please browse to the path which you set ```XI_CC3200_PATH_CCS_TOOLS```.
	2. Open up the ```compiler/``` and note the the name of the toolchain.
	3. Compare this to the toolchain name stored in the ```COMPILER``` variable near the top of the file in ```mt-cc3200```.  Update the ```COMPILER``` variable as necessary.

### Build Xively C Client library

The process for building slightly depends on your host OS:

#### Windows:

Set paths for ```gmake``` and ```mkdir```

    PATH=%PATH%;c:\ti\ccsv6\utils\bin
    PATH=%PATH%;c:\ti\ccsv6\utils\cygwin

Clean and build the library:

    gmake PRESET=CC3200_REL_MIN clean
    gmake PRESET=CC3200_REL_MIN

#### MacOS and Linux:

Clean and build the library:

_From the `xively-client-c` root folder:_

    make PRESET=CC3200_REL_MIN clean
    make PRESET=CC3200_REL_MIN

For all host platforms the PRESET=CC3200_REL_MIN_UNSECURE results in a Xively C Client version without a secure TLS connection. This can be useful for development purposes against local MQTT brokers, like [mosquitto](https://mosquitto.org/) but is not advised for devices in a real production enviorment.


## Step 4 of 8: Install the wolfSSL embedded SSL library.

WolfSSL is used to create secure TLS connections.  There is a version of WolfSSL provided on-chip when using the CC3200, but it does not provide OCSP support. OCSP support is crucial in detecting compromised and revoked Certficates, and therefore we have provided instructions on building and linking against a newer version of the WolfSSL library so that OCSP can be leveraged by your project.

### Download WolfSSL library source
- Download WolfSSL library source code from [wolfssl](https://github.com/wolfSSL/wolfssl/releases/tag/v3.9.6)
- Put the WolfSSL main directory under the PATH_TO_XIVELY_LIBRARY_MAIN_FOLDER/xively-client-c/src/import/tls/
    - **If needed rename the folder so it is just `wolfssl`; it does not need to include the version number**

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

**Important Note** - Depending on the version of the packages installed, the folder of `BIOS_INSTALL_DIR` may be different. Please check inside the `~/ti/tirex-content` folder to ensure the variable references the correct folder.

#### Further wolfSSL build customizations:

- In the file `{..}/wolfssl/wolfssl/wolfcrypt/settings.h` add a new platform macro `WOLFSSL_NOOS_XIVELY` with the following content. This will configure the wolfSSL features needed for connecting to the Xively service.

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

- To compile the above settings change the following variable in the file `wolfssl/tirtos/wolfssl.bld`:

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

#### Build wolfSSL static library

- MacOS:
    _From the `{..}/wolfssl/tirtos/` folder:_

        make -f wolfssl.mak all

- Windows:

        PATH=%PATH%;c:\ti\ccsv6\utils\bin
        gmake -f wolfssl.mak all

The resulting file is ```wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f```. This is the WolfSSL library will provide TLS support to the example application below.


## Step 5 of 8: Create your Xively (digital) device.

(todo)

In Device page, download the creds (devicePassword & deviceID).

In MQTT tab, select the topic from the dropdown and blue text to copy.


## Step 6 of 8: Build your client application.

We suggest the _ent_wlan_ networking example from the CC3200 SDK as the basis for connecting to Xively. We will first import the example into Code Composer Studio™, and then add some code to build your IoT Client connection to the Xively service.

### Build the _ent_wlan_ Example

#### Import _ent_wlan_
1. In Code Composer Studio™, select ```File```->```Import```.
2. Select ```Code Composer Studio™```->```CCS Projects``` and click ```Next >```
3. To the right of ```Select search-directory``` click Browse.
4. From this directory, browse to ```ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/ent_wlan``` and highlight the ```ccs``` folder.  Click ```Open```.
5. Click ```Finish```.

#### Build and Run the Example
1. Select ```Project```-> ```Build Project```
	1. When complete, you should see in the ```Console```:

			<Linking>
			Finished building target: ent_wlan.out
			...
			**** Build Finished ****

2. Before the first execution, you will need to create a Configuration so that Code Composer Studio™ knows which platform you're loading the source onto.
	1. Select ```View``` -> ```Target Configurations```.  The ```Target Configurations``` panel opens to the right side of the IDE.
	2. Right click on ```User Defined``` and select ```New Target Configuration```.
	3. Choose a filename or keep the default.  Click ```Finish```.
	4. In the ```Connection``` pulldown, select ```Stellaris In-Circuit Debug Interface```.
	5. Select the box next to ```CC3200``` to add a check mark.
	6. Click the ```Save``` button to the right. _You may have to scroll the middle window to the right to see the button._
	7. Back in the ```Target Configurations``` panel to the right, expand ```User Defined```.
	8. Right click on your new target configuration and select ```Set as Default```.

3. 	Execute the example on the CC3200 device
	1. connect the device to your PC or Mac with USB cable
	2. hit the green bug button on the top in the CCS, or select ```Run``` ->```Debug```

This should upload your program to RAM and end up with a debugger standing at the first line of main function in main.c.

Reaching this point means you are able to produce and execute CC3200 compatible binary on the device itself.  Congratulations!

**NOTE**: As per Texas Instruments instructions, keep the J15 Jumper set to ON and push Reset button on the board before each debug session. In case of trouble get help from [TI's CC3200 help doc](http://www.ti.com/lit/ds/symlink/cc3200.pdf)

### Add the Xively Client to ent_wlan

Next we're going to add a function to connect to the Xively Broker. Its implementation is based on the examples in the Client repo, e.g. `xively-client-c/examples/mqtt_logic_producer/src/mqtt_logic_producer.c`.

- Paste the following code within `main.c` of the _ent_wlan_ anywhere in the main portion of the file

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
            xi_initialize( "xi_account_id", "xi_device_id", 0 );

            xi_context_handle_t xi_context = xi_create_context();

            if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
            {
                printf( " xi failed to create context, error: %d\n", xi_context );
            }

            xi_state_t connect_result = xi_connect(
                    xi_context,
                    "35f9a1ba-2f71-4084-9b68-995eac71ef6b",         // Xively Device Id
                    "9Xusd8+jjTghQcggpvEPhu5AFY1GlVnuV9WYwxp8ZT8=", // Xively Device Secret
                    10, 20,
                    XI_SESSION_CLEAN, &on_connection_state_changed );

            xi_events_process_blocking();

            xi_delete_context( xi_context );

            xi_shutdown();
        }

- you can get a Device ID and Device Secret from the Devices Page of CPM. Navigate to a specific device page and click the Get Password button in the top right of that page to download a file with the values.

- locate the successful wifi connection point in the `main.c` of the ent_wlan example (around line 647, comment: "//wait for few moments"). Here put a call on the ConnectToXively(); function we just added. 

- to make aboves buildable you'll need to
    - add two include paths to your project to help the compiler find `xively.h` and friends: ```Project```->```Properties```->```Build```->```ARM Compiler```->```Include Options```:
        - `xively-client-c/include`
        - `xively-client-c/include/bsp`
    - add two libraries Xively C Client and wolfSSL: ```Project```->```Properties```->```Build```->```ARM Linker```->```File Search Path```:
        - `xively-client-c/bin/cc3200/libxively.a`
        - `xively-client-c/src/import/tls/wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f`
    - add two files to the project: ```Project```->```Add Files```: `ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/common`
        - `timer_if.h`
        - `timer_if.c`
    - implement two functions as follows:

            #include <time.h>
            #include <xi_bsp_rng.h>
            #include <xi_bsp_time.h>

            time_t XTIME(time_t * timer)
            {
                return xi_bsp_time_getcurrenttime_seconds();
            }

            uint32_t xively_ssl_rand_generate()
            {
                return xi_bsp_rng_get();
            }

    - update the memory map in file `cc3200v1p32.cmd`. This should do it:

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

    - update wifi settings in main.c

        - update AP name and password defines according to your wifi settings:

                #define ENT_NAME    "AccessPointName"
                #define USER_NAME   "UsernameIfAny"
                #define PASSWORD    "Password"

        - select a security type in the `EntWlan()` function according to your wifi settings.  For example, in the case of WPA2 set

                g_SecParams.Type = SL_SEC_TYPE_WPA_WPA2;

            and delete the variable `eapParams`. Then pass NULL as the last attribute to the connect function:

                lRetVal = sl_WlanConnect(ENT_NAME,strlen(ENT_NAME),NULL,&g_SecParams,NULL);

    - all set: ```Project```->```Build``` and ```Run```->```Debug```

        This should result in a CC3200 connected to Xively Services.




## Step 7 of 8: Flash your client application onto the device.

By default Code Composer uploads your application into RAM for execution. This is great for quick iterations, but it also means that your device will lose your changes when you uplug it.

To permanently make changes to the device you must flash the device using a Windows binary executable called Uniflash. This tool is external to Code Composer Studio™.

### Download and Install Code Composer Studio™ Uniflash Software

* From [Code Composer Studio™ Uniflash download page](http://processors.wiki.ti.com/index.php/CCS_Uniflash_v3.4.1_Release_Notes) choose Windows Offline Version
* Begin the installation process
* On the "Select Components" window
    * Please leave only ```Simplelink WiFi CC31xx/CC32xx``` the selected and continue installation process

### Run CCS Uniflash Software

* Plug in your CC3200 device and make sure that the J15 Jumper is set to ON
* From ```File``` select ```New Configuration``` and select
    * Connection: ```CC3x Serial(UART) Interface```  
    * Board or Device: ```SimpleLink WiFi CC3100/CC3200```
* On the left panel under the ```System Files``` please highlight the ```/sys/mcuimg.bin```file
* From the right panel press the ```Browse``` button right next to the ```Url``` field
* Pick the ```name_of_your_project.bin``` from your ```workspace_name/project_name/RELEASE/```
* From the left panel hightlight ```CC31xx/CC32xx Flash Setup and Control```  
* Press ```Program``` button
* Set the J15 jumper to OFF and restart your device it should now run the test program


## Congratulations!

(todo)

You did great! 
Here's what the end result should look like.


## What to do next.

(todo)

