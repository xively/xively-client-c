# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

# pick the proper configuration file for TLS library

ifneq ($(XI_USE_EXTERNAL_TLS_LIB),1)
#If using XI_USE_EXTERNAL_TLS_LIB, you'll need to provide XI_TLS_LIB_INC_DIR
XI_TLS_LIB_CONFIG_FNAME ?= make/mt-config/mt-tls-$(XI_BSP_TLS).mk
include $(XI_TLS_LIB_CONFIG_FNAME)

TLS_LIB_PATH := $(LIBXIVELY_SRC)import/tls/$(XI_BSP_TLS)
endif

XI_INCLUDE_FLAGS += -I$(XI_TLS_LIB_INC_DIR)

XI_LIB_FLAGS += $(foreach d, $(XI_TLS_LIB_NAME), -l$d)
XI_LIB_FLAGS += -L$(XI_TLS_LIB_BIN_DIR)

ifneq (,$(findstring Windows,$(XI_HOST_PLATFORM)))
    TLS_LIB_PREPARE_CMD :=
else ifeq ($(XI_USE_EXTERNAL_TLS_LIB),1)
    TLS_LIB_PREPARE_CMD :=
else
    TLS_LIB_PREPARE_CMD := (cd $(LIBXIVELY_SRC)/import/tls/ && ./download_and_compile_$(XI_BSP_TLS).sh)
endif

$(TLS_LIB_PATH):
	$(info ############################)
	$(info # Xively TLS Build Notice: #)
	$(info ############################)
	$(info The Xively Client Build Configuration you're using includes )
	$(info a third party TLS implementation XI_BSP_TLS: $(XI_BSP_TLS) )
	$(info )
	$(info The Xively make system will download and build this TLS library for you. )
	$(info )
	$(info Please review the license information for your TLS library selection below )
	$(info and check make help_tls for more information.)
	$(info )
	$(TLS_LIB_PREPARE_CMD)
