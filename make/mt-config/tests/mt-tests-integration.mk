# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-config/tests/mt-tests.mk

XI_ITESTS_OBJDIR := $(XI_TEST_OBJDIR)/itests
XI_ITESTS_CFLAGS = $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS)

# CMOCKA LIBRARY SECTION
CMOCKA_DIR := $(LIBXIVELY)/src/import/cmocka
CMOCKA_BUILD_DIR := $(CMOCKA_DIR)/cmocka_build/$(XI_CONST_PLATFORM_CURRENT)
CMOCKA_MAKEFILE := $(CMOCKA_BUILD_DIR)/Makefile
CMOCKA_LIBRARY := $(CMOCKA_BUILD_DIR)/src/libcmocka.a
CMOCKA_LIBRARY_DEPS := $(CMOCKA_LIBRARY)
CMOCKA_INCLUDE_DIR := $(CMOCKA_DIR)/include/

# add cmocka library
XI_MOCK_LIB_DIR := $(CMOCKA_BUILD_DIR)/src

$(CMOCKA_BUILD_DIR):
	@mkdir -p $(CMOCKA_BUILD_DIR)

ifeq ($(XI_CONST_PLATFORM_CURRENT),$(XI_CONST_PLATFORM_ARM))
    CMAKE_FLAGS=-DCMAKE_TOOLCHAIN_FILE=$(LIBXIVELY)/$(XI_CONST_PLATFORM_ARM).cmake
endif

$(CMOCKA_MAKEFILE): | $(CMOCKA_BUILD_DIR)
	   cmake -B$(CMOCKA_BUILD_DIR) -H$(CMOCKA_DIR) $(CMAKE_FLAGS) -DWITH_STATIC_LIB=ON

$(CMOCKA_LIBRARY): | $(CMOCKA_MAKEFILE)
	@make -C $(CMOCKA_BUILD_DIR)

# MOCK TEST SECTION
XI_ITESTS_SUITE := xi_itests
XI_ITESTS_SOURCE_DIR := $(XI_TEST_DIR)/itests
XI_ITESTS_SOURCES := $(XI_ITESTS_SOURCE_DIR)/$(XI_ITESTS_SUITE).c
XI_ITESTS := $(XI_TEST_BINDIR)/$(XI_ITESTS_SUITE)

# ADD INTEGRATION TEST CASE FILES
XI_ITESTS_SOURCES += $(wildcard $(XI_ITESTS_SOURCE_DIR)/xi_itest_*.c)
XI_ITESTS_SOURCES += $(wildcard $(XI_ITESTS_SOURCE_DIR)/tools/xi_*.c)
XI_ITESTS_SOURCES += $(wildcard $(XI_ITESTS_SOURCE_DIR)/tools/dummy/*.c)

# ADD INTEGRATION TEST TOOLS AND COMMON FILES
XI_ITESTS_SOURCES += $(wildcard $(XI_TEST_DIR)/*.c)

# ADD dummy io layer
XI_ITESTS_SOURCES += $(wildcard $(LIBXIVELY)/src/libxively/io/dummy/*.c)

ifdef XI_SECURE_FILE_TRANSFER_ENABLED
    XI_ITESTS_SOURCES += $(wildcard $(XI_TEST_DIR)/common/control_topic/*.c)
else
    XI_ITESTS_SOURCES := $(filter-out $(XI_ITESTS_SOURCE_DIR)/xi_itest_sft.c, $(XI_ITESTS_SOURCES))
endif

# removing TLS layer related tests in case TLS is turned off from compilation
ifeq (,$(findstring tls_bsp,$(CONFIG)))
    XI_ITESTS_SOURCES := $(filter-out $(XI_ITESTS_SOURCE_DIR)/xi_itest_tls_layer.c, $(XI_ITESTS_SOURCES))
endif

ifndef XI_GATEWAY_FEATURE_ENABLED
    XI_ITESTS_SOURCES := $(filter-out $(XI_ITESTS_SOURCE_DIR)/xi_itest_gateway.c, $(XI_ITESTS_SOURCES))
endif

XI_ITEST_OBJS := $(filter-out $(XI_ITESTS_SOURCES), $(XI_ITESTS_SOURCES:.c=.o))
XI_ITEST_OBJS := $(subst $(XI_ITESTS_SOURCE_DIR), $(XI_ITESTS_OBJDIR), $(XI_ITEST_OBJS))
XI_ITEST_OBJS := $(subst $(LIBXIVELY)/src, $(XI_OBJDIR), $(XI_ITEST_OBJS))

XI_ITESTS_INCLUDE_FLAGS += -I$(CMOCKA_INCLUDE_DIR)
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/tools
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/common/control_topic
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/itests
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/itests/tools
XI_ITESTS_INCLUDE_FLAGS += -I$(XI_TEST_DIR)/itests/tools/dummy
XI_ITESTS_INCLUDE_FLAGS += -I$(LIBXIVELY)/src/libxively
XI_ITESTS_INCLUDE_FLAGS += $(foreach platformdep,$(XI_PLATFORM_MODULES) \
	,-I$(XI_ITESTS_SOURCE_DIR)/platform/$(XI_PLATFORM_BASE)/$(platformdep))

XI_INCLUDE_FLAGS += $(XI_ITESTS_INCLUDE_FLAGS)

ifdef MAKEFILE_DEBUG
$(info "--mt-tests-intergration-- Platform base: ${XI_PLATFORM_BASE})
endif
