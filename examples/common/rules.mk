# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

CC ?= cc
AR ?= ar

MD ?= @

XI_EXAMPLE_SRCDIR := $(CURDIR)/src
XI_EXAMPLE_OBJDIR := $(CURDIR)/obj
XI_EXAMPLE_BINDIR ?= $(CURDIR)/bin

XI_EXAMPLE_SRCS := common/commandline.c
XI_EXAMPLE_SRCS += $(XI_EXAMPLE_NAME).c

XI_EXAMPLE_DEPS := $(filter-out $(XI_EXAMPLE_SRCS), $(XI_EXAMPLE_SRCS:.c=.d))
XI_EXAMPLE_OBJS := $(filter-out $(XI_EXAMPLE_SRCS), $(XI_EXAMPLE_SRCS:.c=.o))

XI_EXAMPLE_DEPS := $(addprefix $(XI_EXAMPLE_OBJDIR)/,$(XI_EXAMPLE_DEPS))
XI_EXAMPLE_OBJS := $(addprefix $(XI_EXAMPLE_OBJDIR)/,$(XI_EXAMPLE_OBJS))

XI_EXAMPLE_BIN := $(XI_EXAMPLE_BINDIR)/$(XI_EXAMPLE_NAME)

XI_CLIENT_PATH ?= $(CURDIR)/../../
XI_CLIENT_INC_PATH ?= $(CURDIR)/../../include
XI_CLIENT_LIB_PATH ?= $(CURDIR)/../../bin/osx

XI_CLIENT_ROOTCA_LIST := $(CURDIR)/../../res/trusted_RootCA_certs/xi_RootCA_list.pem

XI_FLAGS_INCLUDE += -I$(XI_CLIENT_INC_PATH)
XI_FLAGS_COMPILER ?= -Wall -Werror -Wno-pointer-arith -Wno-format -fstrict-aliasing -O0 -g -Wextra

# TLS BSP related configuration
XI_BSP_TLS ?= wolfssl

ifneq ("$(XI_BSP_TLS)", "")
  # pick the proper configuration file for TLS library
  XI_TLS_LIB_CONFIG_FNAME ?= $(XI_CLIENT_PATH)/make/mt-config/mt-tls-$(XI_BSP_TLS).mk
  include $(XI_TLS_LIB_CONFIG_FNAME)

  TLS_LIB_CONFIG_FLAGS := -L$(addprefix $(XI_CLIENT_PATH),$(XI_TLS_LIB_BIN_DIR))
  TLS_LIB_CONFIG_FLAGS += $(foreach d, $(XI_TLS_LIB_NAME), -l$d)
endif

# -lm is only needed by linux
# -lpthread only if both linux and multithreading is enabled in the Xively C Client at compile time
XI_FLAGS_LINKER := -L$(XI_CLIENT_LIB_PATH)  -lxively -lm -lpthread $(TLS_LIB_CONFIG_FLAGS)
