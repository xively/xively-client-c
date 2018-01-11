# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_TEST_DIR = $(LIBXIVELY)/src/tests
XI_TEST_OBJDIR := $(XI_OBJDIR)/tests
XI_TEST_BINDIR := $(XI_BINDIR)/tests

ifneq (,$(findstring arm,$(TARGET)))
	include make/mt-config/tests/mt-qemu-cortex-m3.mk
else
	include make/mt-config/tests/mt-native.mk
endif
