# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_FTEST_MAX_TOTAL_TIME ?= 10
XI_FTEST_MAX_LEN ?= 256

#XI_RUN_UTESTS = $(XI_UTESTS) -l0 --terse
XI_RUN_UTESTS := (cd $(dir $(XI_UTESTS)) && LD_LIBRARY_PATH=$(dir $(XI)):$$LD_LIBRARY_PATH exec $(XI_UTESTS) -l0)
XI_RUN_ITESTS := (cd $(dir $(XI_ITESTS)) && LD_LIBRARY_PATH=$(dir $(XI)):$$LD_LIBRARY_PATH exec $(XI_ITESTS))
XI_RUN_FUZZ_TEST = (cd $(XI_FUZZ_TESTS_BINDIR) && $(1) $(XI_FUZZ_TESTS_CORPUS_DIR)/$(notdir $(1))/ -max_total_time=$(XI_FTEST_MAX_TOTAL_TIME) -max_len=$(XI_FTEST_MAX_LEN));
