# Porting Guide for Xively C Client

## Table of Contents
1. [Target Audience](#target-audience)
2. [Xively Client versions](#xively-client-versions)
3. [Supported Platforms](#supported-platforms)
4. [Board Support Package](#board-support-package)
5. [Porting the C Client](#porting-the-c-client)
    - [Preceding Step - A Native Build](#preceding-step---a-native-build)
    - [The Porting Step](#the-porting-step)
6. [Xively Client Features](#xively-client-features)


## Target Audience
This document is intended for Developers who have access to the source code of the Xively Python Client and to the Xively C Client. Developers who want to port the Xively C to a particular platform will find this to be the main guide for that endeavor.

Details of build, config flags and the location of platform-specific code are included in this document.

## Xively Client versions

#### C Client

The C Client is designed for low resources platforms. The single threaded implementation is non-blocking, which allows it to operate on NoOs platforms. It also unlocks the ability for processing outgoing and incoming messages concurrently.

An event system is included and can be used by client applications to scheduled publications at regular intervals without any platform-specific timers or tasks.

The client provides this functionality while remaining small enough to reside on low resource MCUs, consumes minimal memory, processor power and energy consumption.

The codebase is built against C99 standards.

For more information place see the Xively C Client user guide in the Xively Developer Center.

#### Python Client

The Python Client is meant for platforms where Python interpreter is available. Typically
where there's no need for low resource consumption and where battery power or energy consumption is not a bottleneck.

Python 2.7 and 3.4+ are both supported.

The development version of the Xively Python Client is in this repository under xi_client_py.  A release version of this same source-base is accessible in two ways:

- pip install xiPy OR
- public git repository: [xiPy](https://github.com/xively/xipy)

## Supported Platforms

The Xively Client has been deployed on many different devices already shipping in the IoT space running the various following platform stacks:

  - POSIX
  - Microchip
  - WMSDK
  - SimpleLink

Platforms above are officially supported and tested.

Porting the Xively C Client to new platforms is accelerated by the Board Support Package (BSP). The BSP implements a series of abstract platform dependencies abstracts the platform specific implementations required by the of Xively C Client logic to a well separated space to ease the focus on porting.

## Board Support Package

The Board Support Package (BSP) is the set of functions that must be tailored to a platform's specific secure networking, memory, random, and time implementations. All of these platform-specific function calls are channeled into a few files that reside under a single directory. It is this directory (xi\_client\_c/src/bsp) that you may focus your attention when compiling the client for your particular device or platform.

The BSP was designed to minimize the time it takes to make a port of the library. With it an engineer can ignore the MQTT codec or the non-blocking / asynchronous engine that resides in the main library source.

All of the BSP function declarations can be found under the _xi\_client\_c/include/bsp_
directory. It is here where you can find the doxygen documentation for each of the function calls that you will need to implement.

Functions are broken down by logical subsystem as follows:

### BSP modules

- BSP IO NET: Networking Stack Integration (xi\_bsp\_io\_net.h)
- BSP TLS: Transport Layer Security Integration (xi\_bsp\_tls.h)
- BSP MEM: Heap Memory Management (xi\_bsp\_mem.h)
- BSP RNG: Random Number Generator (xi\_bsp\_rng.h)
- BSP TIME: Time Function (xi\_bsp\_time.h)

### Reference implementations

Reference implementations are separated into two directories: *platform* and *tls*. The *platform* directory contains networking, memory, random and time implementations, the *tls* directory contains reference implementations for *wolfSSL* and *mbedTLS*.

#### BSP Platforms

Numerous platform implementations are provided for your reference. These are available
under directory *xi\_client\_c/src/bsp/platforms/[PLATFORM]*.

The POSIX implementation is probably the most familiar to most engineers.
Some of the other reference BSP implementations might be closer to the platform that
you're working on.

If your platform is not supported by one of these reference implementations then we recommended that you:

- copy an implementation into a new directory *xi\_client\_c/src/bsp/platform/[NEW_PLATFORM_NAME]*
- call `make` with parameter `XI_BSP_PLATFORM=[NEW_PLATFORM_NAME]`

#### BSP TLS

BSP TLS reference implementations for wolfSSL and mbedTLS can be found in a _xi\_client\_c/src/bsp/tls/[TLS]_ directory.

If neither wolfSSL nor mbedTLS fits your target platform the build system is open for further BSP TLS implementations. But before selecting your target TLS solution be sure it supports:

- asynchronous networking
- loading custom root CA certs from a buffer
- OCSP

To create a new BSP TLS solution:

- implement of the all BSP TLS API function found in *include/bsp/xi_bsp_tls.h*. As a template to follow it is adviced to check at least one of the wolfSSL or mbedTLS implementations.
- put this implementation into directory *src/bsp/tls/[NEW_TLS_LIBRARY_NAME]*
- copy file *make/mt-config/mt-tls-wolfssl.mk* to *make/mt-config/mt-tls-[NEW_TLS_LIBRARY_NAME].mk* and set the path variables inside according to the new TLS library's internal directory structure
- call `make` with parameter `XI_BSP_TLS=[NEW_TLS_LIBRARY_NAME]`

#### Chosing both: platform and TLS

Although a bare call on `make` defaults to `make XI_BSP_PLATFORM=posix XI_BSP_TLS=wolfssl` which is fine for MacOSX woth wolfSSL builds most probably during a porting process cross-compilation selecting both at the same time are required. For instance:

`make XI_BSP_PLATFORM=cc3200 XI_BSP_TLS=microSSL`

This command assumes directory *src/bsp/platform/cc3200* containing networking, memory, random and time implementations as well as directory *src/bsp/tls/microSSL* covering the TLS calls.


## Porting the C Client

Building a Xively C Client consists of building the Xively C Client _library_ and linking
it to the actual _application_. Obviously both of these steps need to be tailored to the target platform.

### Preceding Step - A Native Build

We recommend building the Xively Client on OSX or Linux before attempting to cross compile the sources to another target platform. This process could serve as an introduction to the basic architecture, tools and steps which will come handy during the final build onto the target platform.

#### Build System: make

The Xively C Client uses _make_ to build the executable for OSX and Linux. The main makefile is xi\_client\_c/Makefile. Internal and more detailed configurations can be found under directory xi\_client\_c/make which contains numerous make target (mt) files. There you can find the anchor mt files mt-config (processing the CONFIG
flags) and the mt-os files (processing the TARGET flags).

More informatoin about CONFIG and TARGET flags can be found in the espective sections below.

#### Build System: IDE Builds

Although the _make_ build system is suitable for OSX and Linux builds, some MCU platforms may not support it and prefer their own build Integrated Development Environment. For instance, the IAR toolchain uses their own graphical IDE.

The key to building the Xively Client with an IDE is to ensure that you have the correct Preprocessor Definitions defined and to import the correct set of files.

##### Preprocessor Definitions

Executing a makefile build on OSX or Linux with your desired config flags (see below) and then examining the make output is highly recommended.  By calling make with attribute 'MD=' you can reveal compiler and linker parameters, showing you which Preprocessor Definitions need to be migrated to your IDE environment.

Importing these definitions in an IDE varies considerably by IDE.  Some IDEs let you define these in a source or configuration file. Others require you to enter them as fields in a project configuration window.

##### C Sourcefiles

A vast majority of the sources in the Xively Client Library are applicable to all platforms, some source files might not be needed depending on your configuration choices.  Once again, like the preprocessor flags, we recommend running a make build with your desired configuration to determine which source files need to be added to your IDE builds, and which includes paths should be set.

#### First Build on MacOSX / Linux

Firing a bare `make` should be enough for an OSX build. It defaults to *posix* platform and to wolfSSL for a TLS solution. It also includes the download (from github repo) and build of the wolfSSL library.

Run command

`make`

Alternatively one can use mbedTLS instead of wolfSSL if preferred:

`make XI_BSP_TLS=mbedTLS`

Occasionally cleaning the build directories are necessary. This is done with the commands

`make clean`
    OR
`make clean_all`

##### Building the examples

`cd examples`

`make`

here if previously library was built with mbedTLS setting the XI_BSP_TLS variable is necessary to tell the linker which TLS lib to link agains:

`make XI_BSP_TLS=mbedTLS`


#### Build dependencies

- autotools: autoconf, automake, libtool
- cmake is needed by the integration test framework _cmocka_

On OSX these are available through _brew_:

    - brew install autoconf
    - brew install automake
    - brew install libtool
    - brew install cmake

#### Xively C Client Build Flags

The make system uses dash '-' separated string to define the TARGET and CONFIGuration
for a build. TARGET defines the build environment for specific platforms and toolchains.
The collection of CONFIG flags specify selections from a list of Xively Client modules and features to be included in the Xively C Client library.

_Default_ values will be chosen for you if you do not set the TARGET or CONFIG makefile variables.

The TARGET and CONFIG flags are enumerated below, and can be mixed and matched in any order and are separated by the - (minus sign / dash) character.

##### TARGET flags

A typical TARGET flag looks like this:

_osx-static-debug_

###### Platform flag

    - [ osx | linux | arm-linux ]   - selects the target platform

###### Output Library type

    - [ static | dynamic ]          - selects the output library type

###### Build Type

    - [ debug | release ]           - selects the build type

##### CONFIG flags

A typical CONFIG flag:

_posix\_io-posix\_fs-thread\_module-posix\_platform-xrm-tls-senml-control\_topic-memory\_limiter

###### Xively Client Feature flags

    - control_topic     - the control topic feature is in development.
                          Currently activating this feature has no behavioral benefit.
    - senml             - turns on SENML JSON serialization support for timeseries data. To maintain a small
                          footprint size we recommend turning this off (by removing senml flag from CONFIG).
    - threading         - turns on threading: application callbacks will be called on separate thread.
                          Not having this flag set application callbacks are called on the sole main
                          thread of the Xively C Client. Only POSIX implementation is available.
    - xrm               - obsolete, target Xively Server selector, must be always ON

###### File System flags

    - posix_fs          - POSIX implementation of File System calls
    - memory_fs         - an in-memory File System implementation
    - dummy_fs          - empty implementation of File System calls

    - expose_fs         - adds a new Xively C Client API function which allows external definition
                          of File System calls

###### Networking flags

    - posix_io          - POSIX implementation of Networking calls
    - microchip_io      - turns on Microchip Networking solution
    - mbed_io           - turns on an Mbed Networking solution
    - dummy_io          - empty implementation of Networking calls

###### Development flags

    - memory_limiter    - turns on memory limiting and monitoring functionality. The purpose of this configuration
                          option is to aid development processes by simulating low-memory environment behavior.
                          The memory monitor part is useful for memory consumption tracking and to hunt down memory
                          leaks.
    - mqtt_localhost    - the Client will connect to localhost MQTT server. The purpose of this configuration option
                          is to help development processes.
    - no_certverify     - lowers security by disabling TLS certificate verification
    - tls               - build with TLS module for securely connecting with TLS 1.2 to servers. The lack of this
                          flag will skip the TLS handshaking which may helpful in connecting to mosquitto to test,
                          but may not be supported on some secure services and is not recommend for production
                          deployment

###### Platform selector flags

    - posix_platform    - selects implementation for non-BSP time and non-BSP memory solutions
    - wmsdk_platform    - selects implementation for non-BSP time and non-BSP memory solutions

#### Example make command

By executing a simple 'make' under directory xi\_client\_c should be sufficient on OSX
and Linux. This will result using the default flags:

    - CONFIG: posix_io-posix_fs-posix_platform-xrm-tls-senml-control_topic-memory_limiter
    - TARGET: osx-static-debug

The result will be a development version Xively C Client static library with debug outputs,
secure TLS connection, POSIX networking and file system, artificial memory limits and memory guards
turned on and with SENML support for timeseries formatting.

The release version flags without SENML and memory limits may look like this:

    - CONFIG=posix_io-posix_fs-posix_platform-xrm-tls
    - TARGET=osx-static-release

For CI configurations please look at the file [.travis.yml](../../.travis.yml).

#### Example Xively Client Applications linked with the library

Example application binaries can be found under directory _xi\_client\_c/src/examples_.
These examples use of the Xively C Client library for connecting to Xively Servers,
subscribing to topics, and sending and receiving data.
The examples are command line applications libxively links against. They require a Xively-specific account-id, username, password and optional topicname to subscribe or publish on.

##### _XI_BSP_PLATFORM_

    - [ posix | microchip | wmsdk | ... ] - selects the bsp implementation from the available implementations, by default this flag is set to posix platform

##### _XI_BSP_TLS_

    - [ wolfssl | mbedtls ] - selects the TLS library which is used to provide secure connection with Xively servers, WolfSSL is set by default

### The Porting Step

#### Cross-compilation with the _make_ build system

The Xively C Client's makefile structure supports toolchain customization. Adapting
the existing makefile structure to a new toolchain consists of:

- assigning the cross-compiler, linker, archiver executables to certain variables.
  This can be done in a new _mt-PLATFORM_ file under directory _make/mt-os_. A new
  platform shall extend the available TARGET flags. This new flag is picked up by the
  file _make/mt-os/mt-os_ which needs this adaptation, too.
- providing all BSP module API sources by picking from existing reference BSP implementations or
  by writing new ones. These implementations may reside in new directory
  _src/bsp/platforms/PLATFORM_.
- adding these BSP sources to the build can be done with a _XI_BSP_PLATFORM_ Makefile flag.

There are two existing cross-compilation examples in the make build system which may
serve as template for a new one.

- WMSDK cross-compilation is defined in file _make/mt-os/mt-wmsdk_. Its corresponding
  TARGET flag is 'wmsdk'
- arm-linux cross-compilation is defined in file _make/mt-os/mt-arm-linux_ with TARGET
  flag 'arm-linux'

Both WMSDK and arm-linux cross-compilation targets are for OSX and linux.

#### Using the toolchain's build system

In the case that the _make_ build system is not supported by your platform toolchain then the source base and the compiler flags that you
set must  be transferred to the platform's own build system. This consists of
the following steps:

- populating all of the Xively C Client platform independent source and BSP functions in the
  platform specific build system. These are almost all files under directory
  _src/libxively_. Exceptions are the multiple implementations of the BSP functions for
  different platforms. E.g. _src/libxively/memory_ has the BSP stub
  right in the first level, but non-BSP platform specific implementations of the
  same function reside under subdirectories. The BSP source should be fed to the build system, but the non-BSP should not.
- collecting the proper BSP module API implementations covering all BSP modules
  (NET, MEM, RNG, TIME) into a directory and feeding this to build system as well
    - this might require you to "only" cherry-pick the proper implementations from the directory
      _xi\_client\_c/src/bsp/platforms_, depending on your platform.
    - or this might result in code writing: you will need to implement new definitions of the BSP
      function(s) that are specific to your platform and not compatible with the reference
      implementations provided in the Xively client repository
- compiler definitions also play essential role during the build process. These turn
  modules on and off. Following the CONFIG flags in file _mt-config_ as a guide, the
  compiler flags used in the "Preceding Step" can be looked up and fed to the
  platform specific build system as well. Another option is to echo the makefile build
  systemvariable XI_CONFIG_FLAGS during building on OSX to see which flags are set.


## Xively Client Features

#### TLS

The Xively C Client brings its own secure connection solution suitable for multiple
platforms. In order to provide TLSv1.2 secure communication with Xively servers -
Xively Client exposes BSP TLS with reference implementations against [WolfSSL's TLS](https://www.wolfssl.com/wolfSSL/Home.html) and [mbed TLS](https://tls.mbed.org/) libraries.

If you do not have a Xively Client compiled for you by Xively Professional Services, then you will need to compile a TLS implementation yourself.

You can obtain the WolfSSL sources here:  https://www.wolfssl.com/wolfSSL/Home.html
mbed TLS sources can be downloaded from here: https://tls.mbed.org/

#### Back-off logic

Xively Clients have some Distributed Denial of Service protection logic (DDoS) built into the library itself, which we call Back-off logic. This logic randomly spreads out reconnection attempts based on the number of recently failed connection attempts.  The goal is to prevent all of the devices choking the Xively Service in the rare case that there was a networking outage.

Xively Servers may ban clients which don't keep the rules of Back-off's delayed reconnection. To avoid banning your devices, please DO NOT alter this Back-off logic.

#### Control Topic

This is a Xively Client specific feature. Currently this consist of a single step: subscription
to a control topic at the beginning of the connection.  More features are coming soon.
