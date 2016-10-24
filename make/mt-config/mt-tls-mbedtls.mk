XI_TLS_LIB_INC_DIR ?= ./src/import/tls/mbedtls/include/
XI_TLS_LIB_BIN_DIR ?= ./src/import/tls/mbedtls/library/
XI_TLS_LIB_NAME ?= mbedtls mbedx509 mbedcrypto

XI_CONFIG_FLAGS += -DTLSLIB_MBEDTLS
