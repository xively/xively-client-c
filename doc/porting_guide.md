# Porting Guide for Xively C Client
##### Copyright (c) 2003-2018, LogMeIn, Inc.

## Table of Contents
1. [Target Audience](#target-audience)
2. [C Client Overview](#c-client-overview)
3. [Supported Platforms](#supported-platforms)
4. [Board Support Package (BSP)](#board-support-package-bsp)
5. [Porting the C Client](#porting-the-c-client)
    - [Suggested First Step - A Native Build](#suggested-first-step---a-native-build)
    - [Porting the Xively C Client to Your Platform](#porting-the-xively-c-client-to-your-platform)
6. [Further Reading](#further-reading)


## Target Audience
This document is intended for Embedded Developers who have access to the source code of the Xively C Client and want to run it on their custom platform.

Details of the Xively C Client build steps, build configuration flags, and the location of the functions needed to port the library to new platforms are included in this document.

For information about the Xively Python client, please visit its public git repository: [xiPy](https://github.com/xively/xipy).

## C Client Overview

The Xively C Client is designed for low resources platforms. The single threaded implementation is non-blocking, which allows it to operate on NoOs platforms. The implementation also supports seamless concurrent processing of outgoing and incoming messages.

An event system is included in the client which can be used by your client application as well. The client's event system is a platform independant implementation that doesn't require porting to any platform-specific timers or tasks.

The client provides this functionality while remaining small and efficient enough to fit into low-resource devices, with minimal footprint and processing power requirements.

The codebase is built against C99 standards.

For more information please see the Xively C Client User Guide: `doc/user_guide.md`.


## Supported Platforms

The Xively Client has been deployed on many different devices shipping in the IoT space running the following platform stacks. The following is a catalog of our tested ports:

  - POSIX systems (OSX, Linux)
  - Microchip TCP SDK
  - Texas Instruments CC3200
  - Texas Instruments CC3220SF
  - STM3241G-EVAL
  - STM32 NUCLEO-F429ZI Ethernet
  - STM32F4 CLOUD-JAM RushUp WiFi
  - STM32 NUCLEO-F401RE *(with IDW01M1 WiFi)*
  - STM32 NUCLEO-L476RG *(with IDW01M1 WiFi)*
  - STM32 NUCLEO-L053R8 *(with IDW01M1 WiFi)*
  - Marvell
  - wmsdk

Porting the Xively C Client to new platforms is accelerated by the Board Support Package (BSP).

## Board Support Package (BSP)

The Board Support Package (BSP) is the well-defined set of functions that the Xively Client invokes to interact with a platform's specific networking, file IO, [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security), memory management, random number generator, time and firmware management SDKs.

We've organized the sources of the Xivey Client so that your BSP implementation resides in a few files of the directory `src/bsp`.  You should focus your attention here when porting the Xively Client to your device SDK, safely ignoring the MQTT codec or the non-blocking / asynchronous engine that resides in the rest of the main library source.

All of the BSP **function declarations** can be found under the `include/bsp`
directory. Doxygen documentation for these functions can be found in the `doc/doxygen/bsp/html/index.html`.

BSP Functions are broken down by logical subsystems as follows:

### BSP Modules

#### Optional
- BSP FWU: Firmware Update Inegration (`include/bsp/xi_bsp_fwu.h`)
- BSP IO FS: File System Integration for Secure File Transfer (SFT) (`include/bsp/xi_bsp_io_fs.h`)

#### Required
- BSP IO NET: Networking Stack Integration (`include/bsp/xi_bsp_io_net.h`)
- BSP TLS: Transport Layer Security Integration (`include/bsp/xi_bsp_tls.h`)
- BSP MEM: Heap Memory Management (`include/bsp/xi_bsp_mem.h`)
- BSP RNG: Random Number Generator (`include/bsp/xi_bsp_rng.h`)
- BSP TIME: Time Function (`include/bsp/xi_bsp_time.h`)


### Reference Implementations

Reference **function implementations** are separated into two directories: `src/bsp/platform` and `src/bsp/tls`.

The *platform* directory contains reference BSP implementations for networking, file IO, memory management, random number generator, time and firmware management. The *tls* directory contains reference implementations for *wolfSSL* and *mbedTLS* libraries which supply secure TLS v1.2 connections over TCP/IP.

#### BSP Platforms

Numerous platform implementations are provided for your reference. These are available
under directory `src/bsp/platforms/[PLATFORM]`.

The POSIX implementation is probably the most familiar to most engineers. 
However, some of the other reference BSP implementations might be closer to the platform that you're working on, if you're working in the embedded device space.

If your platform is not supported by one of these reference implementations then we recommended that you:

- copy an implementation into a new directory `src/bsp/platform/[NEW_PLATFORM_NAME]`
- call `make` with parameter `XI_BSP_PLATFORM=[NEW_PLATFORM_NAME]`

#### BSP TLS

BSP TLS reference implementations for wolfSSL and mbedTLS can be found in a `src/bsp/tls/[TLS]` directory.

If neither wolfSSL nor mbedTLS fits your target platform the build system can be configured to use other BSP TLS implementations.

Before selecting another TLS solution other than those mentioned above, be sure that it supports:

- asynchronous networking
- accepting Root CA certs to verify Xively's Server certificate
- Online Certificate Status Protocol [(OCSP)](https://en.wikipedia.org/wiki/Online_Certificate_Status_Protocol) or an actively maintained [Certificate Revocation List](https://en.wikipedia.org/wiki/Certificate_revocation_list)
- The added functionality of [OCSP Stapling](https://en.wikipedia.org/wiki/OCSP_stapling) is prefered.

To create a new BSP implementation for TLS:

- implement all of the BSP TLS API functions found in `include/bsp/xi_bsp_tls.h`. It is advised to reference at least one of the wolfSSL or mbedTLS implementations throughout this process to guide your development.
- put this implementation into directory `src/bsp/tls/[NEW_TLS_LIBRARY_NAME]`
- copy the file `make/mt-config/mt-tls-wolfssl.mk` to `make/mt-config/mt-tls-[NEW_TLS_LIBRARY_NAME].mk` and set the path variables inside according to the new TLS library's internal directory structure
- call `make` with parameter `XI_BSP_TLS=[NEW_TLS_LIBRARY_NAME]`

#### Customizing Both: Platform and TLS BSPs

Invoking `make` without parameters will silently default the configuration to `make XI_BSP_PLATFORM=posix XI_BSP_TLS=wolfssl`.

This is fine for building on OSX or Linux/Unix using wolfSSL for a TLS library.  However, for cross-compilation you will most likely need to define both parameters explicitly. For instance:

	`make XI_BSP_PLATFORM=cc3200 XI_BSP_TLS=microSSL`

*Note: microSSL is a fictional TLS library we've conjured-up as a quick custom example.*

This command assumes directory `src/bsp/platform/cc3200` containing networking, memory, random and time implementations as well as directory `src/bsp/tls/microSSL` covering the TLS calls.


## Porting the C Client

Building a Xively C Client consists of building the Xively C Client *library*, the TLS library and linking them to the actual *application*. Obviously these steps need to be tailored to the target platform.

### Suggested First Step - A Native Build

We recommend building the Xively Client on OSX or Linux/Unix before attempting to cross compile the sources to another target platform. This process could serve as an introduction to the basic architecture, tools and building steps of the Xively Client source. This basic knowledge will be handy later when doing a full cross-compiled port of the software to an embedded device.

#### Build System: make

The Xively C Client uses _make_ to build for OSX or Linux/Unix. The main makefile is in the root directory of the repository.

Further build configurations can be found under directory `make/` which contains numerous make target (mt) files which are included by the main makefile. Here `make/mt-config/mt-config.mk` and `make/mt-os/mt-os.mk` files should be your main focus.  These are always included by the main makefile, and further include optional mt files based on the CONFIG and TARGET flags you specify when building.

For instance, the `make/mt-os/mt-cc3200.mk` file is included by `make/mt-os/mt-os.mk` when building for the TI CC3200.

More information about CONFIG and TARGET flags can be found in the respective sections below.

Additionally you can run with `make MAKE_DEBUG=1` defined and our makefiles will log many of the source, include and flag variables that it's compiling so you can see where certain flags reside.


#### Build System: IDE Builds

Although the _make_ build system is suitable for OSX or Linux/Unix builds, some MCU platforms may not support building via make. Often these devices supply their own Integrated Development Environment (IDE). For instance, the IAR toolchain uses their own graphical IDE.

The key to building the Xively Client with an IDE is to ensure that you have the correct Preprocessor Definitions defined and to import the correct set of files.

Here's a a suggestion on how to determine which flags and files you should import:

##### Preprocessor Definitions

Executing a makefile build on OSX or Linux/Unix with your desired config flags (see below) and then examining the make output is highly recommended.

Additionally, by calling make with attribute 'MD=' you can reveal compiler and linker parameters, showing you which Preprocessor Definitions need to be migrated to your IDE environment.

Importing these definitions in an IDE varies considerably by IDE.  Some IDEs let you define these in a source or configuration file. Others require you to enter them as fields in a project configuration window.

##### C Sourcefiles

A vast majority of the sources in the `src/libxively` directory Xively Client Library are applicable to all platforms, with the platform sepecific files residing in the `src/bsp` directory.

However some source files might not be needed depending on your configuration choices.  Once again, like the preprocessor flags, we recommend running a make build with your desired configuration to determine which source files need to be added to your IDE builds, and which includes paths should be set.

#### First Build on OSX or Linux/Unix

Firing a bare `make` should be enough for an OSX build. It defaults to *posix* platform and to wolfSSL for a TLS solution. It also includes the download (from github repo) and build of the wolfSSL library.

Run command

	make

Alternatively one can use mbedTLS instead of wolfSSL if preferred:

	make XI_BSP_TLS=mbedtls

Occasionally cleaning the build directories are necessary. This is done with either of the following commands:

	make clean
	make clean_all

##### Building the Examples
From the base directory of the repository:

	cd examples
	make

Here if the library was previously built with the mbedTLS setting then the XI_BSP_TLS variable is necessary.  Otherwise you will get link errors.

	make XI_BSP_TLS=mbedTLS


#### Build Dependencies

- autotools: autoconf, automake, libtool are needed by wolfSSL
- cmake is needed by the integration test framework _cmocka_

On OSX these are available through `brew`:

    - brew install autoconf
    - brew install automake
    - brew install libtool
    - brew install cmake

On Ubuntu these are available through `apt-get`:

	- sudo apt-get update
	- sudo apt-get install autoconf
	- sudo apt-get install autotools-dev
	- sudo apt-get install libtool
	- sudo apt-get install cmake


#### Xively C Client Build Flags

The make system uses dash '-' separated string to define the TARGET and CONFIGuration for a build.

TARGET defines the build environment for specific platforms and toolchains.

CONFIG flags specify selections from a list of Xively Client modules and features to be included in the Xively C Client library.

_Default_ values will be chosen for you if you do not set the TARGET or CONFIG makefile variables.

The TARGET and CONFIG flags are enumerated below, and can be mixed and matched in any order.  Each flag is separated by the - (minus sign / dash) character.

##### TARGET Flags

A typical TARGET flag looks like this:

    osx-static-debug

###### Platform Flag

    - [ osx | linux | arm-linux ]   - selects the target platform

###### Output Library Type

    - [ static | dynamic ]          - selects the output library type

###### Build Type

    - [ debug | release ]           - selects the build type

##### CONFIG Flags

A typical CONFIG flag:

	posix_fs-posix_platform-tls_bsp-senml-control_topic-memory_limiter

###### Xively Client Feature flags

    - control_topic        - turing on this feature makes the Client establish a control
                             topic channel to the Xively Services. This channel is the
                             transport layer for background Xively Service tasks like
                             Secure File Transfer and Firmware Update.
                             
    - secure_file_transfer - enables the Secure File Transfer and Firmware Update features.
                             SFT keeps certain files up-to-date by defining a file
                             set in the API function 'xi_set_updateable_files'. 
                             The control_topic flag must be turned on as well.
                             
    - senml                - turns on SENML JSON serialization support for timeseries
                             data. To maintain a small footprint size we recommend
                             turning this off (by removing senml flag from CONFIG).

    - threading            - POSIX only. This causes pub, sub, and connection callbacks
                             to be called on separate thread. Not having this flag set,
                             application callbacks are called on the sole main thread of
                             the Xively C Client.

###### File System flags

    - posix_fs          - POSIX implementation of File System calls
    - memory_fs         - an in-memory File System implementation
    - dummy_fs          - empty implementation of File System calls
    - expose_fs         - adds a new Xively C Client API function which allows external definition
                          of File System calls.  This will be part of our BSP system in the future.

###### Development flags

    - memory_limiter    - turns on memory limiting and monitoring functionality.
						  The purpose of this configuration is to aid development processes by simulating low-memory environment behavior.
                          The memory monitor part is useful for memory consumption tracking and to hunt down memory
                          leaks.  When a leak occurs a stack trace of the initial allocation will be logged.
    - mqtt_localhost    - the Client will connect to localhost MQTT server. The purpose of this configuration option
                          is to help development processes.
    - no_certverify     - lowers security by disabling TLS certificate verification
    - tls_bsp           - the Client will use third party TLS 1.2 implementations to encrypt data before it's sent over network sockets.
					      The lack of this flag will skip the TLS handshaking.  This be may helpful in connecting to mosquitto to test,
                          but may not be supported on some secure services and is not recommend for production
                          deployment.  The Xively Gateway will not accept connections without TLS.

###### Platform selector flags

    - posix_platform    - selects implementation for non-BSP time and non-BSP memory solutions
    - wmsdk_platform    - selects implementation for non-BSP time and non-BSP memory solutions

The Platform Selector configurations will eventually be sunset.  Currently these configure the build system to include Critical Section implementations for invoking callbacks on new threads.

We suggest defining `posix_platform` and omit `threading` from your CONFIG options when building for custom platforms.  `threading` is currently ommitted by default.

For more information about thread safe callback support please see the Xively C Client User Guide: `doc/user_guide.md`.

#### Example make Command

By executing a simple 'make' under the base directory of the repository should be sufficient on OSX or Linux/Unix. This will result in build configuration with the following default flags:

    - CONFIG: posix_fs-posix_platform-tls_bsp-senml-control_topic-memory_limiter-secure_file_transfer
    - TARGET: osx-static-release

The result will be a release version Xively C Client static library with debug outputs, secure TLS connection, POSIX networking and file system, artificial memory limits and memory guards turned on, SENML support for timeseries formatting, and SFT features enabled.

The development version flags without SENML and memory limits may look like this:

    - CONFIG=posix_io-posix_fs-posix_platform-tls_bsp-control_topic-secure_file_transfer
    - TARGET=osx-static-debug

For CI configurations please look at the file [.travis.yml](../../.travis.yml).

#### Example Xively Client Applications Linked with the Library

Application binaries and source can be found under the directory `examples/`. 

These examples use the Xively C Client Library for connecting to Xively Servers, subscribing to topics, publishing data, and receiving data.  They can be built on POSIX by running `make` in the `examples/` directory.

The examples are command line applications that link against the Xively C Client library. They require a Xively-specific account-id, username, password and optional topic name to subscribe or publish on.

### Porting the Xively C Client to Your Platform

#### Let's Begin

As a rule of thumb if you are stuck or confused, then please use existing platform config files like: `make/mt-os/mt-cc3200.mk` or `make/mt-os/mt-linux.mk` files as examples.

The following is written assuming a new ficitious platform named the **np2000**.


#### The Goal: Cross Compilation with the _make_ Build System

The ideal goal to cross-compile the Xively C Client on OSX or Linux/Unix to your platform.  For the sake of this tutorial, let's pretend that your platform is a new fictitious platform: The *NP2000*.  Your platform sounds awesome.

Our goal is construct the following command to build the Xively Client for the NP2000:

  	make PRESET=np2000

To mop up generated files type

    make PRESET=np2000 clean

To make this possible, the following steps have to be taken.


#### Porting Checklist

- [x] create a new file `make/mt-os/mt-np2000.mk`
    - include the common *mt* file

            include make/mt-os/mt-os-common.mk

    - add the lines

            XI ?= $(XI_BINDIR)/libxively.a
            XI_COMPILER_FLAGS += -DNO_WRITEV // optional. this turns off a wolfssl feature which is not available on micro platforms

    - define CC and AR and point them to your compiler and archiver executables, e.g.:

            CC = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/ti-cgt-arm_15.12.1.LTS/bin/armcl
            AR = $(XI_CC3200_PATH_CCS_TOOLS)/compiler/ti-cgt-arm_15.12.1.LTS/bin/armar

    - add compiler flags by appending them to variable XI_COMPILER_FLAGS. We don't know what your flags would be, but perhaps they might look like this:

            XI_COMPILER_FLAGS += -I$(XI_CC3200_PATH_SDK)/simplelink/include
            XI_COMPILER_FLAGS += -mv7M4

    - add archiver flags by appending them to the variable XI_ARFLAGS, like this:

            XI_ARFLAGS := r $(XI)

- [x] alter `make/mt-config/mt-presets.mk` by adding the following details for your new platform:

    - define a Xively Client feature and target configurations which will be used in this same file afterwards

            CONFIG_NP2000_MIN = memory_fs-tls_bsp
            TARGET_NP2000_REL = -static-release

    - define make system variables for PRESET *np2000*

            else ifeq ($(PRESET), np2000)
                CONFIG = $(CONFIG_NP2000_MIN)
                TARGET = $(TARGET_NP2000_REL)
                XI_BSP_PLATFORM = np2000
                XI_TARGET_PLATFORM = np2000

- [x] extend `make/mt-os/mt-os.mk` to check the TARGET make parameter for 'np2000' and include the `make/mt-os/mt-np2000.mk` config file when the np2000 TARGET is found:

        XI_CONST_PLATFORM_NP2000 := np2000

        ifneq (,$(findstring $(XI_CONST_PLATFORM_NP2000),$(TARGET)))
            XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_NP2000)
        endif

- [x] provide BSP implementations for all modules.

	- Create the following source files:
		- `src/bsp/np2000/xi_bsp_fwu_np2000.c`
		- `src/bsp/np2000/xi_bsp_io_fs_np2000.c`
		- `src/bsp/np2000/xi_bsp_io_net_np2000.c`
		- `src/bsp/np2000/xi_bsp_mem_np2000.c`
		- `src/bsp/np2000/xi_bsp_rng_np2000.c`
		- `src/bsp/np2000/xi_bsp_time_np2000.c`
 	- in these sourcefiles, define the functions declared in these corresponding Xively C Client BSP headers:
 		- **firmware update notifications** (`include/bsp/xi_bsp_fwu.h`)
 		- **file storage** (`include/bsp/xi_bsp_io_fs.h`)
 		- **networking** (`include/bsp/xi_bsp_io_net.h`)
 		- **memory** (`include/bsp/xi_bsp_mem.h`)
 		- **time** (`include/bsp/xi_bsp_time.h`)
 		- **random** (`include/bsp/xi_bsp_rng.h`)

    **Hint**: to attain a simple successful build just create the files and implement all the BSP API functions with an **empty body**.  You may use files under `src/bsp/platform/dummy` as an example empty starting point. While this won`t execute properly, it should at least build, link and run.

- [x] select TLS implementation
    - the default selection is wolfssl.  If wolfssl fits your needs then there's nothing to do here.
    - to select a different TLS lib add

            XI_BSP_TLS=mbedtls

        or

            XI_BSP_TLS=myTLSlibrary

        to the make commandline like this:

            make PRESET=np2000 XI_BSP_TLS=myTLSlibrary

    - for wolfssl and mbedtls the BSP TLS implementations are available in files: `src/bsp/tls/wolfssl/xi_bsp_tls_wolfssl.c` and `src/bsp/tls/mbedtls/xi_bsp_tls_mbedtls.c`
    - if you chose a third one: *myTLSlibrary* then you have to write your own implementation
        - create a file `src/bsp/tls/myTLSlibrary/xi_bsp_tls_myTLSlibrary.c` and implement all functions declared in `include/bsp/xi_bsp_tls.h`
        - as samples you can follow the two existing implementations: wolfssl and mbedtls
    - create a file `make/mt-config/mt-tls-myTLSlibrary.mk` and fill in with content similar to `make/mt-config/mt-tls-wolfssl.mk` or `make/mt-config/mt-tls-mbedtls.mk`. This is where you would defined the include directory, the static library directory, the static library file to link against and the config flags of the custom TLS library.

#### Using a Third Party IDE (non-make)

In the case that the _make_ build system is unavailable for your platform you can migrate the
Xively C Client build environment to your platform's own Integrated Development Environment (IDE).

While we cannot completely predict how this process would work for every IDE and toolchain, here are some suggested steps to follow when working through this process:

- import all of the Xively C Client source files in the `src/libxively` directory into your project workspace. This is where all of the Xively Client's platform independent code resides.
	- Some of these files might not be required depending on your desired library configuration.  You could prune some files later by comparing the fileset against a mac / linux build log.
- import all of the source files in one of the subdirectories of `src/bsp/platform`.
  The Xively C Client BSPs contain all of the missing hooks which tie the platform independent
  code to a particular device. Complete implementations should include a source file for each
  of the BSP subsystems (networking, file IO, memory, time, firmware, etc).
	- NOTE: The Xively C Client contains reference BSP implementations for POSIX, TI CC3200, and some ST32F4 Nucleo platforms. We also have partial implementations for specific networking APIs.
    Modules from these 'incomplete' BSP implementations could be used as substitutes. For instance, on devices that mirror POSIX completely except for networking, like Simplelink, the networking module from `src/bsp/platform/posix` could be ovewritten with the one from `src/bsp/platform/simplelink_incomplete`.
- import one of the BSP TLS implementations in `src/bsp/tls`.  Currently we provide two different TLS BSP implementations: WolfSSL or mbedTLS.

- alter the include path for the toolchain as follows:
	- add all of the directories in `src/libxively` to your include path
	- add the `include` directory to your include path
	- add the `include/bsp` directory to your include path
 	- add any corresonding preprocessor defintions to toggle on/off Xively client features.
  
Using the CONFIG flags in file `make/mt-config/mt-config.mk` as a guide, the compiler flags used in the "Preceding Step" can be looked up and fed to the platform specific build system as well. Another option is to echo the makefile build system variable XI_CONFIG_FLAGS during building on OSX to see which flags are set.

## Further Reading
For more informationa about the Xively Client, check these other documents in the [github repository](https://github.com/xively/xively-client-c):


### README.md

Contains general information about the file structure of the sources, how to build on OSX and Linux,  a general overview of Security.

### doc/UserGuide.md

Contains an in-depth description of the Client Design and IoT Features, including MQTT logic, Event System, Back-Off Logic, Platform Security Requirements, etc.

### doxygen/api

Contains the function specifications for communicating with the Xively C Client from our application.  Functionality such as Connect, Subscribe, and Publish are outlined here.


### doxygen/bsp

Contains the declarations and documentations of the abstracted Board Support Package functions that you will need to implement to couple the Xively C Client to your hardware platform.

