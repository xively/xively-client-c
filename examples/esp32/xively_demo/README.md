ESP32 Demo - Technical Documentation
====================================
______

##### Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.

This is part of the Xively C Client library, it is licensed under the BSD 3-Clause license.

For more information please read our License.md file located in the base directory of the [Xively C Client repository](https://github.com/xively/xively-client-c).

Pre-requisites
==============

1. Set up xtensa-esp32 GCC toolchain: https://esp-idf.readthedocs.io/en/latest/get-started/index.html#setup-toolchain
2. Set up the ESP-IDF SDK: https://esp-idf.readthedocs.io/en/latest/get-started/index.html#get-esp-idf

WiFi and Xively Credentials Configuration
=========================================

## Provisioning your device over UART [Default]

First of all, you need to connect to your device's serial port after flashing it.
These are the settings it uses:

- 115200 bps
- 8-N-1 bits

If you've never connected your device to Xively before, it will ask you to set
new WiFi and MQTT credentials. Simply follow the instructions presented to you
over the serial terminal session.

If you'd like to change the credentials at any point, you can reboot the device
and press the GPIO0 button at any point while the LED is flashing rapidly. This
will kickstart the provisioning process (ask for your credentials again), and
over-write the stored credentials.

## Using hardcoded credentials [Alternative]

In some cases, you may want to disable the usage of credentials in Non Volatile
Storage (NVS), in favour of credentials hardcoded into the application. You can
do so by enabling the `USE_HARDCODED_CREDENTIALS` macro in `main.c`. You'll also
have to set your credentials in these lines:

```
#define APP_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
#define APP_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
#define APP_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#define APP_WIFI_SSID "[SET YOUR WIFI NETWORK NAME HERE]"
#define APP_WIFI_PASS "[SET YOUR WIFI NETWORK PASSWORD HERE]"
```

Build instructions
==================

Remember to update these in each of the following commands:

- `$(XIVELY_CLIENT_C_PATH)`: Path to this repository's root folder.
- `$(XTENSA_ESP32_ELF_PATH)`: Path to the `xtensa-esp32-elf` directory with the
ESP32 toolchain

1. Download and build the WolfSSL library:
    ```
    cd $(XIVELY_CLIENT_C_PATH)/examples/esp32/xively_demo/wolfssl-make/
    make GCC_XTENSA_TOOLCHAIN_PATH=$(XTENSA_ESP32_ELF_PATH)
    ```
2. Build the MQTT library:
    ```
    cd $(XIVELY_CLIENT_C_PATH)
    make PRESET=ESP32 XI_GCC_XTENSA_TOOLCHAIN_PATH=$(XTENSA_ESP32_ELF_PATH)
    ```
3. Build the ESP32 example:
    ```
    cd $(XIVELY_CLIENT_C_PATH)/examples/esp32/xively_demo/
    make
    # In the build configuration GUI, configure the path to your device's serial
    # port as explained in this tutorial:
    # https://esp-idf.readthedocs.io/en/latest/get-started/index.html#configure
    ```
4. Flash the device:
    ```
    make flash
    ```
