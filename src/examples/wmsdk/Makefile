# Copyright (C) 2008-2013 Marvell International Ltd.
# All Rights Reserved.

# Batch Build Applications using GNU Cross-Compiler and Make.
#
# Usage Guide:
#
#     The Applications are built against a pre-built SDK
#     and for a specified hardware board.
#
#     The pre-built SDK location is passed using SDK_PATH
#     variable. It should be an absolute location.
#
#     The board file is passed using the BOARD_FILE variable.
#
#     all:     build all the apps
#     clean:   cleans the apps directory to pristine state
#
#     install: install the app artifacts in location pointed
#              to by INSTALL_DIR variable.
#     The Makefile simply does recursive build for all the
#     apps.

# Changes made by Team Left Shark
#
#	  The pre-built SDK location is passed using WMSDK_PATH not SDK_PATH

ifneq ($(NOISY),1)
AT=@
SILENT=-s
endif

# Location of GNU toolchain specific files
TOOLCHAIN_DIR := $(CURDIR)/toolchains/gnu

# Install in ./bin if no INSTALL_DIR is passed
INSTALL_DIR   ?= $(CURDIR)/bin

export TOOLCHAIN_DIR INSTALL_DIR
export WMSDK_PATH

-include $(WMSDK_PATH)/.config

# All the apps to build
APPS = async_test_app mqtt_logic_test hello_world

# Install only a select few apps
INSTALL_APPS = $(APPS)

all: apps what_next

app_build := $(subst /,@,$(APPS))
apps:  $(app_build)

$(app_build): check_precond
	export OBJ_DIR=$(XI_OBJDIR_BASE)/$@; \
	export BIN_DIR=$(XI_BINDIR)/$@; \
	$(MAKE) $(SILENT) -C $(CURDIR)/$@ all;

# Convenience Checking rules
#
# check_perm:
#  -- Ensure that the tarball is extracted with the right permissions
# check_sdk:
#   -- Ensure that SDK_PATH variable is set and points to a pre-built
#      SDK
#

check_precond: check_perm check_sdk

check_perm:
	@PERM=`ls -l Makefile | awk '{print $$1}'`; \
	if [  "$$PERM" =  "----------+" ]; then \
	echo "ERROR: File permissions do not seem right."; \
	echo "Please use the tar cmd line program to untar the software tarballs!";	\
	exit 1; \
	fi

check_sdk:
#   Ensure that SDK_PATH variable is set.
ifeq ($(WMSDK_PATH),)
	$(error "ERROR: WMSDK_PATH not set. Please set WMSDK_PATH variable to a configured and compiled SDK")
endif
	@if [ ! -e $(WMSDK_PATH)/.config ] || [ ! -e $(WMSDK_PATH)/incl/autoconf.h ]; then \
		echo "ERROR: $(WMSDK_PATH) does not contain configured and compiled SDK"; \
		exit 1; \
	fi

what_next: apps
	@echo
	@echo "############################################"
	@echo "Sample Apps build is now ready for $(BOARD) board!"
	@echo "Now, you can go to $(WMSDK_PATH)/tools/mc200/OpenOCD and use:"
	@echo " ramload.sh: for loading the axf in RAM"
	@echo " flashprog.sh: for writing images to flash"
	@echo ""
	@echo "Note: If intended demo is not built, make sure"
	@echo "relevant config options are enabled."
	@echo "############################################"
	@echo
