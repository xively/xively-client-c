# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_EXAMPLE_DIR = $(LIBXIVELY)/src/examples
XI_EXAMPLE_OBJDIR = $(XI_OBJDIR)/examples
XI_EXAMPLE_BINDIR = $(XI_BINDIR)/examples

XI_EXAMPLE_SOURCES = internal/xi_coroutine.c

ifdef XI_TLS_BSP
	XI_EXAMPLE_SOURCES += mqtt_logic_example_tls_bsp.c
endif

XI_INTERNAL_EXAMPLES = $(addprefix $(XI_EXAMPLE_BINDIR)/,$(XI_EXAMPLE_SOURCES:.c=))

XI_EXAMPLE_INCLUDE_FLAGS := $(foreach d, $(LIBXIVELY_INTERFACE_INCLUDE_DIRS), -I$d)
