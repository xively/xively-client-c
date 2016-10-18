# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/mt-target-platform.mk

# CONFIG presets
CONFIG_POSIX_MAX			=bsp_posix-posix_fs-posix_platform-tls-senml-control_topic-memory_limiter
CONFIG_POSIX_MAX_THREADING  =bsp_posix-posix_fs-posix_platform-tls-senml-control_topic-threading-memory_limiter
CONFIG_POSIX_MID			=bsp_posix-posix_fs-posix_platform-tls-senml-control_topic
CONFIG_POSIX_MID_UNSECURE   =bsp_posix-posix_fs-posix_platform-senml-control_topic
CONFIG_POSIX_MIN			=bsp_posix-posix_fs-posix_platform-tls

# arm configs
CONFIG_DUMMY_MAX			=bsp_dummy-memory_fs-memory_limiter-control_topic-senml
CONFIG_DUMMY_MIN			=bsp_dummy-memory_fs

CONFIG_CC3200_MAX			=bsp_cc3200-memory_fs-tls-senml-control_topic-memory_limiter
CONFIG_CC3200_MIN			=bsp_cc3200-memory_fs-tls
CONFIG_CC3200_MIN_UNSECURE	=bsp_cc3200-memory_fs

# TARGET presets
TARGET_STATIC_DEV			=-static-debug
TARGET_STATIC_REL			=-static-release

TARGET_ARM_REL				=-static-release

TARGET_CC3200_REL			=-static-release

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
else ifeq ($(PRESET), CC3200_REL_MIN)
	CONFIG = $(CONFIG_CC3200_MIN)
	TARGET = $(TARGET_CC3200_REL)
	XI_BSP_PLATFORM = cc3200
	XI_TARGET_PLATFORM = cc3200
else ifeq ($(PRESET), CC3200_REL_MIN_UNSECURE)
	CONFIG = $(CONFIG_CC3200_MIN_UNSECURE)
	TARGET = $(TARGET_CC3200_REL)
	XI_BSP_PLATFORM = cc3200
	XI_TARGET_PLATFORM = cc3200
else ifeq ($(PRESET), CC3200_REL)
	CONFIG = $(CONFIG_CC3200_MAX)
	TARGET = $(TARGET_CC3200_REL)
	XI_BSP_PLATFORM = cc3200
	XI_TARGET_PLATFORM = cc3200

# -------------------------------------------------------
# DEFAULT
else
# default settings in case of undefined or unrecognised PRESET
CONFIG ?= $(CONFIG_POSIX_MIN)
TARGET ?= $(TARGET_STATIC_REL)
  $(info INFO: PRESET: '$(PRESET)' not recognised, using default CONFIG: [$(CONFIG)] and TARGET: [$(TARGET)])
endif

TARGET := $(addprefix $(XI_TARGET_PLATFORM), $(TARGET))

preset_output:
	$(info )
	$(info # Using build PRESET: "$(PRESET)" to set CONFIG and TARGET variables:)
	$(info .    XI_BSP_PLATFORM: [$(XI_BSP_PLATFORM)] )
	$(info .    XI_BSP_TLS:      [$(XI_BSP_TLS)] )

preset_help:
	$(info # Build Presets #)
	$(info . The Xively C Client has many optional modules and platform targets. )
	$(info . While you can set CONFIG and TARGET flags directly,)
	$(info . there are some prebaked profiles of the Xively C Client)
	$(info . that have been created for quick development.")
	$(info . For instance, "make PRESET=POSIX_DEV" for development on)
	$(info . MacOSX or Linux.)
	$(info )
	$(info . All PRESET configurations reside in make/mt-config/mt-presets.mk.)
	$(info )
	$(info . A List of Popular Valid Presets: )
	$(info .   POSIX_DEV_MIN)
	$(info .   POSIX_DEV)
	$(info .   POSIX_REL_MIN)
	$(info .   POSIX_REL)
	$(info .   POSIX_UNSECURE_REL)
	$(info .   POSIX_THREADING_REL)
	$(info .   CC3200_REL_MIN)
	$(info .   CC3200_REL_MIN_UNSECURE)
	$(info .   CC3200_REL)
	$(info )

