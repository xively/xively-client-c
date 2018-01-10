# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk

# The path to CC and AR must be in the environment $PATH as an IDF requirement
CC = xtensa-esp32-elf-gcc
AR = xtensa-esp32-elf-ar

XI_ESP32_PREREQUISITE_DOWNLOAD_PATH := $(HOME)/Downloads/esp32

################################
# auto-provide ESP32 SDK #######
################################
ifeq (,$(wildcard $(IDF_PATH)))
    $(info NOTE: ESP32 SDK was NOT found, using auto-downloaded ESP32 SDK from)
    $(info .     $(XI_ESP32_PREREQUISITE_DOWNLOAD_PATH))

    XI_ESP_IDF_SDK_PATH := $(XI_ESP32_PREREQUISITE_DOWNLOAD_PATH)/esp-idf

$(XI_ESP_IDF_SDK_PATH):
	@-mkdir -p $@
	git clone --recursive https://github.com/espressif/esp-idf.git $(XI_ESP_IDF_SDK_PATH)
	@-cd $(XI_ESP_IDF_SDK_PATH) && git checkout -b xively_tested_version e6afe28 && git submodule update

XI_BUILD_PRECONDITIONS += $(XI_ESP_IDF_SDK_PATH)

XI_ESP32_POST_BUILD_ACTION_SDK := XI_ESP32_POST_BUILD_ACTION_SDK
XI_ESP32_POST_BUILD_ACTION_SDK:
	$(info NOTE: The environment variable IDF_PATH was not found in your system,)
	$(info .     so we`ve downloaded the SDK for you. You can find it in directory)
	$(info .     $(XI_ESP_IDF_SDK_PATH). You`ll need to export it to your environment)
	$(info .     using this command before compiling your application:)
	$(info .         'export IDF_PATH=$(XI_ESP_IDF_SDK_PATH)')
	$(info .)

else

#$(IDF_PATH) This is exported in the shell as as IDF requirement
XI_ESP_IDF_SDK_PATH ?= $(IDF_PATH)

endif

################################
# auto-provide ESP32 toolchain #
################################
XI_ESP32_AVAILABILITY_CHECK_CC := $(shell which $(CC) 2> /dev/null)

ifndef XI_ESP32_AVAILABILITY_CHECK_CC
    $(info NOTE: ESP32 compiler was NOT found, using auto-downloaded ESP32 toolchain from)
    $(info .     $(XI_ESP32_PREREQUISITE_DOWNLOAD_PATH))

    XI_ESP32_TOOLCHAIN_BIN := $(XI_ESP32_PREREQUISITE_DOWNLOAD_PATH)/xtensa-esp32-elf/bin

    CC := $(XI_ESP32_TOOLCHAIN_BIN)/$(CC)
    AR := $(XI_ESP32_TOOLCHAIN_BIN)/$(AR)

ifeq ($(XI_HOST_PLATFORM),Darwin)
    # osx cross-compilation toolchain downloads

    XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE := $(HOME)/Downloads/esp32/xtensa-esp32-elf-osx-1.22.0-73-ge28a011-5.2.0.tar.gz

    XI_ESP32_TOOLCHAIN_URL := https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-73-ge28a011-5.2.0.tar.gz

else ifeq ($(XI_HOST_PLATFORM),Linux)

    XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE := $(HOME)/Downloads/esp32/xtensa-esp32-elf-linux64-1.22.0-73-ge28a011-5.2.0.tar.gz

    XI_ESP32_TOOLCHAIN_URL := https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-73-ge28a011-5.2.0.tar.gz

endif

XI_BUILD_PRECONDITIONS += $(CC)

$(XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE):
	@echo XI ESP32 BUILD: downloading xtensa esp32 toolchain to file $@
	@-mkdir -p $(dir $@)
	@curl -L -o $@ $(XI_ESP32_TOOLCHAIN_URL)

$(CC): $(XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE)
	@echo XI ESP32 BUILD: extracting xtensa esp32 toolchain to have compiler $(CC)
	@tar -xf $< -C $(HOME)/Downloads/esp32
	touch $@

XI_ESP32_POST_BUILD_ACTION_TOOLCHAIN := XI_ESP32_POST_BUILD_ACTION_TOOLCHAIN
XI_ESP32_POST_BUILD_ACTION_TOOLCHAIN:
	$(info NOTE: The ESP32 compiler `$(notdir $(CC))` was not found in your system PATH,)
	$(info .     so we`ve downloaded the toolchain for you. You can find it in directory)
	$(info .     $(XI_ESP32_TOOLCHAIN_BIN). You`ll need to add this directory to you PATH)
	$(info .     variable using this command before compiling your application:)
	$(info .         `export PATH=$$PATH:$(XI_ESP32_TOOLCHAIN_BIN)`)
	$(info .)

# XI_ESP32_AVAILABILITY_CHECK_CC
endif

# Hint for custom TLS library build: for wolfSSL the XI_BSP_TLS is equal to 'wolfssl'.
# You can find the corresponding file in directory make/mt-config. To build custom TLS
# library as part of the libxively build create your own config file there, e.g.:
# mt-tls-mbedtls-esp32.mk. Or delete the line below to turn off TLS lib cross compilation.
include make/mt-config/mt-tls-$(XI_BSP_TLS)-esp32.mk

##################
# Libxively Config
##################
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

#########################
# ESP System Include Dirs
#########################
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/driver/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/heap/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/soc/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/soc/esp32/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/esp32/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/app_update/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/spi_flash/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/nvs_flash/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/tcpip_adapter/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/wear_levelling/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/vfs/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/fatfs/src
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/log/include

###################
# LWIP Include Dirs
###################
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/lwip/system
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/lwip/include/lwip
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/lwip/include/lwip/port

#######################
# FreeRTOS Include Dirs
#######################
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/freertos/include/freertos
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/freertos/include

XI_COMPILER_FLAGS += -I$(LIBXIVELY)/build/include
XI_COMPILER_FLAGS += -I$(LIBXIVELY)/src/bsp/platform/esp32
XI_COMPILER_FLAGS += -I$(LIBXIVELY)/src/bsp/platform/esp32/include

####################
# Code configuration
####################
XI_COMPILER_FLAGS += -DSNTP_MAX_SERVERS=4 #if modified, update your app's component.mk and xi_bsp_time_esp32_sntp.c too

################################
# xtensa-esp32 toolchain options
################################
XI_COMPILER_FLAGS += -fstrict-volatile-bitfields
XI_COMPILER_FLAGS += -ffunction-sections
XI_COMPILER_FLAGS += -fdata-sections
XI_COMPILER_FLAGS += -mlongcalls
XI_COMPILER_FLAGS += -nostdlib
XI_COMPILER_FLAGS += -ggdb
XI_COMPILER_FLAGS += -Os
XI_COMPILER_FLAGS += -DNDEBUG
XI_COMPILER_FLAGS += -std=gnu99
XI_COMPILER_FLAGS += -Wno-old-style-declaration
XI_COMPILER_FLAGS += -Wno-unused-but-set-variable

XI_ARFLAGS += -rs -c $(XI)


#ifdef XI_TRAVIS_BUILD
#### TOOLCHAIN AUTODOWNLOAD SECTION --- BEGIN
#
#	XI_BUILD_PRECONDITIONS += ESP32_SDK
#.PHONY : ESP32_SDK
#ESP32_SDK:
#	git clone -b esp32 git@github.com:xively/xively-client-artifactory.git $(HOME)/Downloads/xively-client-artifactory
#
#### TOOLCHAIN AUTODOWNLOAD SECTION --- END
#endif

#endif

XI_POST_BUILD_ACTION := XI_ESP32_POST_BUILD_ACTION

$(XI_POST_BUILD_ACTION): $(XI_ESP32_POST_BUILD_ACTION_SDK) $(XI_ESP32_POST_BUILD_ACTION_TOOLCHAIN)

XI_POST_COMPILE_ACTION =
