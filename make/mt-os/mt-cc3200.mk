# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk

ifdef XI_TRAVIS_BUILD
	# For Xively C Client Continuous Integration only.
	COMPILER ?= arm_15.12.3.LTS
endif


###
## STEP 1: COMPILER NAME AND SDK
##
## Set the version names of tools and SDK.
## If you don't know the name of your compiler
## please check the TI CCSv7 tools directory for the subdirectory compiler.
## In that directory there should be a directory with a name similar
## to the default COMPILER value below.
### 

COMPILER ?= ti-cgt-arm_16.9.3.LTS
XI_CC3200_SDK ?= CC3200SDK_1.1.0

###
## STEP 2: HOST SYSTEM PATHS
##
## Configure the paths to where you've installed your CC3200 toolchain and
## SDK depending on if you're running on macOS, Windows, or Linux.
###

#
## MAC HOST OS
#
ifeq ($(XI_HOST_PLATFORM),Darwin)
	# osx cross-compilation downloads

	XI_CC3200_PATH_CCS_TOOLS ?= /Applications/ti/ccsv7/tools
	XI_CC3200_PATH_SDK ?= $(HOME)/ti/tirex-content/$(XI_CC3200_SDK)/cc3200-sdk

	CC = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

#
## WINDOWS HOST OS
#
else ifneq (,$(findstring Windows,$(XI_HOST_PLATFORM)))
	# windows cross-compilation

	XI_CC3200_PATH_CCS_TOOLS ?= C:/ti/ccsv7/tools
	XI_CC3200_PATH_SDK ?= C:/ti/$(XI_CC3200_SDK)/cc3200-sdk/

	CC = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

#
## LINUX HOST OS
#
else ifeq ($(XI_HOST_PLATFORM),Linux)
	# linux cross-compilation prerequisite downloads

	XI_CC3200_PATH_CCS_TOOLS ?= $(HOME)/Downloads/xively-client-artifactory/ti/ccsv6/tools
	XI_CC3200_PATH_SDK ?= $(HOME)/Downloads/xively-client-artifactory/ti/$(XI_CC3200_SDK)/cc3200-sdk

	CC = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armcl
	AR = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/bin/armar

ifdef XI_TRAVIS_BUILD
### TOOLCHAIN AUTODOWNLOAD SECTION --- BEGIN
# for Xively C Client Continuous Integration only.
	XI_BUILD_PRECONDITIONS := $(CC)

$(CC):
	git clone -b ti git@github.com:xively/xively-client-artifactory.git $(HOME)/Downloads/xively-client-artifactory
	$@ -version
### TOOLCHAIN AUTODOWNLOAD SECTION --- END
endif
endif

$(info .    XI_CC3200_SDK:   [$(XI_CC3200_SDK)] )
$(info .    XI_CC3200_COMPILERPATH:  [$(CC)] )

# removing these compiler flags since they are not parsed and emit warnings
XI_COMPILER_FLAGS_TMP := $(filter-out -Wall -Werror -Wno-pointer-arith -Wno-format -Wextra -Os -g -O0, $(XI_COMPILER_FLAGS))
XI_COMPILER_FLAGS = $(XI_COMPILER_FLAGS_TMP)

XI_COMPILER_FLAGS += -mv7M4
XI_COMPILER_FLAGS += -me
XI_COMPILER_FLAGS += -ms
XI_COMPILER_FLAGS += --define=css
XI_COMPILER_FLAGS += --define=cc3200
XI_COMPILER_FLAGS += --define=WOLFSSL_NOOS_XIVELY
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
XI_COMPILER_OUTPUT = --output_file=$@

ifneq (,$(findstring release,$(TARGET)))
    XI_COMPILER_FLAGS += -O4
endif

ifneq (,$(findstring debug,$(TARGET)))
    XI_COMPILER_FLAGS += -O0 -g
endif

XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/driverlib
XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/inc
XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/example/common
XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/simplelink/include
XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/simplelink_extlib/provisioninglib
XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/simplelink_extlib/include

XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_CCS_TOOLS)/compiler/$(COMPILER)/include

# Xively Client config flags
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

# wolfssl API
XI_CONFIG_FLAGS += -DNO_WRITEV
XI_CONFIG_FLAGS += -DSINGLE_THREADED

XI_ARFLAGS := r $(XI)
XI_LIB_FLAGS := -llibxively.a

XI_POST_COMPILE_ACTION =
