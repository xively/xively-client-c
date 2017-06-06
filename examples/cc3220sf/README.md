Installing Dependencies & Related Documentation
===============================================

Context:

- Hardware: CC3220SF-LaunchXL
- SDK Version: simplelink_cc32xx_sdk_1_30_01_03
- Computer OS: Windows 10
- CCS Version: 7.0.0.00042
- Compiler: ti-cgt-arm_16.9.0.LTS

Dependencies:

1. Install Code Composer Studio
2. Install the [SimpleLink CC3220 SDK](http://www.ti.com/tool/simplelink-cc3220-sdk)
3. Install [UniFlash v4](http://ti.com/tool/uniflash)

Documentation for similar projects:

- [Compiling CC3200 with Enhanced Security](https://developer.xively.com/v1.0/docs/ti-cc3200-advanced)

Compiling the Firmware With Custom WolfSSL
==========================================

1. Clone xively-client-c into your Windows machine
2. Export paths to gmake in Windows' `cmd`
    ```
    PATH=%PATH%;c:\ti\ccsv7\utils\bin
    PATH=%PATH%;c:\ti\ccsv7\utils\cygwin
    ```
3. [Download WolfSSL source](https://developer.xively.com/v1.0/docs/ti-cc3200-advanced#section-download-wolfssl-library-source).
Follow the steps in that tutorial to unzip and rename the WolfSSL folder
4. Clean the Xively Client C library from `cmd`
    ```
    $ cd $PATH_TO_XIVELY_CLIENT_C
    $ gmake PRESET=CC3220 clean
    ```
5. Build the Xively Client C library from `cmd`
    ```
    $ gmake PRESET=CC3220                                         \
        XI_CC3220_PATH_SDK=C:\ti\simplelink_cc32xx_sdk_1_30_01_03 \
        XI_CC3220_PATH_XDC_SDK=C:\ti\xdctools_3_50_01_12_core     \
        XI_CC3220_PATH_CCS_TOOLS=C:\ti\ccsv7\tools                \
        COMPILER=ti-cgt-arm_16.9.0.LTS                            \
        BSP_DEBUG_LOG=1 XI_DEBUG_OUTPUT=1
    ```
6. Create a Code Composer Studio Workspace
(e.g. `$PATH_TO_XIVELY_CLIENT_C/examples/cc3220/ccs_workspace`)
7. `Project->Import CCS Project...` `$PATH_TO_XIVELY_CLIENT_C/examples/cc3220/xively_demo`
8. `Project->Import CCS Project...` `$PATH_TO_XIVELY_CLIENT_C/examples/cc3220/wolfssl_ccs`
9. `Project->Import CCS Project...` `C:\ti\simplelink_cc32xx_sdk_1_30_01_03\kernel\tirtos\builds\CC3220SF_LAUNCHXL\release\ccs`
10. Update all necessary system paths in the `XivelyExample` and `wolfssl_ccs`
project settings
11. Build the `wolfssl_ccs` project
12. Select the XivelyExample project's `wolfssl_tls_debug` Build Configuration.
The project's default configuration uses the CC3220's on-board TLS
13. Build the `XivelyExample` project

If everything went well, you should have the `XivelyExample.bin` binary you'll
flash into the device.

Remember to clean and re-build the Xively Client C library when switching
between on-board and custom TLS builds.

Compiling the Firmware for On-Board TLS
=======================================

1. Clone xively-client-c into Windows box
2. Export paths to gmake in Windows' `cmd`
    ```
    PATH=%PATH%;c:\ti\ccsv7\utils\bin
    PATH=%PATH%;c:\ti\ccsv7\utils\cygwin
    ```
3. Clean the library from `cmd`
    ```
    $ cd $PATH_TO_XIVELY_CLIENT_C
    $ gmake PRESET=CC3220 clean
    ```
4. If we're not signing the code with a CA-approved signing cert, disable the
3220's certificate catalog in `$LIBXIBELY/make/mt-os/mt-cc3220.mk` uncommenting
this line:
    ```
    XI_CONFIG_FLAGS += -DXI_CC3220_UNSAFELY_DISABLE_CERT_STORE
    ```
    - **SECURITY WARNING**: We're uploading xively's root cert manually, so
that's not a security issue, but disabling the certificate store will also
disable the Cert Revokation List in it. **REMEMBER TO UNDO THIS FOR PRODUCTION!!**
The production Certificate Store does contain Xively's root (as opposed to the
playground), so it won't throw the UNKNOWN_CA error.
5. Compile the Xively Client C library using `cmd`
    ```
    $ gmake PRESET=CC3220_TLS_SOCKET                              \
        XI_CC3220_PATH_SDK=C:\ti\simplelink_cc32xx_sdk_1_30_01_03 \
        XI_CC3220_PATH_XDC_SDK=C:\ti\xdctools_3_50_01_12_core     \
        XI_CC3220_PATH_CCS_TOOLS=C:\ti\ccsv7\tools                \
        COMPILER=ti-cgt-arm_16.9.0.LTS                            \
        BSP_DEBUG_LOG=1 XI_DEBUG_OUTPUT=1
    ```
6. Create CCS Workspace (e.g. `$XI_CLIENT_C/examples/cc3220/ccs_workspace`)
7. `Project->Import CCS Project...` `$XI_CLIENT_C/examples/cc3220/xively_demo`
8. `Project->Import CCS Project...` `C:\ti\simplelink_cc32xx_sdk_1_30_01_03\kernel\tirtos\builds\CC3220SF_LAUNCHXL\release\ccs`
9. Update all necessary system paths in the XivelyExample CCS project settings

If everything went well, you should have the XivelyExample.bin binary you'll
flash into the device.

Remember to clean and re-build the Xively Client C library when switching
between on-board and custom TLS builds.

Flashing the Device
===================

Make sure the `J15` jumper is set to 2 for flashing, and remove it to run the
example.

1. Open Uniflash 4
2. Select the `cc31xx/cc32xx Serial` device
3. `Project Management`
4. Import Uniflash project from ZIP file: `$XI_CLIENT_C/examples/cc3220/uniflash/XivelyProduction_[...].zip`
5. `Service Pack` -> `Browse` -> `$32XX_SDK_PATH/tools/cc32xx_tools/servicepack-cc3x20/sp_3.3.0.0_2.0.0.0_2.2.0.4.bin`
6. `User Files` - Update the firmware image:
    - Delete the existing `/sys/mcuflashimg.bin`
    - `Action` -> `Select MCU Image` -> `$PATH_TO_XIVELY_DEMO/$CCS_BUILD_CONFIG/XivelyExample.bin`
    - Tick the boxes `Failsafe`, `Secure` and `Public Write`
    - `Private Key File Name`: `$PATH_TO_CERTIFICATE_PLAYGROUND/dummy-root-ca-cert-key`
    - `Certification File Name`: `dummy-root-ca-cert`
    - `Save`
6. `User Files` - Update the config file:
    - `Get File` `/etc/xively_cfg.txt`
    - Find the `-etc-xively_cfg.txt` file in your system. Edit it with your credentials
    - `Overwrite File` `/etc/xively_cfg.txt` in Uniflash with your new .txt
7. `Save`
8. `Generate Image`
9. `Program Image`
