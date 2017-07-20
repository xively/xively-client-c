Pre-requisites
==============

1. Set up xtensa-esp32 GCC toolchain: https://esp-idf.readthedocs.io/en/latest/get-started/index.html#setup-toolchain
2. Set up the ESP-IDF SDK: https://esp-idf.readthedocs.io/en/latest/get-started/index.html#get-esp-idf

Build instructions
==================

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
