# Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/tests/mt-tests.mk

XI_TEST_TOOLS_SRCDIR := $(XI_TEST_DIR)/tools
XI_TEST_TOOLS_OBJDIR := $(XI_TEST_OBJDIR)/tools
XI_TEST_TOOLS_BINDIR := $(XI_TEST_BINDIR)/tools

XI_TEST_TOOLS_LIBXIVELY_DRIVER_FILENAME = xi_libxively_driver
XI_TEST_TOOLS_LIBXIVELY_DRIVER = $(XI_TEST_TOOLS_BINDIR)/$(XI_TEST_TOOLS_LIBXIVELY_DRIVER_FILENAME)

XI_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION := \
	$(XI_TEST_TOOLS_SRCDIR)/$(XI_TEST_TOOLS_LIBXIVELY_DRIVER_FILENAME)/$(XI_TEST_TOOLS_LIBXIVELY_DRIVER_FILENAME).c

XI_TEST_TOOLS = $(addprefix $(XI_TEST_TOOLS_BINDIR)/,$(notdir $(XI_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION:.c=)))

XI_TEST_TOOLS_SOURCES_TMP :=  $(wildcard $(XI_TEST_TOOLS_SRCDIR)/xi_libxively_driver/*.c)
XI_TEST_TOOLS_SOURCES_TMP += $(wildcard $(LIBXIVELY)/src/import/protobuf-c/library/*.c)
XI_TEST_TOOLS_SOURCES :=  $(patsubst $(XI_TEST_TOOLS_SOURCES_WITH_MAIN_FUNCTION), , $(XI_TEST_TOOLS_SOURCES_TMP))

XI_TEST_TOOLS_OBJS = $(filter-out $(XI_TEST_TOOLS_SOURCES), $(XI_TEST_TOOLS_SOURCES:.c=.o))
XI_TEST_TOOLS_OBJS := $(subst $(XI_TEST_TOOLS_SRCDIR), $(XI_TEST_TOOLS_OBJDIR), $(XI_TEST_TOOLS_OBJS))
XI_TEST_TOOLS_OBJS := $(subst $(LIBXIVELY)/src, $(XI_OBJDIR), $(XI_TEST_TOOLS_OBJS))

XI_TEST_TOOLS_INCLUDE_FLAGS := -I$(LIBXIVELY_SOURCE_DIR)/../import/protobuf-c/library

