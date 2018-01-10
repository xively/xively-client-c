# Secure File Transfer Demo Applications

***Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.***

## Overview

The .bin files in this directory are two precompiled firmware images of the Xively Secure File Transfer demo for the TI CC3200.  

Please see the [Xively SFT CC3200 Example Tutorial in our Xively Developer Center](https://developer.xively.com/v1.0/docs/ti-cc3200-sft-example) for information on how these files should be used.

## Behavior
Both firmware images will publish their Firmware Revision string to the Xively SFT Service when a button is pressed on the device.  The images will subsequently fetch and store new firmware file revisions with the name of `firmware` from the Xively SFT service.  Any other files supplied by the SFT service will be rejected and errors will be logged to the serial console.

Remote tracking of the firmware installation progress and errors is done via Xively Device Logs. These logs can be viewed in Xively CPM on the Device's Details Page.

The device will automatically reboot itself when new firwmare has been downloaded. This boot will be a test boot of the firwmare. The device will revert to the previous firmware image if the test boot fails.  If the device can successfully build a connection to the Xivel service when in test mode, then the new firmware will be marked as valid and committed.

In order for this demo to work, one of the firmware images must be flashed onto the device using Uniflash. Then the other must be hosted in Xively SFT.  Please see the Xively SFT Tutorial (link above) for directions on how to setup your Xively Account for this demo.

## Files

### firmwareA.bin
Subscribes to a blink topic for the device. Incoming messages on that topic will toggle the **Green and Orange Light in unison.** 

You can send messages to the blink topic from the Messaging Widget via the Device Details Page, or by running a python script from your PC.  Please see the aformentioned tutorial (above) for more information.

The button **SW2** on the device forces the device to publish its current firmware revision string, which is `1.0`

#### firmwareB.bin
Subscribes to a blink topic for the device. Incoming messages on that topic will toggle the Green and Orange Light in toggle mode. First the Green light will be illuminated, then the Orange, and so on. There is no moment when both lights are off once incoming messags are received on the device.

The button **SW2** on the device forces the device to publish its current firmware revision string, which is `1.0` as well.

