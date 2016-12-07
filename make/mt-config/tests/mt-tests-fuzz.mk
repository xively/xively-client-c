# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_FUZZ_TESTS_BINDIR := $(XI_TEST_BINDIR)/fuzztets
XI_FUZZ_TESTS_OBJDIR := $(XI_TEST_OBJDIR)/fuzztests
XI_FUZZ_TESTS_CFLAGS := $(XI_CONFIG_FLAGS)

XI_FUZZ_TESTS := xi_fuzztests xi_fuzztest_mqtt_layer
XI_FUZZ_TESTS_SOURCE_DIR := $(XI_TEST_DIR)/fuzztests
XI_FUZZ_TESTS_CORPUS_DIR := $(XI_FUZZ_TESTS_SOURCE_DIR)/corpuses
XI_FUZZ_TESTS_SOURCES := $(foreach fuzztest, $(XI_FUZZ_TESTS), $(XI_FUZZ_TESTS_SOURCE_DIR)/$(fuzztest).cpp)

XI_FUZZ_TESTS := $(foreach fuzztest, $(XI_FUZZ_TESTS), $(XI_FUZZ_TESTS_BINDIR)/$(fuzztest))
XI_FUZZ_TESTS_OBJS := $(XI_FUZZ_TESTS_SOURCES:.cpp=.cpp.o)
XI_FUZZ_TESTS_OBJS := $(subst $(XI_FUZZ_TESTS_SOURCE_DIR), $(XI_FUZZ_TESTS_OBJDIR), $(XI_FUZZ_TESTS_OBJS))
XI_FUZZ_TESTS_OBJS := $(subst $(LIBXIVELY)/src, $(XI_OBJDIR), $(XI_FUZZ_TESTS_OBJS))

XI_FUZZ_TEST_LIBRARY := -lFuzzer

include make/mt-config/tests/mt-native

