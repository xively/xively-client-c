# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_FUZZ_TESTS_OBJDIR := $(XI_TEST_OBJDIR)/fuzztests
XI_FUZZ_TESTS_CFLAGS := $(XI_CONFIG_FLAGS)

XI_FUZZ_TESTS_SUITE := xi_fuzztests
XI_FUZZ_TESTS_SOURCE_DIR := $(XI_TEST_DIR)/fuzztests
XI_FUZZ_TESTS_SOURCES := $(XI_FUZZ_TESTS_SOURCE_DIR)/$(XI_FUZZ_TESTS_SUITE).cpp
XI_FUZZ_TESTS := $(XI_TEST_BINDIR)/$(XI_FUZZ_TESTS_SUITE)

XI_FUZZ_TESTS_OBJS := $(filter-out $(XI_FUZZ_TESTS_SOURCES),$(XI_FUZZ_TESTS_SOURCES:.cpp=.cpp.o))
XI_FUZZ_TESTS_OBJS := $(subst $(XI_FUZZ_TESTS_SOURCE_DIR), $(XI_FUZZ_TESTS_OBJDIR), $(XI_FUZZ_TESTS_OBJS))
XI_FUZZ_TESTS_OBJS := $(subst $(LIBXIVELY)/src, $(XI_OBJDIR), $(XI_FUZZ_TESTS_OBJS))
XI_FUZZ_TEST_LIBRARY := -lFuzzer


