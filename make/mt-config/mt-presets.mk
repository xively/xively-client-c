# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/mt-target-platform.mk

# CONFIG presets
CONFIG_POSIX_MAX                =posix_fs-posix_platform-tls_bsp-senml-control_topic-memory_limiter
CONFIG_POSIX_MAX_THREADING      =posix_fs-posix_platform-tls_bsp-senml-control_topic-threading-memory_limiter
CONFIG_POSIX_MID                =posix_fs-posix_platform-tls_bsp-senml-control_topic
CONFIG_POSIX_MID_UNSECURE       =posix_fs-posix_platform-senml-control_topic
CONFIG_POSIX_MIN                =posix_fs-posix_platform-tls_bsp
CONFIG_POSIX_MIN_UNSECURE       =posix_fs-posix_platform

# arm configs
CONFIG_DUMMY_MAX                =memory_fs-memory_limiter-control_topic-senml
CONFIG_DUMMY_MIN                =memory_fs

CONFIG_CC3200                   =memory_fs-tls_bsp
CONFIG_CC3200_TLS_SOCKET        =memory_fs-tls_socket

CONFIG_CC3220SF                   =bsp_cc3220sf-memory_fs-tls_bsp
CONFIG_CC3220SF_TLS_SOCKET        =bsp_cc3220sf-memory_fs-tls_socket

CONFIG_STM32FX                  =memory_fs-tls_bsp
CONFIG_STM32FX_NUCLEO_WIFI      =memory_fs-tls_socket

# TARGET presets
TARGET_STATIC_DEV               =-static-debug
TARGET_STATIC_REL               =-static-release

TARGET_ARM_REL                  =-static-release

PRESET ?= POSIX_REL

# -------------------------------------------------------
# BSP DEV
ifeq ($(PRESET), POSIX_DEV_MIN)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_DEV)
    XI_BSP_PLATFORM = posix
else ifeq ($(PRESET), POSIX_DEV)
    CONFIG = $(CONFIG_POSIX_MAX)
    TARGET = $(TARGET_STATIC_DEV)
    XI_BSP_PLATFORM = posix

# BSP REL
else ifeq ($(PRESET), POSIX_REL_MIN)
    CONFIG = $(CONFIG_POSIX_MIN)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = posix
else ifeq ($(PRESET), POSIX_REL)
    CONFIG = $(CONFIG_POSIX_MAX)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = posix

# -------------------------------------------------------
# UNSECURE
else ifeq ($(PRESET), POSIX_UNSECURE_REL)
    CONFIG = $(CONFIG_POSIX_MID_UNSECURE)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = posix
    XI_BSP_TLS =

# + THREADING
else ifeq ($(PRESET), POSIX_THREADING_REL)
    CONFIG = $(CONFIG_POSIX_MAX_THREADING)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = posix

# -------------------------------------------------------
# ARM
else ifeq ($(PRESET), ARM_REL_MIN)
    CONFIG = $(CONFIG_DUMMY_MIN)
    TARGET = $(TARGET_ARM_REL)
    XI_BSP_PLATFORM = dummy
    XI_TARGET_PLATFORM = arm-linux
else ifeq ($(PRESET), ARM_REL)
    CONFIG = $(CONFIG_DUMMY_MAX)
    TARGET = $(TARGET_ARM_REL)
    XI_BSP_PLATFORM = dummy
    XI_TARGET_PLATFORM = arm-linux

# -------------------------------------------------------
# Texas Instruments CC3200
else ifeq ($(PRESET), CC3200)
    CONFIG = $(CONFIG_CC3200)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = cc3200
    XI_TARGET_PLATFORM = cc3200
else ifeq ($(PRESET), CC3200_SDK120)
    CONFIG = $(CONFIG_CC3200)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = cc3200
    XI_TARGET_PLATFORM = cc3200
    XI_CC3200_SDK = CC3200SDK_1.2.0
else ifeq ($(PRESET), CC3200_TLS_SOCKET)
    CONFIG = $(CONFIG_CC3200_TLS_SOCKET)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = cc3200
    XI_TARGET_PLATFORM = cc3200

# -------------------------------------------------------
# Texas Instruments CC3220SF
else ifeq ($(PRESET), CC3220SF)
    CONFIG = $(CONFIG_CC3220SF)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = cc3220sf
    XI_TARGET_PLATFORM = cc3220sf
else ifeq ($(PRESET), CC3220SF_TLS_SOCKET)
    CONFIG = $(CONFIG_CC3220SF_TLS_SOCKET)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = cc3220sf
    XI_TARGET_PLATFORM = cc3220sf


# -------------------------------------------------------
# ST Micro STM32FX
else ifeq ($(PRESET), STM32FX)
    CONFIG = $(CONFIG_STM32FX)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = stm32fx
    XI_TARGET_PLATFORM = stm32fx

# ST Micro STM32FX_NUCLEO_WIFI
else ifeq ($(PRESET), STM32FX_NUCLEO_WIFI)
    CONFIG = $(CONFIG_STM32FX_NUCLEO_WIFI)
    TARGET = $(TARGET_STATIC_REL)
    XI_BSP_PLATFORM = stm32fx_nucleo_wifi
    XI_TARGET_PLATFORM = stm32fx_nucleo_wifi

# -------------------------------------------------------
# Fuzz Tests
else ifeq ($(PRESET), FUZZ_TESTS)
	ifeq ($(XI_HOST_PLATFORM),Darwin)
$(error Fuzz testing won\'t work on OSX)
	endif
	CONFIG = $(CONFIG_POSIX_MIN_UNSECURE)_fuzz_test
	TARGET = $(TARGET_STATIC_REL)
	XI_BSP_PLATFORM = posix
	XI_BSP_TLS =

# -------------------------------------------------------
# DEFAULT
else
    ifndef PRESET
    # default settings in case of undefined
    CONFIG ?= $(CONFIG_POSIX_MIN)
    TARGET ?= $(TARGET_STATIC_REL)
  	    $(info INFO: '$(PRESET)' not detected, using default CONFIG: [$(CONFIG)] and TARGET: [$(TARGET)])
    else
    # error in case of unrecognised PRESET
    $(error Invalid PRESET, see valid presets in make/mt-config/mt-presets.mk)
    endif
endif

TARGET := $(addprefix $(XI_TARGET_PLATFORM), $(TARGET))

preset_output:
    $(info )
    $(info # Using build PRESET: "$(PRESET)" to set CONFIG and TARGET variables:)
    $(info .    XI_BSP_PLATFORM: [$(XI_BSP_PLATFORM)] )
    $(info .    XI_BSP_TLS:      [$(XI_BSP_TLS)] )
