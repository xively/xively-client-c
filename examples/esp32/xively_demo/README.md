Build instructions
==================

1. Build the MQTT library: `cd $(XIVELY_CLIENT_C_PATH); make PRESET=ESP32`
2. Build the WolfSSL library: `cd $(XIVELY_CLIENT_C_PATH)/examples/esp32/xively_demo/wolfssl-make/; make`
3. Build this example: `cd $(XIVELY_CLIENT_C_PATH)/examples/esp32/xively_demo/; make`

In step 3, you will have to select the path to your serial port. i.e. `/dev/cu.SLAB_USBtoUART`

Flash instructions
==================

`cd $(XIVELY_CLIENT_C_PATH)/examples/esp32/xively_demo/; make flash`
