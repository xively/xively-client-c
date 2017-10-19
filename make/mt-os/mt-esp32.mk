# Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk
include make/mt-config/mt-tls-wolfssl-esp32.mk

# The path to CC and AR must be in the environment $PATH as an IDF requirement
CC = xtensa-esp32-elf-gcc
AR = xtensa-esp32-elf-ar

################################
# auto-provide ESP32 SDK #######
################################
ifdef IDF_PATH
ifeq (,$(wildcard $(IDF_PATH)))
    $(info SDK NOT available)

    XI_ESP_IDF_SDK_PATH ?= $(HOME)/Downloads/esp32/esp-idf

$(XI_ESP_IDF_SDK_PATH):
	@-mkdir -p $@
	git clone --recursive -b release/v2.1 https://github.com/espressif/esp-idf.git $(XI_ESP_IDF_SDK_PATH)

XI_BUILD_PRECONDITIONS += $(XI_ESP_IDF_SDK_PATH)

endif
endif

#$(IDF_PATH) This is exported in the shell as as IDF requirement
XI_ESP_IDF_SDK_PATH ?= $(IDF_PATH)

################################
# auto-provide ESP32 toolchain #
################################
XI_ESP32_AVAILABILITY_CHECK_CC := $(shell which $(CC) 2> /dev/null)

ifndef XI_ESP32_AVAILABILITY_CHECK_CC
    $(info CC NOT available)

    CC := $(HOME)/Downloads/esp32/xtensa-esp32-elf/bin/$(CC)
    AR := $(HOME)/Downloads/esp32/xtensa-esp32-elf/bin/$(AR)

ifeq ($(XI_HOST_PLATFORM),Darwin)
    # osx cross-compilation toolchain downloads

    XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE = ~/Downloads/esp32/xtensa-esp32-elf-osx-1.22.0-73-ge28a011-5.2.0.tar.gz

    XI_ESP32_TOOLCHAIN_URL := https://dl.espressif.com/dl/xtensa-esp32-elf-osx-1.22.0-73-ge28a011-5.2.0.tar.gz

endif

XI_BUILD_PRECONDITIONS += $(CC)

$(XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE):
	@echo XI ESP32 BUILD: downloading xtensa esp32 toolchain to file $@
	@-mkdir -p $(dir $@)
	@curl -L -o $@ $(XI_ESP32_TOOLCHAIN_URL)

$(CC): $(XI_ESP32_TOOLCHAIN_DOWNLOAD_FILE)
	@echo XI ESP32 BUILD: extracting xtensa esp32 toolchain to have compiler $(CC)
	@tar -xf $< -C ~/Downloads/esp32
	touch $@

# XI_ESP32_AVAILABILITY_CHECK_CC
endif

##################
# Libxively Config
##################
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

################
# WolfSSL Config
################
XI_CONFIG_FLAGS += -DNO_WRITEV
XI_COMPILER_FLAGS += -DSINGLE_THREADED

#########################
# ESP System Include Dirs
#########################
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/driver/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/heap/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/soc/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/soc/esp32/include
XI_COMPILER_FLAGS += -I$(XI_ESP_IDF_SDK_PATH)/components/esp32/include

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

#TODO: sdkconfig file was generated by the SDK's menuconfig GUI in $(XI_ESP_IDF_SDK_PATH)/make/project.mk
#      We may want to include a pre-built one and get rid of that GUI somehow
XI_COMPILER_FLAGS += -I$(LIBXIVELY)/build/include
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

XI_POST_COMPILE_ACTION =