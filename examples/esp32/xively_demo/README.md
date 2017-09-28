ESP32 Demo - Technical Documentation
====================================

##### Copyright (c) 2003-2017, LogMeIn, Inc. All rights reserved.

This is part of the Xively C Client library, it is licensed under the BSD 3-Clause license.

For more information please read our License.md file located in the base directory of the [Xively C Client repository](https://github.com/xively/xively-client-c).

Demo Tutorial
=============

A detailed tutorial on how to build, flash and run this demo can be found in our
Developer Centre: https://developer.xively.com/docs/esp32

This README contains technical details on the codebase so you can understand it
and re-use it more easily.

Application Architecture
========================

## File Structure

```
                            +-------------------------+
                            |          main.c         |
                            +-------------------------+
                            | Device Initialization   |
                            | RTOS tasks coordination |
                            | App-relevant callbacks  |
                            +---+-----+-----+-----^---+
                                |     |     |     |
          +---------------------+     |     |     +----------------------+
          |                           |     |                            |
          |                   +-------+     +------+                     |
          |                   |                    |                     |
+---------v--------+ +--------v---------+ +--------v---------+ +---------v--------+
|   gpio_if.[ch]   | |  user_data.[ch]  | | provisioning.[ch]| |  xively_if.[ch]  |
+------------------+ +------------------+ +------------------+ +------------------+
|Button interrupts | |Credentials struct| |Gather credentials| |libxively task    |
|LED control       | |NVS read/write    | | at runtime       | |Task-safe control |
+---------^--------+ +--------^---------+ +--------^---------+ +---------^--------+
          |                   |                    |                     |
          |                   | SPI                | UART                | MQTT+TLS
          |                   |                    |                     |
   XXXXXX\v/XXXXX       XXXXX\v/XXXXXX    .........v..........   XXXXXXX\v/XXXXXX
  XX            XX     XX            XX   .                  .  XXXX          XXXX
 XX    ESP32's   XX   XX   ESP32's    XX  .    O             . XXX    Xively    XXX
XX      GPIO      XX XX  Non-Volatile  XX .   -|-  User      . XXXXX  Broker  XXXXX
 XX              XX   XX   Storage    XX  .   / \            . XXX              XXX
  XX            XX     XX            XX   .                  .  XXXX          XXXX
   XXXXXXXXXXXXXX       XXXXXXXXXXXXXX    .........^..........   XXXXXXX/^\XXXXXX
                                                   |                     |

                                                   |                     |
                                                   +-   -   -   -   -   -+
                                                   (HTTP+TLS || MQTT+TLS)
```

## MQTT setup

Used channels:

- `/Button`: Every time you press the button, an int will switch between 1 and 0,
and it will be published by the device to this channel
- `/LED`: Sending the messages `1` or `0` to the device's LED topic will turn
the LED on or off

Provisioning your device over UART
==================================

First of all, you need to connect to your device's serial port after flashing it.
These are the settings it uses:

- 115200 bps
- 8-N-1 bits

If you've never connected your device to Xively before, it will ask you to set
new WiFi and MQTT credentials. Simply follow the instructions presented to you
over the serial terminal session.

If you'd like to change the credentials at any point, you can reboot the device
and press the GPIO0 button at any point while the LED is flashing rapidly. This
will kickstart the provisioning process (ask for your credentials again), and
over-write the stored credentials.

## Using hardcoded credentials [Alternative]

In some cases, you may want to disable the usage of credentials in Non Volatile
Storage (NVS), in favour of credentials hardcoded into the application. You can
do so by enabling the `USE_HARDCODED_CREDENTIALS` macro in `main.c`. You'll also
have to set your credentials in these lines:

```
#define APP_XI_ACCOUNT_ID "[SET YOUR XIVELY ACCOUNT ID HERE]"
#define APP_XI_DEVICE_ID  "[SET YOUR XIVELY DEVICE  ID HERE]"
#define APP_XI_DEVICE_PWD "[SET YOUR XIVELY DEVICE PASSWORD HERE]"
#define APP_WIFI_SSID "[SET YOUR WIFI NETWORK NAME HERE]"
#define APP_WIFI_PASS "[SET YOUR WIFI NETWORK PASSWORD HERE]"
```

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
