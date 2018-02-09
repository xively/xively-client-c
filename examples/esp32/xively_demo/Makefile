#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#
XI_CLIENT_C_PATH = $(realpath $(PROJECT_PATH)/../../..)

PROJECT_NAME         := xively_demo
EXTRA_COMPONENT_DIRS += $(XI_CLIENT_C_PATH)/src/bsp/platform/esp32/xi_esp32_bsp_component

include $(IDF_PATH)/make/project.mk
