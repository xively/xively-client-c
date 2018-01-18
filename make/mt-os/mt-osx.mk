# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

CC ?= gcc
AR ?= ar

XI_CONFIG_FLAGS += -DXI_BUILD_OSX
XI_LIB_FLAGS += $(XI_TLS_LIBFLAGS) -lpthread -lm

include make/mt-os/mt-os-common.mk

ifdef XI_SHARED
  XI = $(XI_BINDIR)/libxively.dylib
  XI_ARFLAGS := -shared -o $(XI) $(XI_TLS_LIBFLAGS)
  AR = gcc
  XI_COMPILER_FLAGS += -fPIC
  XI_CONFIG_FLAGS += -DXI_SHARED
else
  XI_ARFLAGS += -rs -c $(XI)
endif

XI_CONFIG_FLAGS += -DXI_MULTI_LEVEL_DIRECTORY_STRUCTURE
