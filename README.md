# Xively C Client
___
##### Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.

This is part of the Xively C Client library, it is licensed under the BSD 3-Clause license. 

For more information please read our [License.md](License.md) file located in the [Xively C Client repository](https://github.com/xively/xively-client-c).



## What is the Xively C Client?

The Xively C Client is an easy to port, open-source C client that connects low-end IoT devices to Xively services. Low resource use is achieved by implementing a [cooperative multitasking pattern](https://en.wikipedia.org/wiki/Cooperative_multitasking) providing a single-threaded library with non-blocking behaviour. Portability is achieved via a Board Support Package (BSP) which puts platform-specific functionality into a well-defined series of functions and directories. Specific Xively Client BSP modules are: Networking, Transport Layer Security (TLS), Memory Management, Random Number Generator and Time.

## Stability and QA

20 different combinations of compiler and feature sets are continously built by [Travis CI][travis-private-repo-page]. 58 functional, 23 integration and 249 unit tests are executed after each build. Tested with two TLS libraries: [WolfSSL](https://www.wolfssl.com) and [mbedTLS](https://tls.mbed.org). The library is open  for further TLS libraries as well via its TLS BSP.

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
- **examples** - example sources with their own Makefiles. After building, the examples'
             executables will reside here as well.
- **include** - header files of Xively C Client's API. This directory must be added to the
            include path when linked against the Xively C Client library.
- **include/bsp** - header files of the Board Support Package (BSP). Functions here
                shall be implemented during a porting process. Implementation of these
                functions contain all platform specific functionality needed by the
                Xively C Client.
- **include_senml** - header files of the Xively C Client's SenML API. This directory should
                  be added to the include path when using the SenML functionality of
                  Xively C Client library.
- **licenses** - licenses of 3rd party software used by the Xively C Client
- **make** - build system configuration files
- **obj** - object files generated during a build
- **proj** - project files assisting development of the Xively C Client
- **res** - resource files (e.g. trusted root CA certificates)
- **src** - the source files of the Xively C Client

## How to Build?

The Xively C Client's preferred build enviornment is make with gcc on a POSIX platform, though it has been cross ported to other platforms via toolchains specific IDEs.

We recommend building the Xively Client on OSX or Linux before attempting to cross compile the sources to another target. This process could serve as an introduction to the basic architecture, tools and steps which will come handy during the final build onto the target platform.

### Build prerequisites

- autotools: autoconf, automake, libtool is needed by the default TLS library: WolfSSL
- cmake is needed by the integration test framework _cmocka_

On OSX these are available through _brew_:

    - brew install autoconf
    - brew install automake
    - brew install libtool
    - brew install cmake

### Building a TLS static library

By default the Xively C Client library is built with secure connection support and therefore requires a 3rd party TLS implementation to build against. The build enviornment can be configured to use these headers to compile against and libraries to link by calling a script to set the respecitve make variables.

For information on how to run this script please see the [Security](#security) section, then continue with Building the library step below.

### Building the library

    cd xi_client_c
    make libxively

### Building the examples

Examples require both the Xively C Client library and a TLS library to be built beforehand.

    cd xi_client_c/examples
    make

Execution of the examples requires Xively accountId, username, password and usually an mqtt topic to communicate over. These credentails can be obtained via your Device Details page of the [Xively Connected Product Management (CPM)](https://app.xively.com/login).  If you don't yet have a Xively Account, then you may [Register For One Here](https://app.xively.com/register).

For a secure connection a file named xi_RootCA_list.pem is also required to be placed into the current working directory (CWD) of the example, although this is done automatically by the build system.

### Building and executing the tests

Tests require both the Xively C Client and a TLS library to be built beforehand.

    cd xi_client_c
    make

By default the build process includes the test execution as the final step. Although one can execute the tests manually as well by

    cd xi_client_c/bin/osx/tests
    ./xi_utests
    ./xi_itests

### Cross-compilation

For cross-compilation please see the porting guide under *doc* directory. But in short it consists of

- extending the build system with a new cross-compilation setup (examples coming soon for arm-linux, microchip, wmsdk)
- a Board Support Package (BSP) implementation that you have written for your platform
- a TLS library built for the target platform

## Security

The Xively C Client supports secure connection by utilizing a 3rd party TLS library service. The Xively C Client is built and tested against two TLS libraries: [WolfSSL](https://www.wolfssl.com) and [mbedTLS](https://tls.mbed.org). The Xively C Client repository itself does not directly contain TLS libraries in any forms but offers a script to download and build either WolfSSL and mbedTLS library on OSX and Linux.  Please  see the *src/import/tls* directory.

The Xively C Client is open to use any further TLS library solution through its BSP TLS API, please see the porting guide in *doc*. If you choose a library other than mbedTLS or WolfSSL then ensure that it supports async I/O, the loading of custom root CA certificates, and OCSP.

The Xively C Client library can be built without TLS support. The primary purpose for a non-TLS build is to  test against localhosted non-TLS mqtt servers like the [mosquitto](http://mosquitto.org) mqtt server. Turning off TLS is strongly discouraged for release versions. For turning off secure connections please see porting guide's [**CONFIG flags**] section in the *doc*.

A TLS static library is a prerequisite for all Xively C Client examples and test framework executable builds.

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

See more details in directory *doc*

- *doxygen/* - doxygen generated docs
- *porting_guide.md* - a detailed description to aid porting process
- *user_guide.md* - a detailed descript to the Xively C Client
