# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk

XI_COMPILER_FLAGS += -fPIC
XI_COMPILER_FLAGS += -mcpu=cortex-m3 -mthumb
# Temporarily disable these warnings until the code gets changed.
XI_COMPILER_FLAGS += -Wno-format
XI_COMPILER_FLAGS  += -specs=rdimon.specs
XI_LIBS_FLAGS += lrdimon

XI_ARFLAGS += -rs -c $(XI)

include make/mt-utils/mt-get-gnu-arm-toolchain.mk
