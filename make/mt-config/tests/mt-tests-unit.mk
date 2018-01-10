# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/tests/mt-tests.mk

XI_UTEST_SUITE ?= xi_utests
XI_UTEST_FIXTURE ?= xi_utest_basic_testcase_frame
XI_UTEST_SOURCE_DIR ?= $(XI_TEST_DIR)/utests
XI_UTEST_OBJDIR := $(XI_TEST_OBJDIR)/utests

XI_UTEST_SUITE_SOURCE := $(XI_UTEST_SOURCE_DIR)/$(XI_UTEST_SUITE).c

XI_UTEST_EXCLUDED :=

ifndef XI_SENML_ENABLED
    XI_UTEST_EXCLUDED += xi_utest_senml.c xi_utest_senml_serialization.c
endif

ifndef XI_MEMORY_LIMITER_ENABLED
    XI_UTEST_EXCLUDED += xi_utest_memory_limiter.c
endif

ifndef XI_CONTROL_TOPIC_ENABLED
    XI_UTEST_EXCLUDED += xi_utest_protobuf_engine.c xi_utest_protobuf_endianess.c xi_utest_control_topic.c
endif

ifdef XI_SECURE_FILE_TRANSFER_ENABLED
    XI_UTEST_SOURCES += $(wildcard $(XI_TEST_DIR)/common/control_topic/*.c)
else
    XI_UTEST_EXCLUDED += xi_utest_cbor_codec_ct_decode.c xi_utest_cbor_codec_ct_encode.c xi_utest_control_message_sft.c xi_utest_sft_logic.c xi_utest_sft_logic_internal_methods.c
endif

XI_UTEST_EXCLUDED := $(addprefix $(XI_UTEST_SOURCE_DIR)/, $(XI_UTEST_EXCLUDED))

XI_UTEST_SOURCES += $(wildcard $(XI_UTEST_SOURCE_DIR)/*.c)
XI_UTEST_SOURCES += $(wildcard $(XI_TEST_DIR)/*.c)
XI_UTEST_SOURCES := $(filter-out $(XI_UTEST_EXCLUDED), $(XI_UTEST_SOURCES))
XI_UTEST_SOURCES := $(subst $(XI_UTEST_SUITE_SOURCE),,$(XI_UTEST_SOURCES))
XI_UTEST_OBJS := $(filter-out $(XI_UTEST_SOURCES), $(XI_UTEST_SOURCES:.c=.o))
XI_UTEST_OBJS := $(subst $(XI_UTEST_SOURCE_DIR), $(XI_UTEST_OBJDIR), $(XI_UTEST_OBJS))
XI_UTEST_OBJS := $(subst $(LIBXIVELY)/src, $(XI_OBJDIR), $(XI_UTEST_OBJS))
XI_UTESTS = $(XI_TEST_BINDIR)/$(XI_UTEST_SUITE)

TINY_TEST_OBJ := $(XI_TEST_OBJDIR)/tinytest.o
TINYTEST_SRCDIR ?= $(LIBXIVELY)/src/import/tinytest/

XI_UTEST_INCLUDE_FLAGS += $(XI_INCLUDE_FLAGS)
XI_UTEST_INCLUDE_FLAGS += -I$(TINYTEST_SRCDIR)
XI_UTEST_INCLUDE_FLAGS += -I$(XI_TEST_DIR)
XI_UTEST_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/tools
XI_UTEST_INCLUDE_FLAGS += $(foreach platformdep,$(XI_PLATFORM_MODULES) \
            ,-I$(XI_UTEST_SOURCE_DIR)/platform/$(XI_PLATFORM_BASE)/$(platformdep))

XI_UTEST_CONFIG_FLAGS = $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS)
XI_UTEST_CONFIG_FLAGS += -DNO_FORKING

$(TINY_TEST_OBJ): $(TINYTEST_SRCDIR)/tinytest.c $(XI_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_UTEST_CONFIG_FLAGS) $(XI_UTEST_INCLUDE_FLAGS) -c $< -o $@
