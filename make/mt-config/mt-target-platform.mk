# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

# Detect the host platform, it can be overriden
ifeq ($(OS),Windows_NT)
XI_HOST_PLATFORM ?= Windows_NT
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		XI_HOST_PLATFORM ?= Linux
		XI_PROTO_COMPILER := $(LIBXIVELY)/src/import/protobuf-c/compiler/protoc-c-linux
	endif
	ifeq ($(UNAME_S),Darwin)
		XI_HOST_PLATFORM ?= Darwin
		XI_PROTO_COMPILER := $(LIBXIVELY)/src/import/protobuf-c/compiler/protoc-c-macos
	endif
endif

#if nothing has been set or detected we could try with Unknown
XI_HOST_PLATFORM ?= Unknown

# Translate host platform to target platform - name differences
ifeq ($(XI_HOST_PLATFORM),Linux)
	XI_TARGET_PLATFORM ?= linux
else ifeq ($(XI_HOST_PLATFORM),Darwin)
	XI_TARGET_PLATFORM ?= osx
endif

# If there is no platform
XI_TARGET_PLATFORM ?= Unknown
