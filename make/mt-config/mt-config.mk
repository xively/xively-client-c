# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

LIBXIVELY_SRC := $(LIBXIVELY)/src/
LIBXIVELY_SOURCE_DIR := $(LIBXIVELY)/src/libxively
LIBXIVELY_INTERFACE_INCLUDE_DIRS := $(LIBXIVELY)/include
LIBXIVELY_INTERFACE_INCLUDE_DIRS += $(LIBXIVELY)/include_senml

XI_LIB_FLAGS += -lxively

ifneq (,$(findstring tls_bsp,$(CONFIG)))
	include make/mt-config/mt-tls.mk
endif

XI_UNIT_TEST_TARGET ?= native

XI_OBJDIR_BASE ?= $(LIBXIVELY)/obj
XI_BINDIR_BASE ?= $(LIBXIVELY)/bin
XI_PROTOBUF_GENERATED ?= $(LIBXIVELY_SOURCE_DIR)/protobuf-generated

XI_PROTO_DIR ?= $(LIBXIVELY)/src/protofiles

# TARGET: Debug Output Options
#
ifneq (,$(findstring release,$(TARGET)))
	XI_DEBUG_OUTPUT ?= 0
	XI_DEBUG_ASSERT ?= 0
	XI_DEBUG_EXTRA_INFO ?=0
endif

XI_DEBUG_PRINTF ?=

ifneq (,$(findstring shared,$(TARGET)))
	XI_SHARED=1
endif

ifneq (,$(findstring backoff_godmode,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_BACKOFF_GODMODE
endif

ifneq (,$(findstring control_topic,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_CONTROL_TOPIC_ENABLED
endif

ifneq (,$(findstring secure_file_transfer,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_SECURE_FILE_TRANSFER_ENABLED
endif

ifneq (,$(findstring expose_fs,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_EXPOSE_FS
endif

ifneq (,$(findstring debug,$(TARGET)))
	XI_DEBUG_OUTPUT ?= 1
	XI_DEBUG_ASSERT ?= 1
	XI_DEBUG_EXTRA_INFO ?= 1
endif

# Settings that will work only on linux and only against clang-4.0 and greater
ifneq (,$(findstring fuzz_test,$(CONFIG)))
    XI_CONFIG_FLAGS += -fsanitize=address -fomit-frame-pointer -fsanitize-coverage=trace-pc-guard -g
endif

XI_COMPILER_FLAGS += -Wall -Werror

# TEMPORARILY disable warnings until the code gets changed
# For all compilers:
XI_COMPILER_FLAGS += -Wno-pointer-arith
ifeq "$(CC)" "clang"
	# For CLANG
	XI_COMPILER_FLAGS += -Wno-gnu-zero-variadic-macro-arguments
else
	# For no CLANG compilers
	XI_COMPILER_FLAGS += -Wno-format
endif

XI_CONFIG_FLAGS += -DXI_DEBUG_OUTPUT=$(XI_DEBUG_OUTPUT)
XI_CONFIG_FLAGS += -DXI_DEBUG_ASSERT=$(XI_DEBUG_ASSERT)
XI_CONFIG_FLAGS += -DXI_DEBUG_EXTRA_INFO=$(XI_DEBUG_EXTRA_INFO)
XI_CONFIG_FLAGS += -DUSE_CBOR_CONTEXT

ifdef XI_CBOR_MESSAGE_MIN_BUFFER_SIZE
XI_CONFIG_FLAGS += -DXI_CBOR_MESSAGE_MIN_BUFFER_SIZE=$(XI_CBOR_MESSAGE_MIN_BUFFER_SIZE)
endif

ifdef XI_CBOR_MESSAGE_MAX_BUFFER_SIZE
XI_CONFIG_FLAGS += -DXI_CBOR_MESSAGE_MAX_BUFFER_SIZE=$(XI_CBOR_MESSAGE_MAX_BUFFER_SIZE)
endif

ifdef XI_SFT_FILE_CHUNK_SIZE
XI_CONFIG_FLAGS += -DXI_SFT_FILE_CHUNK_SIZE=$(XI_SFT_FILE_CHUNK_SIZE)
endif

ifneq (,$(XI_DEBUG_PRINTF))
    XI_CONFIG_FLAGS += -DXI_DEBUG_PRINTF=$(XI_DEBUG_PRINTF)
endif

# CONFIG: mqtt_localhost
#
ifneq (,$(findstring mqtt_localhost,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_MQTT_HOST='{ "localhost", XI_MQTT_PORT }'
endif

# CONFIG: no_certverify
#
ifneq (,$(findstring no_certverify,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_DISABLE_CERTVERIFY
endif

# CONFIG: filesystem
ifneq (,$(findstring dummy_fs,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_FS_DUMMY
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io/fs/dummy
endif
ifneq (,$(findstring memory_fs,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_FS_MEMORY
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io/fs/memory
endif
ifneq (,$(findstring posix_fs,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_FS_POSIX
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io/fs/posix
endif

# DEBUG_EXTENSION: choose debug extensions
ifneq (,$(findstring memory_limiter,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_MEMORY_LIMITER_APPLICATION_MEMORY_LIMIT=524288 # 512 KB
	XI_CONFIG_FLAGS += -DXI_MEMORY_LIMITER_SYSTEM_MEMORY_LIMIT=2024 # 2 KB
	XI_CONFIG_FLAGS += -DXI_MEMORY_LIMITER_ENABLED
	XI_MEMORY_LIMITER_ENABLED := 1
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/debug_extensions/memory_limiter
endif

# CONFIG: modules here we are going to check each defined module

XI_PLATFORM_MODULES ?= xi_thread

# CONFIG: enable threading
ifneq (,$(findstring threading,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_MODULE_THREAD_ENABLED
	XI_PLATFORM_MODULES_ENABLED += xi_thread
endif

# CONFIG: choose modules platform
ifneq (,$(findstring posix_platform,$(CONFIG)))
	XI_PLATFORM_BASE = posix
	XI_CONFIG_FLAGS += -DXI_PLATFORM_BASE_POSIX
else ifneq (,$(findstring wmsdk_platform,$(CONFIG)))
	XI_PLATFORM_BASE = wmsdk
	XI_CONFIG_FLAGS += -DXI_PLATFORM_BASE_WMSDK
else
	XI_PLATFORM_BASE = dummy
	XI_CONFIG_FLAGS += -DXI_PLATFORM_BASE_DUMMY
endif

# CONFIG: BSP related include and source configuration
# enumerate existing platform bsps
BSP_PLATFORM_DIR_EXIST := $(shell if [ ! -d $(XI_BSP_DIR)/platform/$(XI_BSP_PLATFORM) ]; then echo 0; else echo 1; fi; )

ifeq ($(BSP_PLATFORM_DIR_EXIST),0)
	$(error The platform with BSP implementation - [$(XI_BSP_PLATFORM)] couldn't be found. Please check your $(XI_BSP_DIR)/platform/ directory.)
endif

XI_SRCDIRS += $(XI_BSP_DIR)

# platform specific BSP implementations
XI_SRCDIRS += $(XI_BSP_DIR)/platform/$(XI_BSP_PLATFORM)

XI_INCLUDE_FLAGS += -I$(LIBXIVELY)/include/bsp

# platform independent BSP drivers
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io/net
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/memory
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/event_loop
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/time

# if no tls_bsp then set proper flag
ifeq (,$(findstring tls_bsp,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_NO_TLS_LAYER

	ifneq (,$(findstring tls_socket,$(CONFIG)))
		XI_CONFIG_FLAGS += -DXI_BSP_IO_NET_TLS_SOCKET
	else
		XI_CONFIG_FLAGS += -DXI_MQTT_PORT=1883
	endif

	XI_SRCDIRS += $(XI_BSP_DIR)/tls/crypto-algorithms

else
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/tls/certs
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/tls
	XI_SRCDIRS += $(XI_BSP_DIR)/tls/$(XI_BSP_TLS)
endif

#
# SOURCE CONFIGURATIONS
#
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/io/fs
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/event_dispatcher
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/datastructures
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/mqtt/codec
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/mqtt/logic
XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/control_topic

ifneq (,$(findstring senml,$(CONFIG)))
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/senml
	XI_CONFIG_FLAGS += -DXI_SENML_ENABLED
	XI_SENML_ENABLED := 1
endif

ifneq (,$(findstring control_topic,$(CONFIG)))
	XI_SRCDIRS += $(LIBXIVELY_SRC)/import/protobuf-c/library
	XI_PROTOFILES := $(wildcard $(XI_PROTO_DIR)/*.proto)
	XI_PROTOFILES := $(XI_PROTOFILES:$(XI_PROTO_DIR)/%=%)
	XI_PROTOFILES_C := $(addprefix $(XI_PROTOBUF_GENERATED)/,$(XI_PROTOFILES:.proto=.pb-c.c))
	XI_CONTROL_TOPIC_ENABLED := 1
	XI_SRCDIRS += $(XI_PROTOBUF_GENERATED)
	XI_SOURCES += $(XI_PROTOFILES_C)
endif

ifneq (,$(findstring secure_file_transfer,$(CONFIG)))
	XI_INCLUDE_FLAGS += -I$(LIBXIVELY_SRC)/import/cn-cbor/include

	XI_SRCDIRS += $(LIBXIVELY_SRC)/import/cn-cbor/src
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/cbor
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/sft

	XI_SECURE_FILE_TRANSFER_ENABLED := 1
endif

ifneq (,$(findstring gateway,$(CONFIG)))
	XI_CONFIG_FLAGS += -DXI_GATEWAY_FEATURE_ENABLED
	XI_SRCDIRS += $(LIBXIVELY_SOURCE_DIR)/gateway

	XI_GATEWAY_FEATURE_ENABLED := 1
endif

# MODULES SRCDIRS
XI_SRCDIRS += $(foreach platformdep,$(XI_PLATFORM_MODULES_ENABLED) \
			,$(LIBXIVELY_SOURCE_DIR)/platform/$(XI_PLATFORM_BASE)/$(platformdep))

XI_INCLUDE_FLAGS += $(foreach platformdep,$(XI_PLATFORM_MODULES) \
			,-I$(LIBXIVELY_SOURCE_DIR)/platform/$(platformdep))

XI_INCLUDE_FLAGS += $(foreach d, $(LIBXIVELY_INTERFACE_INCLUDE_DIRS), -I$d)

XI_INCLUDE_FLAGS += -I$(LIBXIVELY_SOURCE_DIR)

XI_INCLUDE_FLAGS += $(foreach d, $(XI_SRCDIRS), -I$d)

XI_INCLUDE_FLAGS += -I$(LIBXIVELY_SOURCE_DIR)/platform/$(XI_PLATFORM_BASE)

XI_INCLUDE_FLAGS += $(foreach d,\
			$(wildcard $(LIBXIVELY_SOURCE_DIR)/platform/$(XI_PLATFORM_BASE)/*),-I$d)

XI_SOURCES += $(wildcard ./src/*.c)

XI_SOURCES += $(foreach layerdir,$(XI_SRCDIRS),\
	$(wildcard $(layerdir)/*.c))

ifeq ($(XI_DEBUG_OUTPUT),0)
XI_SOURCES := $(filter-out $(LIBXIVELY_SOURCE_DIR)/xi_debug.c, $(XI_SOURCES) )
endif

ifdef MAKEFILE_DEBUG
$(info --mt-config-- Using [$(XI_BSP_PLATFORM)] BSP configuration)
$(info --mt-config-- event_loop=$(XI_EVENT_LOOP))
$(info --mt-config-- $$XI_PLATFORM_BASE is [${XI_PLATFORM_BASE}])
$(info --mt-config-- $$XI_PLATFORM_MODULES is [${XI_PLATFORM_MODULES}])
$(info --mt-config-- $$LIBXIVELY_SOURCE_DIR is [${LIBXIVELY_SOURCE_DIR}])
$(info --mt-config-- $$XI_SOURCES is [${XI_SOURCES}])
$(info --mt-config-- $$XI_INCLUDE_FLAGS is [${XI_INCLUDE_FLAGS}])
$(info --mt-config-- $$XI_OBJDIR_BASE is [${XI_OBJDIR_BASE}])
$(info --mt-config-- $$XI_BINDIR_BASE is [${XI_BINDIR_BASE}])
$(info --mt-config-- $$XI_SRCDIRS is [${XI_SRCDIRS}])
endif
