#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

CFLAGS += -DMBEDTLS_PLATFORM_MEMORY
CFLAGS += -DSNTP_MAX_SERVERS=4 #if modified, update mt-esp32.mk and xi_bsp_time_esp32_sntp.c too


#TODO: Make these paths relative and test
COMPONENT_EXTRA_INCLUDES += /Users/palantir/Work/Xively/esp32/xively-client-c/include
COMPONENT_ADD_LDFLAGS += /Users/palantir/Work/Xively/esp32/xively-client-c/bin/esp32/libxively.a
