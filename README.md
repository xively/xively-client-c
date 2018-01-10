# Xively C Client
___
##### Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.

This is part of the Xively C Client library, it is licensed under the BSD 3-Clause license.

For more information please read our License.md file located in the base directory of the [Xively C Client repository](https://github.com/xively/xively-client-c).

## What is the Xively C Client?

The Xively C Client is an easy to port, open-source C client that connects low-end IoT devices to Xively services. Low resource use is achieved by implementing a [cooperative multitasking pattern](https://en.wikipedia.org/wiki/Cooperative_multitasking) providing a single-threaded library with non-blocking behaviour. Portability is achieved via a Board Support Package (BSP) which puts platform-specific functionality into a well-defined series of functions and directories. Specific Xively Client BSP modules are: Networking, Transport Layer Security (TLS), Memory Management, Random Number Generator and Time.

## Stability and QA

20 different combinations of compiler and feature sets are continuously built by [Travis CI][travis-private-repo-page]. 58 functional, 23 integration and 256 unit tests are executed after each build. Tested with two TLS libraries: [WolfSSL](https://www.wolfssl.com) and [mbedTLS](https://tls.mbed.org). The library is open  for further TLS libraries as well via its TLS BSP.

Branch      | Build status
------------|-------------
master      | [![Build Status][travis-private-repo-icon-master]][travis-private-repo-page]
development | [![Build Status][travis-private-repo-icon-development]][travis-private-repo-page]

[travis-private-repo-page]: https://travis-ci.com/xively/xively-client-c
[travis-private-repo-icon-master]: https://travis-ci.com/xively/xively-client-c.svg?token=HazV3LTmDbqtLdnDxZX2&branch=master
[travis-private-repo-icon-development]: https://travis-ci.com/xively/xively-client-c.svg?token=HazV3LTmDbqtLdnDxZX2&branch=development

## How to get the sources?
Sources can be obtained by pulling from the master branch at our Xively Client github repository:

[https://github.com/xively/xively-client-c](https://github.com/xively/xively-client-c)

## Directory structure

- **bin** - executables resulting from a build
- **doc** - documentation: doxygen API, User Guide, Porting Guide
- **examples** - example sources with their own Makefiles. After building in this directory with make, the examples' executables will reside here as well.
- **include** - header files of Xively C Client's API. You must add this directory to the include path when compiling and linking against a Xively C Client static library.
- **include/bsp** - header files of the Board Support Package (BSP). Functions here shall be implemented during a porting process. Implementation of these functions contains all of the platform specific code needed tie the Xively C Client to a particular platform.
- **include_senml** - header files of the Xively C Client's SenML API. This directory should be added to the include path when building an application that uses SenML to format timeseries messages.
- **licenses** - licenses of 3rd party software used by the Xively C Client
- **make** - build system configuration files
- **obj** - object files generated during a build
- **proj** - project files assisting development of the Xively C Client
- **res** - resource files (e.g. trusted root CA certificates)
- **src** - the source files of the Xively C Client

## How to Build?

The Xively C Client's preferred build environment is make with gcc on a POSIX platform, though it has been cross-ported to other platforms via toolchains and specific IDEs.

We recommend building the Xively Client on OSX or Linux before attempting to cross compile the sources to another target. This process could serve as an introduction to the basic architecture, tools and steps which will come handy during the final build onto the target platform.

### Build prerequisites

- autotools: autoconf, automake, libtool is needed by the default TLS library: WolfSSL
- cmake is needed by the integration test framework _cmocka_

On OSX these are available through _brew_:

    - brew update
    - brew install autoconf
    - brew install automake
    - brew install libtool
    - brew install cmake

 On Ubuntu these are available through _apt-get_:

    - sudo apt-get update
    - sudo apt-get install autoconf
    - sudo apt-get install autotools-dev
    - sudo apt-get install libtool
    - sudo apt-get install cmake


### Building a TLS static library

By default the Xively C Client library is built with secure connection support and therefore requires a 3rd party TLS implementation to link against. The build environment defaults to use wolfSSL, but it can be configured to use mbedTLS by setting variable XI_BSP_TLS in the make command like this:

        make XI_BSP_TLS=mbedtls

The default wolfssl will run the script xively-client-c/src/import/tls/download_and_compile_wolfssl.sh, mbedtls will run xively-client-c/src/import/tls/download_and_compile_mbedtls.sh

For information on how to run this script please see the [Security](#security) section, then continue with Building the library step below.

### Building the library is as simple as executing the following command in the terminal

    make

### Building the examples

Examples require both the Xively C Client library and a TLS library to be built beforehand.

    cd examples
    make

Execution of the examples requires Xively accountId, username, password and usually an mqtt topic to communicate over. These credentials can be obtained via your Device Details page of the [Xively Connected Product Management (CPM)](https://app.xively.com/login).  If you don't yet have a Xively Account, then you may [Register For One Here](https://app.xively.com/register).

For a secure connection a file named xi_RootCA_list.pem is also required to be placed into the current working directory (CWD) of the example, although this is done automatically by the build system.

### Building and executing the tests

    make tests

By default the build process for ```tests``` includes the test execution as the final step. Although one can execute the tests manually as well by

    cd xi_client_c/bin/osx/tests
    ./xi_utests
    ./xi_itests

### Cross-compilation

For cross-compilation please see the porting guide under ```doc/``` directory. But in short it consists of

- extending the build system with a new cross-compilation preset in the file ```xively-client-c/make/mt-config/mt-presets.mk```
- a Board Support Package (BSP) implementation that you have written for your platform in the directory ```xively-client-c/src/bsp/platform/TARGETPLATFORM```
- a TLS library built for the target platform in the directory ```xively-client-c/src/bsp/tls/TARGETTLSSOLUTION```

## Security

The Xively C Client supports secure connection by utilizing a 3rd party TLS library service. The Xively C Client is built and tested against two TLS libraries: [WolfSSL](https://www.wolfssl.com) and [mbedTLS](https://tls.mbed.org). The Xively C Client repository itself does not directly contain TLS libraries in any forms but offers a script to download and build either WolfSSL and mbedTLS library on OSX and Linux.  Please  see the *src/import/tls* directory.

The Xively C Client is open to use any further TLS library solution through its BSP TLS API, please see the porting guide in ```doc/```. If you choose a library other than mbedTLS or WolfSSL then ensure that it supports async I/O, the loading of custom root CA certificates, and OCSP.

The Xively C Client library can be built without TLS support. The primary purpose for a non-TLS build is to test against a localhost non-TLS mqtt server like the [mosquitto](http://mosquitto.org). Turning off TLS is strongly discouraged for release versions. For turning off secure connection use the PRESET=POSIX_UNSECURE_REL in the make command:

    make PRESET=POSIX_UNSECURE_REL

For further fine tuning feature settings please see the porting guide in the ```doc/``` directory.

## Support

Any issues that appear to be a bug in the library code, examples or documentation,
should be submitted via GitHub and will get fixed once confirmed and understood.
Feature requests should be submitted via GitHub too.

## Contributing

We use GitHub fork/pull-request model and encourage users to contribute
changes back. The general guideline is to submit no more than one feature
in a single pull-request and each individual commit should contain only
related changes.

## Further readings

See more details in directory ```doc/```

- *doxygen/* - doxygen generated docs
- *porting_guide.md* - a detailed description to aid porting process
- *user_guide.md* - a detailed descript to the Xively C Client
- *[CC3200 tutorial](https://developer.xively.com/docs/ti-cc3200)* - a tutorial for Texas Instruments CC3200 board
