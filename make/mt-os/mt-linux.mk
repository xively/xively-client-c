# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

CC ?= gcc
AR ?= ar

XI_COMPILER_FLAGS += -fPIC
XI_LIB_FLAGS += $(XI_TLS_LIBFLAGS) -lpthread -lm

include make/mt-os/mt-os-common.mk

ifdef XI_SHARED
  AR = gcc
  XI = $(XI_BINDIR)/libxively.so
  XI_ARFLAGS += -fPIC -DXI_SHARED -shared -o $(XI)
else
  XI_ARFLAGS += -rs -c $(XI)
endif

# Temporarily disable these warnings until the code gets changed.
XI_COMPILER_FLAGS += -Wno-format
XI_CONFIG_FLAGS += -DXI_MULTI_LEVEL_DIRECTORY_STRUCTURE
