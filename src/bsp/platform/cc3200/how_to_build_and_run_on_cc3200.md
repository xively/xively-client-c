# How to build and run Xively C Client on TI CC3200

This document describes a method which includes compilation and linkage of Xively C Client sources resulting in a CC3200 compatible static library which can be linked agains a Texas Instruments Code Composer Studio (CCS) example application. The result can be uploaded to the RAM and executed with the help of the CCS.

This method requires OSX as development platform although Windows version should be very similar.

#### Prerequisites:

- OSX as development platform
- a [TI CC3200](http://www.ti.com/tool/cc3200-launchxl?keyMatch=launchpad%20cc3200&tisearch=Search-EN-Everything) board
- to download the Code Composer Studio and the SDK you'll need to create a TI account, then these tools are free to download.
- [TI Code Composer Studio](http://www.ti.com/tool/ccstudio) installed on OSX
- [TI CC3200 SDK](http://www.ti.com/tool/cc3200sdk)
    - note: the SDK is downloadable in a Windows .exe compressed package format.
    - after extracting it on a Windows machine you should place it somewhere in your directory structure, e.g. to ~/ti/CC3200SDK


## Building the Xively C Client library

This step results in a static library suitable for CC3200. The command

    make PRESET=CC3200_REL_MIN

should do it, result should be the file xively-client-c/obj/cc3200/libxively.a
This preset includes TLS connection. To disable TLS edit the preset in file mt-presets.mk

    make PRESET=CC3200_REL_MIN clean

cleans the output of the previous build.

*under construction notes*:

## Building the wolfSSL library

The wolfSSL supports TI-RTOS builds. Follow the steps written on [Using wolfSSL with TI-RTOS](http://processors.wiki.ti.com/index.php/Using_wolfSSL_with_TI-RTOS) to generate wolfSSL static library for CC3200.

The following customizations were made for a Xively C Client wolfSSL build:



## Building CC3200 application: CCS ent_wlan example

The end_wlan is a non-os example application providing a wifi connection. This serves as a good light-weight starting point.

Steps to take:

#### Code Coposer Studio + ent_wlan example:

- create a new workspace for Code Composer Studio
- import the ent_wlan example from the CC3200 SDK's example directory
- build it
- debug it on the device
    - connect the device to the Mac
    - hit the green bug button on the top in the CCS

#### Adding Xively Client code to the ent_wlan example:

- locate the successful wifi connection point in the main.c of the ent_wlan example (comment: "wait for few moments")
- copy Xively C Client initialize code from one of the examples in from the Client repo
- to make it build you'll need to
    - include xively.h
    - link CC3200 static library: xively-client-c/obj/cc3200/libxively.a
    - link the wolfSSL static library: xively-client-c/src/import/tls/wolfssl/tirtos/packages/ti/net/wolfssl/lib/wolfssl.aem4f



