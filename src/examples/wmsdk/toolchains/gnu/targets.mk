# Copyright (C) 2008-2012 Marvell International Ltd.
# All Rights Reserved.
#
# Application Makefile Common Targets:
#
#   This Makefile fragment provides boiler plate make targets for
#   application Makefile simplifying writing application Makefile.
#
#	  all: Builds the firmware binary image in both ELF (axf) format
#          (suitable for loading into RAM) and bin format (suitable
#          for flashing)
#
#     clean: Cleans all the build artifacts
#
#     install: Installs ELF image, Bin Image, and MAP file to
#              $(INSTALL_DIR). By default INSTALL_DIR = ./bin.
#
# Application Makefile needs to minimally set the following variables.
#
# SRCS     = list of sources
# DST_NAME = application name prefix for build artifacts
# SDK_PATH = absolute path to a pre-built SDK (for make all)
# INSTALL_DIR = absolute path to install directory (for make install)
#
# The following variables can be overridden by the main Makefile.
#
#   OBJ_DIR:  (default obj)
#		-- directory for object files (relative to main app dir)
#   BIN_DIR:  (default bin)
#		-- directory for final build artifacts (relative to main app dir)

OBJ_DIR     ?= $(CURDIR)/obj
BIN_DIR     ?= $(CURDIR)/bin

# Makefile Targets

ifeq ($(MAKE_FTFS),)
all: $(OBJ_DIR) $(BIN_DIR) board_file $(BIN_DIR)/$(DST_NAME).axf $(BIN_DIR)/$(DST_NAME).bin
else
all: $(OBJ_DIR) $(BIN_DIR) board_file $(BIN_DIR)/$(DST_NAME).axf $(BIN_DIR)/$(DST_NAME).bin $(BIN_DIR)/$(DST_NAME).ftfs
endif

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

board_file:
ifeq ($(BOARD_FILE),)
	$(AT)$(COPY_CMD) $(BOARD_FILE) $(SRC_DIR)/board.c
endif

src/board.c:
#   Ensure that BOARD_FILE variable is set.
ifeq ($(BOARD_FILE),)
	$(error "ERROR: BOARD_FILE not set. Please set BOARD_FILE variable to the absolute path to the board file to be used to build the applications")
endif
	$(AT)echo " [brd] Copying $(BOARD_FILE) to $(SRC_DIR)/board.c"

