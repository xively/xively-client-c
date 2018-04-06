# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_TLS_LIB_INC_DIR ?= ./src/import/tls/wolfssl/
XI_TLS_LIB_BIN_DIR ?= ./src/import/tls/wolfssl/src/.libs/
XI_TLS_LIB_NAME ?= wolfssl

# wolfssl API
XI_CONFIG_FLAGS += -DHAVE_SNI
XI_CONFIG_FLAGS += -DHAVE_CERTIFICATE_STATUS_REQUEST

# WARNING: This is for Posix Desktop compatability
# where the computer does not have a hardened hardware
# based random  number generator. 
# When compiling for an EMBEDDED DEVICE, you should
# compile the WolfSSL Library in Hardened Mode (see 
# src/import/tls/) and attempt to tie WolfSSL's 
# implementation so that TLS uses the hardware random
# number generator on the device.
XI_CONFIG_FLAGS += -DWC_NO_HARDEN

# libxively OCSP stapling feature switch
XI_CONFIG_FLAGS += -DXI_TLS_OCSP_STAPLING
# libxively OCSP feature switch
# XI_CONFIG_FLAGS += -DXI_TLS_OCSP

XI_CONFIG_FLAGS += -DXI_TLS_LIB_WOLFSSL
