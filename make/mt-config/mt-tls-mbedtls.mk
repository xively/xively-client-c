# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_TLS_LIB_INC_DIR ?= ./src/import/tls/mbedtls/include/
XI_TLS_LIB_BIN_DIR ?= ./src/import/tls/mbedtls/library/
XI_TLS_LIB_NAME ?= mbedtls mbedx509 mbedcrypto

XI_CONFIG_FLAGS += -DXI_TLS_LIB_MBEDTLS
