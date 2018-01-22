# Xively Client version 1.3.3
#### Jan 22 2018

## Features
- [CC3220SF] Updated example application support to TI SimpleLink SDK 1.6, XDC Tools version 3.50.04.43.
- [CC3220SF] Example application now detects WiFi disconnections and cleanly reconnects when WiFi AP returns.
- [ESP32] Updated the Xively C Client build to work with IDF SDK: Master commit af63ca1
- [ESP32] Improved LED signaling in demo application.
- [Secure File Transfer] CBOR library assertions are now suppressed.  They can be enabled by defining `XI_DEBUG_ASSERT=1` on the make command line when building the Xively C Client library. 

## Bugfix
- [ESP32] Fix integer overflow in time BSP.
- [ESP32/CC3200] Fix FWU Board Support Packages for SFT when running an SFT migration that does not include new firmware binaries. Previously the device rebooted at switched boot partitions upon all SFT deliveries, now this only occurs if a new firmware image is downloaded.
- [WolfSSL] `src\import\tls\wolfssl.conf` configuration script errantly defined --enable-debug twice.

### Misc
- Removed legacy `Licenses` directory.  All licenses are now in `LICENSE.md` of the base directory of the repository.
- Updated Copyright Header to 2018.
- Travis CI configuration changes to FuzzTest Environment.
- Moved the `cc3200\xively_firmware_updates` to `src\experimental\cc3200\xively_sft_external_client`. This external client application is not our standard Secure File Transfer client, but instead serves as a reference for those who would like to implement SFT on another MQTT client or in another language.  Specifically it demonstrates how to use the CBOR encoder to handle SFT Service Requests and Responses over MQTT. This implementation was moved from our examples directory as it was being confused with our CC3200 SFT implementation via our Board Support Packages (FWU and FS BSPs).  This experimental application will not be kept up to date.  Further development using the Xively C Client SFT feature should be done inside the BSPs of the respective platforms, such as in `src\bsp\platform\cc3200\xi_bsp_fw_cc3200.c` and `src\bsp\platform\cc3200\xi_bsp_io_fs_cc3200.c`.

# Xively Client version 1.3.2
#### Nov 24 2017

## Features

### Extensions to Secure File Transfer (SFT) and Firmware Updates (FWU) feature

- SFT now supports Large File Downloads over HTTP. The SFT Service cannot deliver files
  larger than 8MB through MQTT. However, it now transmits URLs for these large files so
  that larger devices may download them over the HTTP protocol. The Xively C Client now has an
  updated Board Support Package (BSP) which reports these URLs to the Application, which
  must then download the file separately and report the download result to the Xively C Client.
  The file download status report is propagated back to the Xively SFT service by the
  Xively C Client for fleet-wide deployment tracking, and then the Xivley C Client continues
  the update process by working through the next file on the download list. From an update flow
  perspective, nothing has changed, only the protocol that was used to download the file.
- Added the ability for the Applications to order the files downloaded over the Xively Secure
  File Transfer service. Please see the documentation for
  `xi_bsp_io_fwu_order_resource_downloads()` in the header `include/bsp/xi_bsp_fwu.h` for more
  information.
- Auto-test coverage was increased on the new SFT feature.

### ESP32 Support Extended

- ESP32 now supports firmware updates over MQTT using Xively's Secure File Transfer
  protocol.
- Travis CI now builds ESP32 target.

### Misc

- API extension: added `xi_is_context_connected( xi_context_handle_t )` to the standard
  Xively API.


# Xively Client version 1.3.1
#### Oct 10 2017

## Bugfix

- Fix WolfSSL build for the ESP32 platform.


# Xively Client version 1.3.0
#### Oct 06 2017

## Features

### Secure File Transfer (SFT) and Firmware Updates (FWU)

- The Xively C Client now provides Secure File Transfer (SFT) and Firmware
  Update (FWU) functionality over MQTT out of the box. These features are coupled to a
  device's Firmware and File Storage SDKs through two highly-portable Board Support Packages
  (BSPs). The Xivley C Client controls the main update flow, protocol, and chunking of files,
  with only the satellite functionalities of non-volatile storage, bootloader management and
  restart functionalities required for porting.Example implementations for CC3200 and POSIX.

- Secure File Transfer (SFT) and Firmware Updates (FWU) features have been added to the
  CC3200 xively_demo example.

- Changes to our External SFT Implementation Demo: Previously the Xively C Client
  added TI CC3200 SFT and FWU functionalities at the application level (not in our Xively C
  Client Library BSPs).  We will keep the sources for this client-external SFT implementation
  to serve as a quick reference of how a Xively SFT client could be implemented in other
  languages. Please see our CC3200 Secure File Transfer (SFT) Tutorial in
  the Xively Development Center to remote update your CC3200's firmware over MQTT today!

### Ports

- ESP32 Port (New!): A BSP, makefile system, and an example application have been written for
  the ESP32. Please see the Xively Developer Center for a tutorial.

- STM32F4 Tier 1 tutorial support: now the repository contains precompiled STM32F4
  wifi + ethernet application binaries to support the simpliest tutorial of connecting
  these devices to Xively.

### Misc

- CC3220SF Library and Build environment update: tested use of CC3220 SDK v1.4 and
  XDCTOOLS v3.50.02.20, which will be part of our CI builds.

- wolfSSL version update: default builds now work against version 3.10.2-stable

## Documentation

- Porting Guide and User Guide are updated with Secure File Transfer (SFT) and
  Firmware Updates (FWU) feature description.



# Xively Client version 1.2.6
#### Jun 30 2017

## Documentation

- Fixes to `doc/porting_guide.md` referring to improper make enviornment CONFIG flag `tls`. It should be `tls_bsp`.

## Features

- Secure File Transfer: A new TI CC3200 client that allows firmware updates over MQTT via the Xively SFT service.
- Updates to the User Guide to reflect the new Open Source porting process. Also added links to our CC3200 and STM32 Nucleo examples, and to our Further Readings.

### TI CC3200
- Contains a new example `xively_firmware_updates` as our first reference implemention of using Xively Secure File Transfer to update the firwmare of a CC3200.

### TI CC3220SF
- Added TI CC3220SF demo application sources and build process for tutorials on Xively Developer Center.

### STM32F4 Nucleo
- Updated STM32F4 examples to build against latest ST SDKs.
- New build configurations to support both sensor boards (IKS01A1 and IKS01A1) out of the box.
- The github repository is extended with a prebuilt binary for STM32F401RE + WiFi IDW01M1 + MEMS IKS01A1. Tier 1 tutorial (how to use the precompiled binary) is also provided.
- STM32F4 ethernet example now supports runtime provisioning over serial and flash storage of Xively credentials.
- Convenience fixes for Ethernet and WiFi project examples to increase ease use of the tutorial.

## Internal Development
- CI system now builds against ST SDKs v1.16.0 for Ethernet and v3.0.2 for WiFi.


# Xively Client version 1.2.5
#### Mar 14 2017

## Features

- STM32F401RE Nucleo board + wifi X-NUCLEO-IDW01M1 expansion board support. Example Eclipse project and Tutorial (https://developer.xively.com/docs/stm32f4xx-nucleo) included. TLS connection to Xively Servers uses the wifi expansion on-board TLS solution. Accurate time initialised from SNTP servers.


# Xively Client version 1.2.4
#### Jan 17 2017

## Bugfixes

- Fixing two build errors for CC3200's Tutorial project: xively_demo (linker optimization problems with two functions: xi_event_loop_with_evtds and xi_bsp_io_net_select)



# Xively Client version 1.2.3
#### Jan 10 2017

## Features

- TI CC3200: fixed all compiler warnings so that the client compiles cleanly for that platform.
- The library now supports fuzz tests! For more details refer to ```doc/dev/FuzzTestsProgrammerManual.md```.
- Makefile support files in the ```make/``` directory now have the .mk extension.
- Makefile updates for better Linux compatibility.
- Travis CI maintenance, Internal Storage Infrastructure and Build changes for Continuous Integration tools.
  - End result: We'll be to support more platforms.
- Removed legacy TLS BSP files for WolfSSL. These were from a previous Xively C Client BSP architecture.
  - The new BSP files remain in ```src/bsp/tls/wolfssl```.



# Xively Client version 1.2.2
#### Nov 24 2016

## Documentation and Ease of Use

- CC3200 tutorials are now accessible on https://developer.xively.com.
- Added xively_demo application example Code Composer Studio project for CC3200 devices.
- Removed uses of \r from some debug output statements to be more consistent, and to remove some formatting problems on some platforms. We now always simply use \n.
- Added error code numbers to comments in xi_error.h to easily map error codes integer values to their corresponding enumerated type.

## Bugfixes

- Fixed a socket descriptor leak on the CC3200.

## Known issues

- When the io input buffer size increased over 64 bytes incoming messages could be stacked in the tls receive buffer and not parsed

# Xively Client version 1.2.1
#### Nov 04 2016

## Documentation and Ease of Use

- Tutorial for cross-compiling a Xively Client application onto Texas Instruments CC3200 on Windows and macOS has been added to doc/ directory.
- Added commandline build capability on Windows using TI ARM toolchain.
- Ubuntu Build dependency steps are added to README.md.
- Removed out of date project files for IAR IDE and for STM32 evaluation platforms.

# Xively Client version 1.2.0
#### Oct 24 2016

## Documentation and Ease of Use

- The Xively C Client is now Open Source, distributed under the BSD 3-Clause license.
- Finalized our Board Support Package (BSP) implementation. This new system should massively reduce any porting effort of the Xively C Client by focusing platform-specific customizations to a select few files.
- README.md file has been updated serving as a first-step introduction.
- User Guide added to ```doc/``` directory as a markdown file.
- Porting Guide added to ```doc/``` directory as a markdown file.
- Doxygen documentation is now pre-generated in the ```doc/doxygen``` folder. It is provided in two main sections:
	1. API documentation for building client applications that use the Xively Service
	2. the Board Support Package (BSP) documentation for porting the Xively C Client to new platforms.
- Default make target now builds the Xively C Client for POSIX and fetches, downloads and compiles WolfSSL automatically.  Note: Autotools are required to build WolfSSL.
- Default make target's only goal is to build the Xively C Client static library itself without examples and tests.
	- To build tests, execute ```make tests```
	- To build examples, cd to ````examples/``` and run ```make```.
- Created help make targets as a resource for people attempting to navigate our make system. Type ```make help``` for more information.

## Development Enhancements

- Build presets were introduced to ease handling of the multi target and configuration make system. run ```make help_build_presets``` for more information.
- More developer friendly ARM cross compilation on MacOS and Linux.
- Output libxively.a new location is the bin subdirectory.
- Clearer BSP directory structure: New layout of ```src/bsp directory``` has been developed. It contains two subfolders:
	1. ```src/bsp/platform``` for all platform specific implementations (such as network, memory, time, RNG.).  New reference implementations are provided for posix, wmsdk, and microchip TCP.
	2. ```src/bsp/tls``` for support of third party TLS implementations.  Reference implementations are provided for WolfSSL and mbedTLS.
- Library size reductions:
  - removed redundant implementation of the vector data structure
  - new BSP architecture removed unneeded code infrastructure.
- Simplified make build system
- Code reformatted based on ```.clang-format``` file in base directory of repository.
- Removed a ranlib build warning on OSX.
- migrated sources to a new home for open source: ```https://github.com/xively/xively-client-c```

# Xively Client version 1.1.6
#### Jul 08 2016

## Features

- The sources for WolfSSL and CyaSSL are no longer part of the Xively Client repository.
  WolfSSL has to be downloaded and compiled manually. Please follow the building
  Xively Client Library chapter in Porting Guide for more details.
- Custom server address support with two new API functions: xi_connect_to and
  xi_connect_with_lastwill_to. These functions accept two additional parameters
  (host and port) which determine the target address the Xively C Client connects to.


# Xivelt Client version 1.1.5
#### Jun 09 2016

## Features

- [Porting Guide](include/bsp/PORTING-GUIDE.md) is added. Describes the porting process in detail.
- Separated Example Applications. Examples are separated from Xively C Client internal sources and make system. This eases new example creation process for a new customer.
- Internal make-system rationalization. CONFIG flags now share unified style. Config presets are introduced: POSIX\_DEV, POSIX\_REL
- Added new Makefile option to define BSP root folder outside the Xively C Client's directory structure for custom BSP implementations. Just call make BSP_DIR=/your\_path/bsp\_root\_directory

## Notes

- Removed functionality related to reading the username and password from the file.


# Xivelt Client version 1.1.4
#### May 12 2016

## Features

- BSP TIME API added. Reference implementations for POSIX, Microchip and WMSDK. New function ```xi_bsp_time_getcurrenttime_milliseconds``` has been added to new BSP file ```xi_bsp_time.h```.
- BSP MEMORY API added. Now applications can customise memory management of the Xively C Client. For instance a custom implementation can use static memory on devices where heap isn't desired. New functions ```xi_bsp_mem_alloc```, ```xi_bsp_mem_realloc``` and ```xi_bsp_mem_free``` has been introduced within ```xi_bsp_mem.h``` file.

## Bugfixes

- Fixed duplication of msg IDs when sending new messages during continuous session while unacked messages present
- Zero Length Client IDs are now supported and will not cause errors in the client
- Resource management system will now correctly pass-through errors via callbacks
- Added password 'should be unset if username is not' set rule to meet the MQTT 3.1.1 specification

# Xively Client version 1.1.3
#### May 6 2016

## Bugfixes

- Fixed timeout calculation for xi_bsp_io_net_select select function.

# Xivelt Client version 1.1.2
#### April 28th 2016

## Features

- Board Support Package (BSP)
    - New function xi_bsp_io_net_select is now part of the BSP networking package. Two reference implementations of this function for posix and microchip were added. Declaration can be found in xi_bsp_io_net.h and definitions resides in xi_bsp_io_net_posix.c and xi_bsp_io_net_microchip.c.
    - New BSP Random Number Generator API added. API is declared in new xi_bsp_rng.h file. Reference implementation for posix and wolfssl are located in xi_bsp_rng_posix.c and xi_bsp_rng_wolfssl.c.

## Known Bugs

- Msg id duplication when new messages sent in continious session while unacked messages are present.

# Xivelt Client version 1.1.1
#### April 14th 2016

## Features

- Mqtt Clean Session - client side support
    - Xively C Client re-delivers unacknowledged SUBSCRIBEs, QoS 1 and 2 PUBLISHes across
      separate connections if CLEAN SESSION flag is not set. See details on page
      <http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718030>
    - Xively Client keeps messages in memory. This means restart of Xively Client library
      or the device itself will still result in loss of messages waiting for re-delivery.
- BSP NET API documentation.
- Local developer builds are extended with cross compilation ability which accelerates
  and validates portable code development.

## Bugfixes

- XCL-2028: memory leak fix in case of unsuccessful control topic subscription

# Xively Client version 1.1.0
#### March 22nd 2016

## Features

- __v1.1.0 API Update__

  The Xively Client API has received an update to clean up header file dependencies and to increase ease of use.  The following significant changes have been made to the API:

    - Client Application Include Path has been simplified.  We've moved all Client Application header file dependencies to "include" and "include\_senml". These directories contain all dependencies for examples and for any application using the Xively C Client.
        - "include" contains dependencies for a client application to build a Xively connection, manage subscriptions and to publish messages.
        - "include\_senml" is the API of the SenML module.  This functionality is independent of the main Xively API in the "include" directory (only error codes in xively_error.h are shared).
        - Users of previous libxively versions may adapt to these changes by including only "include/xively.h"
    - The structures xi\_data\_desc\_t and xi\_mqtt\_message_t have been removed from the public API
    - The subscription callback function signature has been reworked.  MQTT message data, such as topic, payload, etc, has been broken out into discrete parameters instead of passed in as an abstract structure.  Please see the xively.h documentation and our examples for more information.
    - SenML API reworked in order to clear up header file dependencies.
        - Most of the usage is the same as before, though XI\_CREATE\_SENML\_STRUCT now requires at least one base value initializer (name, unit or time). If no base value is required then the usage of XI\_CREATE\_SENML\_EMPTY\_STRUCT is required.

- __Further Feature Changes__

    - Added xi\_client\_version\_str declared in xively.h. This can be used as a string representation of the Xively Client Library version numbers (xi\_major, xi\_minor, xi\_revision) for debug output purposes and for examining library binary files to determine the version.  The string format is:  "Xively C Client Version: <major>.<minor>.<revision>"
    - Header Files now have a standard format via the use of Clang format.
    - Client examples are formatted with Clang format.
    - Client examples have a standard command line argument parser using mosquitto nomenclature whenever possible.
    - Client examples all have a --help option to print usage.
    - Microchip Platforms: Added Dynamic File System Support to the Microchip Builds.
    - Microchip Platforms: Enforce KeepAlive MQTT packet regardless of publish traffic in order to better detect physical LAN disconnects.
    - TLS: Added support for OCSP Stapling support with WolfSSL
    - TLS: Upgraded supported WolfSSL implementation to 3.7.0
    - Cleaned up header file dependencies of our examples.
    - Replaced xi\_evtd\_stop( event\_dispatcher ) with xi\_events\_stop()
    - Replaced xi\_platform\_loop() with xi\_events\_process\_blocking()
    - Added xi\_events\_process\_tick() for No OS or RTOS devices who can't use xi\_events\_process\_blocking()
    - BSP NET API and its posix implementation released as a first step of the full BSP portfolio. Now porting the network stack should be much easier than before.

## Bugfixes
- XCL-1546: Fixed a Bug in  Microchip IO platforms where there could potentially be a crash in Debug Logging Mode when a critical and unexpected error state occurred. This problem did not occur in Release Builds of the Xively Client Library.

## Known Bugs
- There is a memory leak for each time the control topic subscription is unsuccessful. This bug is fixed in *v1.1.1*.

# Xively Client version 1.0.0
#### August 13th 2015

## Features
- Simplified the API
    - Removed Default Contexts
    - Collapsed multiple publish, subscribe, and connection functions
    - All Xively API functions now take callbacks
- Added Publish Formatted Timeseries Function for publishing float and string timeseries values with an optional category tag.
    - Formatted timeseries can be published with a null time value.  In this configuration the Xively Service's server time will be used to timestamp the message.
- Added Version Numbering
    - Version Numbers are extern integers xi\_major, xi\_minor, xi\_revision and should be available in your source when you include xively.h
- Reduced Memory Usage during the TLS encryption process when transmitting messages. Previously message payloads were being duplicated in memory.
- Reduced Memory Usage of PUBLISH MQTT message during message serialization. Previously PUBLISH message payloads were duplicated before they were sent.
- New API functions to register user event callbacks with a delay and to cancel previously registered functions.

## Bugfixes
- XCL-1229: Fixed a bug where the fifth or higher mqtt topic subscription would not properly register the callback function for the topic.
- XCL-1325: Attempting to invoke shutdown\_connection() on a disconnected context will now result in the XI\_SOCKET\_NO\_ACTIVE\_CONNECTION\_ERROR return code instead of XI\_STATE_OK.
- XCL-1412: Fixed a memory leak on Microchip TCP platforms when a disconnection occurred during a read or a write from a socket.
