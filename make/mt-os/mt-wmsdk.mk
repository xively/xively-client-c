# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

ifeq ($(WMSDK_PATH), )
$(error You need to give WMSDK_PATH)
endif

# From sdk dir we may need to copy the board.c file required to compile
# example applications
BOARDS_DIR ?= $(WMSDK_PATH)/boards

# set compilers and linkers
CROSS_COMPILE ?= arm-none-eabi
AS    := $(CROSS_COMPILE)-as
CC    := $(CROSS_COMPILE)-gcc
LD    := $(CROSS_COMPILE)-ld
AR    := $(CROSS_COMPILE)-ar
OBJCOPY := $(CROSS_COMPILE)-objcopy

# pick the name of the directory with proper tool
ifeq ($(XI_HOST_PLATFORM),Linux)
    WMSDK_BIN = Linux
endif
ifeq ($(XI_HOST_PLATFORM),Windows_NT)
    WMSDK_BIN = Windows
endif
ifeq ($(XI_HOST_PLATFORM),Darwin)
    WMSDK_BIN = Mac
endif

# detect and export the board file
ifndef BOARD
ifndef S_BOARD
export BOARD ?= mc200_8801
else
export BOARD ?= $(S_BOARD)
endif
endif

ifeq ($(BOARD_FILE), )
BOARD_FILE := $(abspath $(BOARDS_DIR)/$(BOARD).c)
else
BOARD=`basename $(BOARD_FILE) .c`
endif

export BOARD_FILE

# tools required for building binary image and file system
export AXF2FW ?= $(WMSDK_PATH)/tools/bin/$(WMSDK_BIN)/axf2firmware # yes this has to be adjusted to other operating systems
export MKFTFS ?= $(WMSDK_PATH)/tools/bin/flash_pack.py

# Compiler Include Flags
#
#   -- Include autoconf.h -- the automatically generated header file
#      generated from the SDK configuration. The application code can
#      check for included SDK features.
#   -- Enable GCC to search for header files in all of the SDK
#      include directories
#
#  These flags get included in XI_COMPILER_FLAGS passed to the compiler.

XI_INCLUDE_FLAGS += -include $(WMSDK_PATH)/incl/autoconf.h \

XI_INCLUDE_FLAGS += \
      -I $(WMSDK_PATH)/incl \
      -I $(WMSDK_PATH)/incl/freertos \
      -I $(WMSDK_PATH)/incl/libc \
      -I $(WMSDK_PATH)/incl/lwip \
      -I $(WMSDK_PATH)/incl/lwip/ipv4 \
      -I $(WMSDK_PATH)/incl/lwip/ipv6 \
      -I $(WMSDK_PATH)/incl/platform/arch \
      -I $(WMSDK_PATH)/incl/platform/os/freertos \
      -I $(WMSDK_PATH)/incl/platform/net/lwip \
      -I $(WMSDK_PATH)/incl/sdk \
      -I $(WMSDK_PATH)/incl/sdk/drivers \
      -I $(WMSDK_PATH)/incl/sdk/drivers/mc200 \
      -I $(WMSDK_PATH)/incl/sdk/drivers/mc200/regs \
      -I $(WMSDK_PATH)/incl/sdk/drivers/wlan \
      -I $(WMSDK_PATH)/incl/sdk/drivers/wlan/mlan \
      -I $(WMSDK_PATH)/incl/cyassl \
      -I $(WMSDK_PATH)/incl/cyassl/ctaocrypt

XI_COMPILER_FLAGS += -Wall -MMD \
          -ffunction-sections -fdata-sections \
          -fno-common -ffreestanding \
          -mcpu=cortex-m3 -mthumb

XI_CONFIG_FLAGS += -DXI_DEBUG_PRINTF=wmprintf

XI_LIB_FLAGS += $(XI_TLS_LIBFLAGS) -lpthread

include make/mt-os/mt-os-common.mk

$(warning "Unit tests for wmsdk disabled. They will be re-enabled soon by the XCL-842")
XI_TEST_OBJS :=
$(warning "Integration tests for wmsdk disabled. No story in the backlog!")

ifdef XI_SHARED
$(error "No shared target for wmsdk!")
else
  XI_ARFLAGS += -rs
  XI_ARFLAGS += $(XI)
endif
