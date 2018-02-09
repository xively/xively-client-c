#
# Component Makefile
#

XI_CLIENT_C_PATH_RELATIVE = ../../../../..

COMPONENT_PRIV_INCLUDEDIRS +=                \
    $(XI_CLIENT_C_PATH_RELATIVE)/src/bsp     \
    $(XI_CLIENT_C_PATH_RELATIVE)/include     \
    $(XI_CLIENT_C_PATH_RELATIVE)/include/bsp \
    $(XI_CLIENT_C_PATH_RELATIVE)/src/import/tls/wolfssl

COMPONENT_SRCDIRS := .
