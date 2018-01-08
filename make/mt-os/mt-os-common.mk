# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_COMPILER_FLAGS += -fstrict-aliasing

XI ?= $(XI_BINDIR)/libxively.a

ifneq (,$(findstring release,$(TARGET)))
    XI_COMPILER_FLAGS += -Os
endif

ifneq (,$(findstring debug,$(TARGET)))
    XI_COMPILER_FLAGS += -O0 -g
endif

# warning level
XI_COMPILER_FLAGS += -Wall -Wextra

XI_OBJS := $(filter-out $(XI_SOURCES), $(XI_SOURCES:.c=.o))
XI_OBJS := $(subst $(LIBXIVELY)/src,$(XI_OBJDIR),$(XI_OBJS))
XI_OBJS := $(subst $(XI_BSP_DIR),$(XI_OBJDIR)/bsp/,$(XI_OBJS))

# UNIT TESTS
XI_TEST_OBJS := $(filter-out $(XI_UTEST_SOURCES), $(XI_UTEST_SOURCES:.c=.o))
XI_TEST_OBJS := $(subst $(XI_UTEST_SOURCE_DIR),$(XI_TEST_OBJDIR),$(XI_TEST_OBJS))

XI_POST_COMPILE_ACTION = @$(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)
