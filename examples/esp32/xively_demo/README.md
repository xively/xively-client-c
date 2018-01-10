ESP32 Demo - Technical Documentation
====================================

##### Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.

This is part of the Xively C Client library, it is licensed under the BSD 3-Clause license.

For more information please read our License.md file located in the base directory of the [Xively C Client repository](https://github.com/xively/xively-client-c).

Demo Tutorial
=============

A detailed tutorial on how to build, flash and run this demo can be found in our
Developer Centre: https://developer.xively.com/docs/esp32

This README contains technical details on the codebase so you can understand it
and re-use it more easily.

The Demo
========

## Hardware setup

This demo uses 2 peripherals:

- User button: Connected to GPIO0, used to kickstart provisioning and to publish messages
- User LED: Connected to GPIO17, used to indicate status and to be controlled via MQTT

The device is assumed to have a 4MB flash chip. If yours is smaller, you'll
have to modify the partition table and possibly some code to fit your setup.

Watch Out! We use the button connected to GPIO0 because it's available in most
development boards, but that's because it's a bootstrap pin for the bootloader.
If you keep the GPIO0 button pressed during boot, the device will go into
bootloader mode.

## MQTT channels

- `/Button`: Every time you press the button, an int will switch between 1 and 0,
and it will be published by the device to this channel
- `/LED`: Sending the messages `1` or `0` to the device's LED topic will turn
the LED on or off

## Demo Behaviour

0. Do NOT keep the GPIO0 button pressed during boot; it will go into bootloader mode,
and the application won't start
1. The device boots, and the LED starts blinking rapidly:
    - If it's missing any required WiFi or Xively credentials, it will start the runtime provisioning routine
    - If the user button is pressed **while the LED is blinking rapidly**, it will start the runtime provisioning routine
2. Runtime provisioning: The LED will be ON. Follow the instructions in the UART
port to provide the device the necessary WiFi and MQTT credentials. These will
be saved to flash, so you only need to do it once
3. The LED will be off until the device is connected to the WiFi Access Point
4. The LED will blink slowly (1s ON/1s OFF) while the device is disconnected from
the MQTT broker
5. The LED will will be off by default, and can be controlled via MQTT with the `/LED`
channel, while the device is connected to the broker
6. If an Over The Air update is triggered from the cloud, the device will rapidly
blink twice every time a file chunk is saved to flash. If the update contained
a new application binary, the device will reboot into it. If it only contained
other files for the filesystem, the device won't reboot
7. There's an 'application-level watchdog' in `main.c`. It will reboot the device
if the Xively Task is shut down for any reason. You may need to remove/modify it
for your own application; check out the `main.c` source code for more details

## Credentials provisioning over UART

First of all, you need to connect to your device's serial port after flashing it.
These are the settings it uses:

- 115200 bps
- 8-N-1 bits

If you've never connected your device to Xively before, it will ask you to set
new WiFi and MQTT credentials. Simply follow the instructions presented to you
over the serial terminal.

If you'd like to change the credentials at any point, you can reboot the device
and press the GPIO0 button while the LED is flashing rapidly. This will kickstart
the provisioning process (ask for your credentials again), and over-write the
stored credentials.

### Using hardcoded credentials [Alternative]

In some cases, during development, you may want to disable the usage
of credentials in Non Volatile Storage (NVS), in favour of credentials hardcoded
into the application. You can do so by enabling the `USE_HARDCODED_CREDENTIALS`
macro in `main.c`. You'll also have to set your credentials in these lines:

```
#define APP_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
#define APP_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
#define APP_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#define APP_WIFI_SSID "[SET YOUR WIFI NETWORK NAME HERE]"
#define APP_WIFI_PASS "[SET YOUR WIFI NETWORK PASSWORD HERE]"
```

## File Structure

```
                           +-------------------------+
                           |          main.c         |
                           +-------------------------+
                           | Device Initialization   |
                           | RTOS tasks coordination |
                           | App-relevant callbacks  |
                           +---+-----+------+-----^--+
                               |     |      |     |
         +---------------------+     |      |     +-----------------------+
         |                           |      |                             |
         |                     +-----+      +---------+                   |
         |                     |                      |                   |
+--------v---------+ +---------v--------+   +---------v--------+ +--------v---------+ 
| provisioning.[ch]| |  xively_if.[ch]  <---|   gpio_if.[ch]   | |  user_data.[ch]  | 
+------------------+ +------------------+   +------------------+ +------------------+ 
|Gather credentials| |libxively task    |   |Button interrupts | |Credentials struct| 
| at runtime       | |Task-safe control |   |LED control       | |NVS read/write    | 
+--------^---------+ +---------^--------+   +---------^--------+ +--------^---------+ 
         |                     |                      |                   |           
         | UART                | MQTT+TLS             |                   | SPI       
         |                     |                      |                   |           
.........v..........   XXXXXXX\v/XXXXXX        XXXXXX\v/XXXXX       XXXXX\v/XXXXXX    
.                  .  XXXX          XXXX      XX            XX     XX            XX   
.    O             . XXX    Xively    XXX    XX    ESP32's   XX   XX   ESP32's    XX  
.   -|-  User      . XXXXX  Broker  XXXXX   XX      GPIO      XX XX  Non-Volatile  XX 
.   / \            . XXX              XXX    XX              XX   XX   Storage    XX  
.                  .  XXXX          XXXX      XX            XX     XX            XX   
.........^..........   XXXXXXX/^\XXXXXX        XXXXXXXXXXXXXX       XXXXXXXXXXXXXX    
         |                     |                                                   
         |                     |
         +-   -   -   -   -   -+
         (HTTP+TLS || MQTT+TLS)
```

## Security Considerations

This demo doesn't implement all the security measures you should use in a production
environment. Here are some recommendations to improve the security of the device;
please, research the manufacturer's documentation to harden your setup to the needs
of your threat model. For starters:

- Firmware binaries should be signed; secure boot enabled
- Flash contents should be encrypted
- For more aggressive threat models, external Hardware Security Modules can be
connected, and certificate-based authentication selected in Xively. This will
require heavier modifications to the source code. Contact us for support in the
process, ideally via GitHub issues

Testing the Demo Without Buttons or LEDs
========================================

If you'd like to test MQTT communication on your ESP32, but you can't easily
connect external peripherals, you can still force the device to publish data
from the source code.

You only need to set the `PUB_INCREASING_COUNTER` macro in `xively_if.c` to 1
and re-build your application. This will enable a scheduled task that publishes
an increasing counter to the `/Button` topic every 10 seconds. Whenever you
publish a message to the `/LED` topic, you'll be able to see it in the UART
logs, so you can see data coming in and out without any buttons or LEDs connected.

If you ever need to reprovision the device, consider using a jumper cable to
trigger the GPIO0 interrupt (GPIO0 to GND), or use the Hardcoded Credentials
method explained below.
