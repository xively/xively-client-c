# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_COMPILER_FLAGS += -fstrict-aliasing

ifneq (,$(findstring release,$(TARGET)))
    XI_COMPILER_FLAGS += -Os
endif

ifneq (,$(findstring debug,$(TARGET)))
    XI_COMPILER_FLAGS += -O0 -g
endif

# warning level
# -Werror
XI_COMPILER_FLAGS += -Wall -Wextra

xi_objects = $(filter-out $(XI_SOURCES), $(XI_SOURCES:.c=.o))
xi_depends = $(filter-out $(XI_SOURCES), $(XI_SOURCES:.c=.d))

XI_OBJS := $(subst $(LIBXIVELY)/src,$(XI_OBJDIR),$(xi_objects))
XI_OBJS := $(subst $(XI_BSP_DIR),$(XI_OBJDIR)/bsp/,$(XI_OBJS))
XI_DEPS = $(addprefix $(XI_OBJDIR)/,$(XI_DEPENDS))

# UNIT TESTS
xi_test_objects = $(filter-out $(XI_UTEST_SOURCES), $(XI_UTEST_SOURCES:.c=.o))
xi_test_depends = $(filter-out $(XI_UTEST_SOURCES), $(XI_UTEST_SOURCES:.c=.d))

XI_TEST_OBJS := $(subst $(XI_UTEST_SOURCE_DIR),$(XI_TEST_OBJDIR),$(xi_test_objects))
XI_TEST_DEPS = $(addprefix $(XI_OBJDIR)/,$(xi_test_depends))
