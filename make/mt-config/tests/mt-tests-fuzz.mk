# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/tests/mt-tests.mk

XI_FUZZ_TESTS_BINDIR := $(XI_TEST_BINDIR)/fuzztests
XI_FUZZ_TESTS_OBJDIR := $(XI_TEST_OBJDIR)/fuzztests
XI_FUZZ_TESTS_CFLAGS := $(XI_CONFIG_FLAGS)

XI_FUZZ_TESTS_SOURCE_DIR := $(XI_TEST_DIR)/fuzztests
XI_FUZZ_TESTS_SOURCES := $(wildcard $(XI_FUZZ_TESTS_SOURCE_DIR)/*.cpp)
XI_FUZZ_TESTS := $(foreach fuzztest,$(XI_FUZZ_TESTS_SOURCES),$(notdir $(fuzztest)))
XI_FUZZ_TESTS := $(XI_FUZZ_TESTS:.cpp=)
XI_FUZZ_TESTS_CORPUS_DIR := $(XI_FUZZ_TESTS_SOURCE_DIR)/corpuses
XI_FUZZ_TESTS_CORPUS_DIRS := $(foreach fuzztest, $(XI_FUZZ_TESTS), $(XI_FUZZ_TESTS_CORPUS_DIR)/$(fuzztest))
XI_FUZZ_TESTS := $(foreach fuzztest, $(XI_FUZZ_TESTS), $(XI_FUZZ_TESTS_BINDIR)/$(fuzztest))

XI_FUZZ_TEST_LIBRARY := -lFuzzer

#### =========================================================

XI_CLANG_TOOLS_DIR := $(LIBXIVELY)/src/import/clang_tools

XI_LIBFUZZER_URL := https://llvm.org/svn/llvm-project/compiler-rt/trunk/lib/fuzzer
XI_LIBFUZZER_DOWNLOAD_DIR := $(XI_CLANG_TOOLS_DIR)/downloaded_libfuzzer
XI_LIBFUZZER := $(XI_LIBFUZZER_DOWNLOAD_DIR)/libFuzzer.a

$(XI_LIBFUZZER_DOWNLOAD_DIR):
	@-mkdir -p $(XI_LIBFUZZER_DOWNLOAD_DIR)
	svn checkout $(XI_LIBFUZZER_URL) $(XI_LIBFUZZER_DOWNLOAD_DIR)

$(XI_LIBFUZZER): $(XI_LIBFUZZER_DOWNLOAD_DIR)
	(cd $(XI_LIBFUZZER_DOWNLOAD_DIR) && clang++ -c -g -O2 -lstdc++ -std=c++11 *.cpp -IFuzzer && ar ruv libFuzzer.a Fuzzer*.o)

$(XI_FUZZ_TESTS_CORPUS_DIRS):
	@-mkdir -p $@
