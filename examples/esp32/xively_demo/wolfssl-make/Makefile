# Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client codebase,
# it is licensed under the BSD 3-Clause license.

#$(IDF_PATH) This is exported in the shell as a requirement of the IDF SDK
ESP_SDK_DIR ?= $(IDF_PATH)
XI_BASE_DIR ?= $(realpath ../../../..)

# The path to CC and AR must be in the environment $PATH as an IDF requirement
CC = xtensa-esp32-elf-gcc
AR = xtensa-esp32-elf-ar

WOLFSSL_BASE_DIR := $(XI_BASE_DIR)/src/import/tls/wolfssl

WOLFSSL_OUTPUT_OBJ_DIR := $(XI_BASE_DIR)/obj/esp32/wolfssl
WOLFSSL_OUTPUT_LIB_DIR := $(XI_BASE_DIR)/bin/esp32
WOLFSSL_OUTPUT_LIB     := $(WOLFSSL_OUTPUT_LIB_DIR)/libwolfssl.a

###############################################################################
# GCC flags
###############################################################################
#Sources
WOLFSSL_SOURCES =                               \
    $(WOLFSSL_BASE_DIR)/src/internal.c          \
    $(WOLFSSL_BASE_DIR)/src/io.c                \
    $(WOLFSSL_BASE_DIR)/src/keys.c              \
    $(WOLFSSL_BASE_DIR)/src/ocsp.c              \
    $(WOLFSSL_BASE_DIR)/src/ssl.c               \
    $(WOLFSSL_BASE_DIR)/src/tls.c               \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/aes.c     \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/arc4.c    \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/asn.c     \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/coding.c  \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/dh.c      \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/error.c   \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/hash.c    \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/hmac.c    \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/integer.c \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/logging.c \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/md5.c     \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/memory.c  \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/random.c  \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/rsa.c     \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/sha.c     \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/sha256.c  \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/wc_port.c \
    $(WOLFSSL_BASE_DIR)/wolfcrypt/src/wc_encrypt.c

#Include flags
WOLFSSL_INCDIRS = \
    -I${WOLFSSL_BASE_DIR}

LWIP_INCDIRS =                                    \
    -I$(ESP_SDK_DIR)/components/lwip/system       \
    -I$(ESP_SDK_DIR)/components/lwip/include/lwip \
    -I$(ESP_SDK_DIR)/components/lwip/include/lwip/port

FREERTOS_INCDIRS =                               \
    -I$(ESP_SDK_DIR)/components/freertos/include \
    -I$(ESP_SDK_DIR)/components/freertos/include/freertos

#Build settings
WOLFSSL_SETTINGS =        \
    -DSIZEOF_LONG_LONG=8  \
    -DSMALL_SESSION_CACHE \
    -DHAVE_OCSP           \
    -DHAVE_SNI            \
    -DHAVE_TLS_EXTENSIONS \
    -DTIME_OVERRIDES      \
    -DNO_DES              \
    -DNO_DES3             \
    -DNO_DSA              \
    -DNO_ERROR_STRINGS    \
    -DNO_HC128            \
    -DNO_MD4              \
    -DNO_OLD_TLS          \
    -DNO_PSK              \
    -DNO_PWDBASED         \
    -DNO_RC4              \
    -DNO_RABBIT           \
    -DNO_SHA512           \
    -DNO_STDIO_FILESYSTEM \
    -DNO_WOLFSSL_DIR      \
    -DNO_DH               \
    -DWOLFSSL_STATIC_RSA  \
    -DWOLFSSL_IAR_ARM     \
    -DNDEBUG              \
    -DHAVE_CERTIFICATE_STATUS_REQUEST
    #-DCUSTOM_RAND_GENERATE_SEED=your_random_seeding_function
    # Already defined in wolfssl/wolfssl/wolfcrypt/settings.h:
    #-DSINGLE_THREADED
    #-DNO_WRITEV

#CC flags as extracted from the SDK
CC_FLAGS =                      \
    -fstrict-volatile-bitfields \
    -ffunction-sections         \
    -fdata-sections             \
    -mlongcalls                 \
    -nostdlib                   \
    -ggdb                       \
    -Os                         \
    -DNDEBUG                    \
    -std=gnu99                  \
    -Wno-old-style-declaration  \
    $(WOLFSSL_INCDIRS)          \
    $(LWIP_INCDIRS)             \
    $(FREERTOS_INCDIRS)         \
    $(WOLFSSL_SETTINGS)

WOLFSSL_OUTPUT_OBJ := $(patsubst $(WOLFSSL_BASE_DIR)%.c,$(WOLFSSL_OUTPUT_OBJ_DIR)%.o,$(WOLFSSL_SOURCES))

###############################################################################
#
# default/lib: build the wolfssl library
#
###############################################################################
lib: $(WOLFSSL_BASE_DIR) $(WOLFSSL_OUTPUT_LIB)

$(WOLFSSL_BASE_DIR):
	cd $(XI_BASE_DIR)/src/import/tls/ && ./download_and_compile_wolfssl.sh

$(WOLFSSL_OUTPUT_OBJ_DIR)%.o: $(WOLFSSL_BASE_DIR)%.c
	@mkdir -p $(dir $@)
	@echo "  [CC] $<"
	@$(CC) $(CC_FLAGS) -c $< -o $@

$(WOLFSSL_OUTPUT_LIB): $(WOLFSSL_OUTPUT_OBJ)
	@mkdir -p $(dir $@)
	@echo "  [AR] Static library: $@"
	@$(AR) -rs -c $@ $^

###############################################################################
#
# clean: rm -rf the output object directory and the static library file
#
###############################################################################
clean:
	rm -rf $(WOLFSSL_OUTPUT_OBJ_DIR)
	rm -f $(WOLFSSL_OUTPUT_LIB)

###############################################################################
#
# all: clean and re-build the library
#
###############################################################################
all: clean lib

.PHONY: all lib clean
