
include make/mt-config/mt-target-platform.mk

# CONFIG presets
CONFIG_POSIX_MAX			=bsp_posix-posix_fs-posix_platform-tls-senml-control_topic-memory_limiter
CONFIG_POSIX_MAX_THREADING  =bsp_posix-posix_fs-posix_platform-tls-senml-control_topic-threading-memory_limiter
CONFIG_POSIX_MID			=bsp_posix-posix_fs-posix_platform-tls-senml-control_topic
CONFIG_POSIX_MID_UNSECURE   =bsp_posix-posix_fs-posix_platform-senml-control_topic
CONFIG_POSIX_MIN			=bsp_posix-posix_fs-posix_platform-tls

CONFIG_POSIX_MAX_NONBSP		=posix_io-posix_fs-posix_platform-tls-senml-control_topic-threading-memory_limiter
CONFIG_POSIX_MIN_NONBSP		=posix_io-posix_fs-posix_platform-tls

# arm configs
CONFIG_DUMMY_MAX			=dummy_io-memory_fs-memory_limiter-control_topic-senml
CONFIG_DUMMY_MIN			=dummy_io-memory_fs

CONFIG_CC3200_MAX			=bsp_cc3200-memory_fs-cc3200-platform-tls-senml-control_topic-memory_limiter
CONFIG_CC3200_MIN			=bsp_cc3200-memory_fs-cc3200-platform-tls

# TARGET presets
TARGET_STATIC_DEV			=-static-debug
TARGET_STATIC_REL			=-static-release

TARGET_ARM_REL				=-static-release

TARGET_CC3200_REL			=-static-release

PRESET ?= POSIX_REL

  $(info )
  $(info -------- )
  $(info >>>> PRESET: $(PRESET))

# -------------------------------------------------------
# NONBSP DEV
ifeq ($(PRESET), POSIX_NONBSP_DEV_MIN)
	CONFIG = $(CONFIG_POSIX_MIN_NONBSP)
	TARGET = $(TARGET_STATIC_DEV)
else ifeq ($(PRESET), POSIX_NONBSP_DEV)
	CONFIG = $(CONFIG_POSIX_MAX_NONBSP)
	TARGET = $(TARGET_STATIC_DEV)

# NONBSP REL
else ifeq ($(PRESET), POSIX_NONBSP_REL_MIN)
	CONFIG = $(CONFIG_POSIX_MIN_NONBSP)
	TARGET = $(TARGET_STATIC_REL)
else ifeq ($(PRESET), POSIX_NONBSP_REL)
	CONFIG = $(CONFIG_POSIX_MAX_NONBSP)
	TARGET = $(TARGET_STATIC_REL)

# -------------------------------------------------------
# BSP DEV
else ifeq ($(PRESET), POSIX_DEV_MIN)
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
	XI_TARGET_PLATFORM = arm-linux
else ifeq ($(PRESET), ARM_REL)
	CONFIG = $(CONFIG_DUMMY_MAX)
	TARGET = $(TARGET_ARM_REL)
	XI_TARGET_PLATFORM = arm-linux

# -------------------------------------------------------
# Texas Instruments CC3200
else ifeq ($(PRESET), CC3200_REL_MIN)
	CONFIG = $(CONFIG_CC3200_MIN)
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
