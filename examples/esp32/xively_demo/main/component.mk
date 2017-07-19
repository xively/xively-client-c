#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the 
# src/ directory, compile them and link them into lib(subdirectory_name).a 
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

#TODO: Make these paths relative and test
COMPONENT_EXTRA_INCLUDES += /Users/palantir/Work/Xively/esp32/xively-client-c/src/import/tls/wolfssl
COMPONENT_ADD_LDFLAGS += /Users/palantir/Work/Xively/esp32/xively-client-c/bin/esp32/libwolfssl.a

COMPONENT_EXTRA_INCLUDES += /Users/palantir/Work/Xively/esp32/xively-client-c/include
COMPONENT_ADD_LDFLAGS += /Users/palantir/Work/Xively/esp32/xively-client-c/bin/esp32/libxively.a
