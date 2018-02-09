#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

XI_CLIENT_C_PATH = $(realpath $(PROJECT_PATH)/../../..)
XI_BSP_TLS ?=wolfssl

ifdef CONFIG_XIVELY_CLIENT
$(error ERROR! The xively component in the IDF SDK is enabled. \
               Disable it with `make menuconfig`, and `make clean` to link the Xively Client)
endif

ifeq ($(XI_BSP_TLS),wolfssl)
COMPONENT_EXTRA_INCLUDES += $(XI_CLIENT_C_PATH)/src/import/tls/wolfssl
COMPONENT_ADD_LDFLAGS    += $(XI_CLIENT_C_PATH)/bin/esp32/libwolfssl.a
else ifeq ($(XI_BSP_TLS),mbedtls)
else
$(error Unknown TLS BSP [$(XI_BSP_TLS)])
endif

COMPONENT_EXTRA_INCLUDES += $(XI_CLIENT_C_PATH)/src/bsp/platform/esp32/xi_esp32_bsp_component/include
COMPONENT_EXTRA_INCLUDES += $(XI_CLIENT_C_PATH)/include
COMPONENT_ADD_LDFLAGS    += $(XI_CLIENT_C_PATH)/bin/esp32/libxively.a
