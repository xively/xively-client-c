# Porting Guide for Xively C Client

## Table of Contents
1. [Target Audience](#target-audience)
2. [Xively Client versions](#xively-client-versions)
3. [Supported Platforms](#supported-platforms)
4. [Board Support Package (BSP)](#board-support-package-bsp)
5. [Porting the C Client](#porting-the-c-client)
    - [Preceding Step - A Native Build](#preceding-step---a-native-build)
    - [The Porting Step](#the-porting-step)
6. [Xively Client Features](#xively-client-features)


## Target Audience
This document is intended for Developers who have access to the source code of the Xively C Client. Developers who want to port the Xively C to a particular platform will find this to be the main guide for that endeavor.

Details of build, config flags and the location of platform-specific code are included in this document.

## Xively Client versions

#### C Client

The C Client is designed for low resources platforms. The single threaded implementation is non-blocking, which allows it to operate on NoOs platforms. It also unlocks the ability for processing outgoing and incoming messages concurrently.

An event system is included and can be used by client applications to schedule publications at regular intervals without any platform-specific timers or tasks.

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

The Xively Client has been deployed on many different devices already shipping in the IoT space running the following platform stacks:

  - POSIX
  - Microchip
  - WMSDK
  - Texas Instruments CC3200

Platforms above are officially supported and tested.

Porting the Xively C Client to new platforms is accelerated by the Board Support Package (BSP). The BSP implements a series of abstract platform dependencies abstracts the platform specific implementations required by the of Xively C Client logic to a well separated space to ease the focus on porting.

## Board Support Package (BSP)

The Board Support Package (BSP) is the set of functions that must be tailored to a platform's specific secure networking, memory, random, and time implementations. All of these platform-specific function calls are channeled into a few files that reside under a single directory. It is this directory (xi\_client\_c/src/bsp) that you may focus your attention when compiling the client for your particular device or platform.

The BSP was designed to minimize the time it takes to make a port of the library. With it an engineer can ignore the MQTT codec or the non-blocking / asynchronous engine that resides in the main library source.

All of the BSP **function declarations** can be found under the _xi\_client\_c/include/bsp_
directory. It is here where you can find the doxygen documentation for each of the function calls that you will need to implement.

Functions are broken down by logical subsystem as follows:

### BSP modules

- BSP IO NET: Networking Stack Integration (xi\_bsp\_io\_net.h)
- BSP TLS: Transport Layer Security Integration (xi\_bsp\_tls.h)
- BSP MEM: Heap Memory Management (xi\_bsp\_mem.h)
- BSP RNG: Random Number Generator (xi\_bsp\_rng.h)
- BSP TIME: Time Function (xi\_bsp\_time.h)

### Reference implementations

Reference **function implementations** are separated into two directories: *platform* and *tls*. The *platform* directory contains networking, memory, random and time implementations, the *tls* directory contains reference implementations for *wolfSSL* and *mbedTLS*.

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

- implement all of the BSP TLS API functions found in *include/bsp/xi_bsp_tls.h*. As a template to follow it is advised to check at least one of the wolfSSL or mbedTLS implementations.
- put this implementation into directory *src/bsp/tls/[NEW_TLS_LIBRARY_NAME]*
- copy file *make/mt-config/mt-tls-wolfssl.mk* to *make/mt-config/mt-tls-[NEW_TLS_LIBRARY_NAME].mk* and set the path variables inside according to the new TLS library's internal directory structure
- call `make` with parameter `XI_BSP_TLS=[NEW_TLS_LIBRARY_NAME]`

#### Customising both: platform and TLS

Although a bare call on `make` defaults to `make XI_BSP_PLATFORM=posix XI_BSP_TLS=wolfssl` which is fine for MacOSX with wolfSSL builds most probably during a cross-compilation selecting both at the same time are required. For instance:

`make XI_BSP_PLATFORM=cc3200 XI_BSP_TLS=microSSL`

This command assumes directory *src/bsp/platform/cc3200* containing networking, memory, random and time implementations as well as directory *src/bsp/tls/microSSL* covering the TLS calls.


## Porting the C Client

Building a Xively C Client consists of building the Xively C Client *library*, the TLS library and linking
them to the actual *application*. Obviously these steps need to be tailored to the target platform.

### Preceding Step - A Native Build

We recommend building the Xively Client on OSX or Linux before attempting to cross compile the sources to another target platform. This process could serve as an introduction to the basic architecture, tools and steps which will come handy during the final build onto the target platform.

#### Build System: make

The Xively C Client uses _make_ to build for OSX and Linux. The main makefile is xi\_client\_c/Makefile. Internal and more detailed configurations can be found under directory xi\_client\_c/make which contains numerous make target (mt) files. There you can find the anchor mt files mt-config (processing the CONFIG
flags) and the mt-os files (processing the TARGET flags).

More informatoin about CONFIG and TARGET flags can be found in the respective sections below.

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

- autotools: autoconf, automake, libtool are needed by wolfSSL
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

    osx-static-debug

###### Platform flag

    - [ osx | linux | arm-linux ]   - selects the target platform

###### Output Library type

    - [ static | dynamic ]          - selects the output library type

###### Build Type

    - [ debug | release ]           - selects the build type

##### CONFIG flags

A typical CONFIG flag:

    posix_io-posix_fs-thread_module-posix_platform-tls-senml-control_topic-memory_limiter

###### Xively Client Feature flags

    - control_topic     - the control topic feature is in development.
                          Currently activating this feature has no behavioral benefit.
    - senml             - turns on SENML JSON serialization support for timeseries data. To maintain a small
                          footprint size we recommend turning this off (by removing senml flag from CONFIG).
    - threading         - turns on threading: application callbacks will be called on separate thread.
                          Not having this flag set application callbacks are called on the sole main
                          thread of the Xively C Client. Only POSIX implementation is available.

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

    - CONFIG: posix_io-posix_fs-posix_platform-tls-senml-control_topic-memory_limiter
    - TARGET: osx-static-debug

The result will be a development version Xively C Client static library with debug outputs,
secure TLS connection, POSIX networking and file system, artificial memory limits and memory guards
turned on and with SENML support for timeseries formatting.

The release version flags without SENML and memory limits may look like this:

    - CONFIG=posix_io-posix_fs-posix_platform-tls
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

To cross-compile the Xively C Client on MacOS or Linux to platform *np2000* the following command should be enough

        make PRESET=np2000

To mop up generated files type

        make PRESET=np2000 clean

But before - to make this possible - the following steps have to be taken.

Let's assume the new platform's name is np2000. And an early advise: as a rule of thumb if you are stuck examine and try to get help from already existing MCU config files like: *mt-cc3200*, *mt-wmsdk*, *mt-microchip*

#### Porting Checklist

- [x] create a new file *make/mt-os/mt-np2000*
    - include the common *mt* file

            include make/mt-os/mt-os-common

    - add lines

            XI ?= $(XI_OBJDIR)/libxively.a
            XI_COMPILER_FLAGS += -DNO_WRITEV // optional this turns off a wolfssl feature which is not available on micro platforms

    - define CC and AR pointing to target platform's compiler and archiver executables, e.g.:

            CC = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl
            AR = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/ti-cgt-arm_15.12.1.LTS/bin/armar

    - to add compiler flags append those to variable XI_COMPILER_FLAGS, like this:

            XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/simplelink/include

    - to add archiver flags append those to variable XI_ARFLAGS, like this:

            XI_ARFLAGS := r $(XI)

- [x] extend *mt-presets.mk* with the followings:

    - define a Xively Client feature and target configurations which will be used in this same file afterwards

            CONFIG_NP2000_MIN = bsp_np2000-memory_fs-tls
            TARGET_NP2000_REL = -static-release

    - define make system variables for PRESET *np2000*

            else ifeq ($(PRESET), np2000)
                CONFIG = $(CONFIG_NP2000_MIN)
                TARGET = $(TARGET_NP2000_REL)
                XI_BSP_PLATFORM = np2000
                XI_TARGET_PLATFORM = np2000

- [x] extend *mt-os* to let the build system pick up the *mt-np2000* config file

        XI_CONST_PLATFORM_NP2000 := np2000

        ifneq (,$(findstring $(XI_CONST_PLATFORM_NP2000),$(TARGET)))
            XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_NP2000)
        endif

- [x] provide BSP implementations for all modules:
    **networking** (*include/bsp/xi_bsp_io_net.h*),
    **memory** (*include/bsp/xi_bsp_mem.h*),
    **time**, (*include/bsp/xi_bsp_time.h*)
    **random** (*include/bsp/xi_bsp_rng.h*),
    into new files:
    - src/bsp/np2000/xi_bsp_io_net_np2000.c
    - src/bsp/np2000/xi_bsp_mem_np2000.c
    - src/bsp/np2000/xi_bsp_rng_np2000.c
    - src/bsp/np2000/xi_bsp_time_np2000.c

    These files should contain all implementations for function declarations in files
    Hint: to reach successful build just create the files and implement all the BSP API functions with **empty body**.

- [x] select TLS implementation
    - default is wolfssl, if wolfssl fits the needs then nothing to do here
    - to select a different TLS lib add

            XI_BSP_TLS=mbedtls

        or

            XI_BSP_TLS=myTLSlibrary

        to the make commandline like this:

            make PRESET=np2000 XI_BSP_TLS=myTLSlibrary

    - for wolfssl and mbedtls the BSP TLS implementations are available in files: *src/bsp/tls/wolfssl/xi_bsp_tls_wolfssl.c* and *src/bsp/tls/mbedtls/xi_bsp_tls_mbedtls.c*
    - if you chose a third one: *myTLSlibrary* then you have to write your own implementation
        - create a file *src/bsp/tls/myTLSlibrary/xi_bsp_tls_myTLSlibrary.c* and implement all function declared in file *include/bsp/xi_bsp_tls.h*
        - as samples you can follow the two existing implementations: wolfssl and mbedtls
    - create file *make/mt-config/mt-tls-myTLSlibrary.mk* and fill in with content similar to *mt-tls-wolfssl.mk* or *mt-tls-mbedtls.mk*. This lets know the build system the include directoy, the binary directory, the static libraries to link against and config flags of the custom TLS library.
    - you have to also provide a script *xively-client-c/src/import/tls/download_and_compile_myTLSlibrary.sh* which downloads the source of the custom TLS library and builds it. As a sample to follow look at the two already existing solutions: *download_and_compile_wolfssl.sh* and *download_and_compile_mbedtls.sh* in the same directory.

#### Using the toolchain's build system

In the case that the _make_ build system is not supported by your platform toolchain then the source base and the compiler flags that you set must be transferred to the platform's own build system. This consists of
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

#### Back-off logic

Xively Clients have some Distributed Denial of Service protection logic (DDoS) built into the library itself, which we call Back-off logic. This logic randomly spreads out reconnection attempts based on the number of recently failed connection attempts.  The goal is to prevent all of the devices choking the Xively Service in the rare case that there was a networking outage.

Xively Servers may ban clients which don't keep the rules of Back-off's delayed reconnection. To avoid banning your devices, please DO NOT alter this Back-off logic.

#### Control Topic

This is a Xively Client specific feature. Currently this consist of a single step: subscription
to a control topic at the beginning of the connection.  More features are coming soon.
