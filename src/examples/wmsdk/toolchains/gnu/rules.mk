# Copyright (C) 2008-2012 Marvell International Ltd.
# All Rights Reserved.
#
# Application Makefile Common Rules:
#
#   This Makefile fragment provides rules for the following:
#
#   1. Compiling C-source files (.c) into Object files (.o)
#   2. Linking application object files with SDK libraries to create
#      application firmware image in ELF format (.axf suffix)
#   3. Converting ELF firmware image into .bin format that is expected
#      by boot2 when booting an application firmware.
#
# Usage:
#
#   Include this Makefile fragment in your main application Makefile.
#
#   The Application Makefile should set the following variables
#   dependent on the application.
#
#   SRCS:      list of source files. Assumed to be all in $(SRC_DIR)
#   LDSCRIPT:  linker file
#   SDK_PATH:  installed (absolute) location of the pre-built SDK
#
#   The following variables can be used to customize the builds
#
#   EXTRACFLAGS: specify additional flags to be passed to C Compiler
#   EXTRALIBS:   specify additional libs  to be passed to the Linker
#
#   Default Variables:
#
#   The following variables can be overridden by the main Makefile.
#
#   SRC_DIR:  (default src)
#		-- directory for source files (relative to main app dir)
#   OBJ_DIR:  (default obj)
#		-- directory for object files (relative to main app dir)
#   BIN_DIR:  (default bin)
#		-- directory for final build artifacts (relative to main app dir)
#   MAP_FILE:
#		-- name of output map file
#		-- default = $(DST_NAME).map if $(DST_NAME) is set else map.txt
#

.NOTPARALLEL:

SRC_DIR ?= src

ifeq ($(DST_NAME),)
MAP_FILE ?= map.txt
else
MAP_FILE ?= $(DST_NAME).map
endif

LDSCRIPT ?= mc200.ld

# FIXME: Review and Fix
#FTFS_API_VERSION ?= 114

# CROSS COMPILER TOOLCHAIN SETTINGS
#
# The following settings apply to GNU Cross-Compiler Toolchain from
# Code-Sourcery.

CROSS_COMPILE ?= arm-none-eabi
AS    := $(CROSS_COMPILE)-as
CC    := $(CROSS_COMPILE)-gcc
LD    := $(CROSS_COMPILE)-ld
AR    := $(CROSS_COMPILE)-ar
OBJCOPY := $(CROSS_COMPILE)-objcopy
COPY_CMD := cp

INC_FLAGS += $(XI_INCLUDE_FLAGS)

# C Compiler Flags
#
# The following CFLAGS are passed to the C-Compiler
#
#   -Wall:	Turn on all warnings
#   -g:         Produce Debugging Information
#   -Os:        Optimize for size
#   -MMD:
#		Generate dependency output file (.d) as part of compilation.
#   -ffunction-sections:
#   -fdata-sections:
#	  	Place each function or data item into its own section in the
#           	output file. This enables flexible mapping of application
#           	code and data to system memory through linker script.
#   -fno-common:
#   		The -fno-common option specifies that the compiler should
#   		place uninitialized global variables in the data section of
#   		the object file, rather than generating them as common
#   		blocks. This has the effect that if the same variable is
#   		declared (without "extern") in two different compilations,
#   		you will get a multiple-definition error when you link them.
#   -ffreestanding:
#		Compilation takes place in a freestanding environment. This
#		implies that the standard library may not exist and the
#       	program startup may not be at main. Equivalent to -fno-hosted.
#
#   -mcpu=cortex-m3
#   		Generate code for Cortex-M3 CPU
#   -mthumb
#   		Use Thumb instruction set
#   -fno-strict-aliasing:
#		Prevent compiler from optimizing by assuming strict aliasing rules.
#       See: http://www.mail-archive.com/linux-btrfs@vger.kernel.org/msg01647.html

CFLAGS += $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS)

ifeq ($(CONFIG_ENABLE_LTO), y)
 CFLAGS += -flto -ffat-lto-objects
endif

# Linker Flags
#
# The following Linker flags are passed to the GNU Linker
#
#  -nostartfiles:
#    Do not use the standard system startup files when linking
#  -M -Map=<mapfile>
#    Print the link map to the specified <mapfile>
#  --gc-sections:
#		Enable garbage collection of unused input sections.
#       Note: any symbols used by dynamic objects are kept.
#  --start-group <SDK_LIBS> --end-group
# 		All of the SDK libraries are included as a group as
#		it allows the linker to resolve cross-references between
#		the SDK libraries.
#
#  Note: The linker is invoked indirectly via the C Compiler.
#        -Xlinker is used to pass options the linker that is not
#		 recognized by the C compiler.
#
#  FIXME: Note on C-Library:
#
LDFLAGS = -T $(LDSCRIPT) -nostartfiles \
	  -Xlinker -M -Xlinker -Map=$(BIN_DIR)/$(MAP_FILE) \
	  -Xlinker --gc-sections

CFLAGS += $(INC_FLAGS) $(EXTRACFLAGS)

ifeq ($(CONFIG_ENABLE_LTO), y)
 LDFLAGS += -Xlinker -flto
endif

LIBDIR = $(WMSDK_PATH)/libs
LIBS = $(subst $(LIBDIR)/,,$(wildcard $(LIBDIR)/*.a))
LIBS_F = $(addprefix $(LIBDIR)/,$(LIBS))

STARTGRP_WITH_LIBS = -Xlinker --start-group $(LIBS_F) $(EXTRALIBS)
ENDGRP = -Xlinker --end-group

OBJ_LIST = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

# --------------- Common Functions --------------------
OS := $(shell uname)

SHELL := /bin/bash
ifneq ($(OS), Linux)
export CYGPATH=cygpath
endif

#  Implicit Rules:
#
#   1. Compile a C-Source (.c) file into Object File (.o)
#		-- The source files must be in $(SRC_DIR)
#		-- The object file is created in $(OBJ_DIR)
#   2. Link the sources with SDK libraries to create an ELF image (.axf)
#   3. Convert the ELF Image to Firmware Binary Format for Flashing (.bin)
#   4. Create an FTFS Image from a directory of files
#
#   The build rules are set for Code Sourcery Compiler Toolchain and
#   have the necessary flags to build for 88MC200 (Cortex-M3) CPU.

.SECONDARY: $(OBJ_LIST) $(BIN_DIR)/%.axf $(BIN_DIR)/%.bin

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(AT)echo " [cc] $< $@"
	$(AT)$(CC) -c -o $@ $(CFLAGS) $(EXTRACFLAGS) $<

$(BIN_DIR)/%.axf: $(OBJ_LIST)
	$(AT)$(CC) $(CFLAGS) $(LDFLAGS) $(STARTGRP_WITH_LIBS) $(OBJ_LIST) $(ENDGRP) -o $@
	$(AT)if [ -e $@ ]; then echo " [axf] $@"; fi

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.axf
	$(AT)cd $(BIN_DIR); $(AXF2FW) $< $@
	$(AT)if [ -e $@ ]; then echo " [bin] $@"; fi

$(BIN_DIR)/%.ftfs:
	$(MKFTFS) $(FTFS_API_VERSION) $@ $(FTFS_DIR)
	$(AT)if [ -e $@ ]; then echo " [ftfs] $@"; fi

