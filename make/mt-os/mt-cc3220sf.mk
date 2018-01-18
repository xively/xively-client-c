# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk

CC32XX ?= 0

###
## COMPILER NAME
###
COMPILER ?= ti-cgt-arm_16.9.6.LTS

###
## MAC HOST OS
###
ifeq ($(XI_HOST_PLATFORM),Darwin)
	# osx cross-compilation downloads

	XI_CC3220SF_PATH_CCS_TOOLS ?= /Applications/ti/ccsv7/tools
	XI_CC3220SF_PATH_SDK ?= /Applications/ti/simplelink_cc32xx_sdk_1_50_00_06
	XI_CC3220SF_PATH_XDC_SDK ?= /Applications/ti/xdctools_3_50_03_33_core


	CC = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

	XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/include

###
## WINDOWS HOST OS
###
else ifneq (,$(findstring Windows,$(XI_HOST_PLATFORM)))
	 # windows cross-compilation

    XI_CC3220SF_PATH_CCS_TOOLS ?= C:/ti/ccsv7/tools

	XI_CC3220SF_PATH_SDK ?= C:/ti/simplelink_cc32xx_sdk_1_60_00_04
	XI_CC3220SF_PATH_XDC_SDK ?= C:/ti/xdctools_3_50_03_33_core

	CC = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

	XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/include

###
## LINUX HOST OS
###
else ifeq ($(XI_HOST_PLATFORM),Linux)
	# linux cross-compilation prerequisite downloads

	XI_CC3220SF_PATH_CCS_TOOLS ?= $(HOME)/Downloads/xi_artifactory/ti/ccsv6/tools
	XI_CC3220SF_PATH_SDK ?= $(HOME)/Downloads/xi_artifactory/ti/CC3220SDK_1.2.0/cc3220-sdk

	CC = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

	XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_CCS_TOOLS)/compiler/$(COMPILER)/include

### TOOLCHAIN AUTODOWNLOAD SECTION --- BEGIN
	XI_BUILD_PRECONDITIONS := $(CC)

$(CC):
	git clone https://github.com/atigyi/xi_artifactory.git ~/Downloads/xi_artifactory
	git -C ~/Downloads/xi_artifactory checkout feature/cc3220_dependencies
	$@ --version
### TOOLCHAIN AUTODOWNLOAD SECTION --- END
endif

# removing these compiler flags since they are not parsed and emit warnings
XI_COMPILER_FLAGS_TMP := $(filter-out -Wall -Werror -Wno-pointer-arith -Wno-format -Wextra -Os -g -O0, $(XI_COMPILER_FLAGS))
XI_COMPILER_FLAGS = $(XI_COMPILER_FLAGS_TMP)

XI_COMPILER_FLAGS += -mv7M4
XI_COMPILER_FLAGS += -me
XI_COMPILER_FLAGS += --define=css
XI_COMPILER_FLAGS += --define=cc3200
XI_COMPILER_FLAGS += --define=WOLFSSL_NOOS_XIVELY
XI_COMPILER_FLAGS += --define=SL_OTA_ARCHIVE_STANDALONE
XI_COMPILER_FLAGS += --display_error_number
XI_COMPILER_FLAGS += --diag_warning=225
XI_COMPILER_FLAGS += --diag_wrap=off
XI_COMPILER_FLAGS += --abi=eabi
XI_COMPILER_FLAGS += --opt_for_speed=0
XI_COMPILER_FLAGS += --code_state=16
XI_COMPILER_FLAGS += --float_support=vfplib
XI_COMPILER_FLAGS += --preproc_with_compile
XI_COMPILER_FLAGS += --preproc_dependency=$(@:.o=.d)
XI_COMPILER_FLAGS += --obj_directory=$(dir $@)
XI_COMPILER_FLAGS += --asm_directory=$(dir $@)
XI_COMPILER_OUTPUT += --output_file=$@

ifneq (,$(findstring release,$(TARGET)))
    XI_COMPILER_FLAGS += -O4
endif

ifneq (,$(findstring debug,$(TARGET)))
    XI_COMPILER_FLAGS += -O0 -g
endif

XI_COMPILER_FLAGS += -DCC32XX_COMPAT=1
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/devices/cc32xx/driverlib
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/net/ota/source
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/drivers
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/drivers/net/wifi
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/drivers/net/wifi/bsd
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/drivers/net/wifi/bsd/sys
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/drivers/net/wifi/bsd/arpa
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/source/ti/devices/cc32xx/inc

# clock
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_SDK)/kernel/tirtos/packages
XI_COMPILER_FLAGS += -I$(XI_CC3220SF_PATH_XDC_SDK)/packages


# Xively Client config flags
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS
XI_CONFIG_FLAGS += -DXI_DEBUG_PRINTF=Report
#XI_CONFIG_FLAGS += -DXI_CC3220SF_UNSAFELY_DISABLE_CERT_STORE #Will also disable the store's CRL

# wolfssl API
XI_CONFIG_FLAGS += -DNO_WRITEV
XI_CONFIG_FLAGS += -DSINGLE_THREADED

XI_ARFLAGS := r $(XI)
XI_LIB_FLAGS := -llibxively.a

XI_POST_COMPILE_ACTION =
