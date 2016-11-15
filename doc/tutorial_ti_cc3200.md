# How to connect your CC3200 to Xively

Please scan the whole tutorial to get familiar with it. Then complete each step in a sequential manner; each step builds upon the previous one.

This tutorial supports mainly macOS and Windows, though the Linux flow should be somewhat similar to macOS.


## What you will learn.

This tutorial will teach you how to build, link and deploy a Xively C Client onto the CC3200 using the Code Composer Studio™. Then you will learn how to connect your device to Xively.


## Hardware you will need.

Texas Instruments [SimpleLink™ Wi-Fi® CC3200 LaunchPad™](http://www.ti.com/tool/cc3200-launchxl) development kit.


## Software you will install during the tutorial.

- Code Composer Studio™
- CC3200 Simplelink™ WiFi SDK
- Xively C Client library
- CC3200 Uniflash _(optional)_

## Step 1 of 7: Install the Code Composer Studio™.

Code Composer Studio™ includes the toolchain (compiler) you'll need to build for the CC3200 and a java-based IDE.

[Download](http://www.ti.com/tool/ccstudio) the Code Composer Studio™ appropriate for your operating system (Windows, Linux or macOS).

1. Complete the free registration.
2. Validate your email address.
3. Complete the brief export approval form and click ```Submit```.
4. Upon approval, click ```Download``` to proceed. Monitor the download process to completion.
5. Once download is complete, start the installation.
6. Accept the license agreement and click ```Next >```.
7. Choose the default install folder and click ```Next >```. Or, if you install into a custom directory, then please note its path as you will need to refer to it later.

	By default the path should be ```c:\ti``` on Windows and ```/Applications/ti``` on macOS.

8. Enable the following two options under ```SimpleLink Wireless MCUs```:
	1. ```CC3200xx Device Support```
	2. ```TI ARM Compiler```

9. Click ```Next >``` twice more, and click ```Finish``` when the button becomes enabled.
10. Once installation completes, click ```Finish``` to leave the installer.


## Step 2 of 7: Install the CC3200 Simplelink™ WiFi SDK.

These are the platform libraries that you'll need to compile and link against when writing software for the CC3200.

1. Launch Code Composer Studio™.
2. If prompted to ```Select a Workspace```, click ```OK``` to select the default path.
3. Select  ```View```->```Resource Explorer``` from the top bar menu.
4. Select ```CC3200 Simplelink WiFi``` from the list of available development tools.
5. On the right side of the screen, click the ```Install on Desktop``` down-arrow icon and select ```Make Available Offline```. Confirm ```Yes``` on the popup window.
6. A ```Dependencies``` popup may appear.  Click ```OK``` to download any software dependencies.

*NOTE*: Windows users may download the [CC3200 Simplelink™ WiFi SDK](http://www.ti.com/tool/cc3200sdk) directly outside of the Code Composer Studio™ if you wish. Once downloaded, please install using the default settings.

## Step 3 of 7: Download the Xively C Client library.

Download the library source code from [xively-client-c](https://github.com/xively/xively-client-c).  Git [clone](https://help.github.com/articles/set-up-git/) the repository or download the source archive from the right side of the github page.

## Step 4 of 7: Build the Xively C Client library.

### Prebuild configuration of the Xively C Client

#### Configure make target file mt-cc3200
1. Open the file ```make/mt-os/mt-cc3200``` in your favorite friendly text editor.
2. Scroll the HOSTS section devoted to your host platform: ```MAC HOST OS```, ```WINDOWS HOST OS```, or ```LINUX HOST OS```.
2. In your newly identified host's section, set ```XI_CC3200_PATH_CCS_TOOLS``` and ```XI_CC3200_PATH_SDK``` to your Code Composer Studio™ and SDK install paths, respectively.  **If you chose the default installation paths for these installations then these values should already be valid and you shouldn't need to change anything.**
3. The toolchain that Code Composer Studio™ downloaded might differ from the default that's configured in this ```mt-cc3200``` file.
	1. Please browse to the path which you set ```XI_CC3200_PATH_CCS_TOOLS```.
	2. Open up the ```compiler/``` directory and note the name of the toolchain.
	3. Compare this to the toolchain name stored in the ```COMPILER``` variable near the top of the file in ```mt-cc3200```.  Update the ```COMPILER``` variable as necessary.

### The process for building slightly depends on your host OS:

_Execute the commands below from the `xively-client-c` root folder:_

#### Windows:

Set paths for ```gmake``` and ```mkdir```

    PATH=%PATH%;c:\ti\ccsv6\utils\bin
    PATH=%PATH%;c:\ti\ccsv6\utils\cygwin

Clean and build the library:

    gmake PRESET=CC3200_TLS_SOCKET clean
    gmake PRESET=CC3200_TLS_SOCKET

#### macOS and Linux:

Clean and build the library:

    make PRESET=CC3200_TLS_SOCKET clean
    make PRESET=CC3200_TLS_SOCKET

## Step 5 of 7: Create your Xively (digital) device.

_You should have a Xively account already created, but if you do not, register one for free at [Xively.com](https://app.xively.com/register)._

To have a device communicate through Xively we will first need to tell the Xively system that a device exists. [Log into the Xively CPM app](https://app.xively.com/) to complete the following steps.

1. Create a device template.
 _This device template will represent the CC3200 type board that we are using for this example._
 - Click on `Devices` > `Device templates`
 - Click on `Add  new device template`
 - Enter any name you want (ex: "CC3200 Launchpad") and click `Ok`

 <img src="https://cloud.githubusercontent.com/assets/1428256/19813190/82157058-9d06-11e6-9b47-99c99e235850.png" width="600">

2. Create an individual device.
 _This individual device will represent the specific CC3200 board that you have physicially connected for this example._
 - Click on `Add new device`
 - The device template we just created should already be selected for the template.
 - Choose any Org from the list
 - Enter any serial number you want (ex: "My Xively CC3200") and click `Ok`

 <img src="https://cloud.githubusercontent.com/assets/1428256/19813191/821704b8-9d06-11e6-89aa-78b52c251d20.png" width="600">

3. Get credentials for this device.
 _In order for your device to securely talk to Xively it needs credentials that it will use to authenticate itself as a valid device within your account._
 - Click on `Get password`
 - When the modal window pops-up, click the `Download` button.

	A file named `MQTTCredentials.txt` gets downloaded. It contains the device credentials that will be used in the next step. The file contains two data items:
 		- the first line is the _Xively Device Secret_
 		- the second line is the _Xively Device Id_.

 <img src="https://cloud.githubusercontent.com/assets/1428256/19813189/8214fda8-9d06-11e6-859f-f3805e34ec04.png" width="600">

 You now have a provisioned device in Xively that your CC3200 will be able to connect as!

## Step 6 of 7: Build your client application.

We suggest the _ent_wlan_ networking example from the CC3200 SDK as the basis for connecting to Xively. We will first import the example into Code Composer Studio™, and then add some code to build your IoT Client connection to the Xively service.

### Build the _ent_wlan_ example

#### Import _ent_wlan_
1. In Code Composer Studio™, select ```File```->```Import```.
2. Select ```Code Composer Studio™```->```CCS Projects``` and click ```Next >```
3. To the right of ```Select search-directory``` click Browse.
4. From this directory, browse to ```ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/ent_wlan``` and highlight the ```ccs``` folder.  Click ```Open```.
5. Click ```Finish```.

#### Build and run the example
1. Select ```Project```-> ```Build Project```
	1. When complete, you should see in the ```Console```:

			<Linking>
			Finished building target: ent_wlan.out
			...
			**** Build Finished ****

2. Before the first execution, you will need to create a Configuration so that Code Composer Studio™ knows which platform you're loading the source onto.
	1. Select ```View``` -> ```Target Configurations```.  The ```Target Configurations``` panel opens to the right side of the IDE.
	2. Right click on ```User Defined``` and select ```New Target Configuration```.
	3. Choose a filename or keep the default.  Click ```Finish```.
	4. In the ```Connection``` pulldown, select ```Stellaris In-Circuit Debug Interface```.
	5. Select the box next to ```CC3200``` to add a check mark.
	6. Click the ```Save``` button to the right. _You may have to scroll the middle window to the right to see the button._
	7. Back in the ```Target Configurations``` panel to the right, expand ```User Defined```.
	8. Right click on your new target configuration and select ```Set as Default```.

3. 	Execute the example on the CC3200 device
	1. Connect the device to your PC or Mac with USB cable
	2. Hit the green bug button on the top in the CCS, or select ```Run``` ->```Debug```

This should upload your program to RAM and end up with a debugger standing at the first line of main function in main.c.

Reaching this point means you are able to produce and execute CC3200 compatible binary on the device itself.  Congratulations!

**NOTE**: As per Texas Instruments instructions, keep the J15 Jumper set to ON and push Reset button on the board before each debug session. In case of trouble review the [TI's CC3200 help doc](http://www.ti.com/lit/ds/symlink/cc3200.pdf)

### Add the Xively Client to ent_wlan

Next we're going to add a function to connect to the Xively Broker. Its implementation is based on the examples in the Client repo, e.g. `xively-client-c/examples/mqtt_logic_producer/src/mqtt_logic_producer.c`.

- Paste the following code within `main.c` of the _ent_wlan_ anywhere in the main portion of the file _before_ the location where we will call the function, which will be around line 647 (see the following steps).

        #include <xively.h>
        #include <stdio.h>

        void on_connection_state_changed( xi_context_handle_t in_context_handle,
                                          void* data,
                                          xi_state_t state )
        {
            printf( "Hello Xively World!, state: %d\n", state );
        }

        void ConnectToXively()
        {
            xi_initialize( "xi_account_id", "xi_device_id", 0 );

            xi_context_handle_t xi_context = xi_create_context();

            if ( XI_INVALID_CONTEXT_HANDLE >= xi_context )
            {
                printf( " xi failed to create context, error: %d\n", xi_context );
            }

            xi_state_t connect_result = xi_connect(
                    xi_context,
                    "11111111-aaaa-bbbb-cccc-222222222222",         // Paste Your Xively Device Id Here
                    "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678=", // Paste Your Xively Device Secret Here
                    10, 20,
                    XI_SESSION_CLEAN, &on_connection_state_changed );

            xi_events_process_blocking();

            xi_delete_context( xi_context );

            xi_shutdown();
        }

- For _Xively Device Id_ and _Xively Device Secret_ use the information you got from Step 5 (_Create your Xively (digital) device_).

- Locate the successful wifi connection point in the `main.c` of the ent_wlan example (around line 647, comment: "//wait for few moments"). Here put a call on the ConnectToXively(); function we just added.

- To make the above buildable you'll need to:
    - add two include paths to your project to help the compiler find `xively.h` and friends: ```Project```->```Properties```->```Build```->```ARM Compiler```->```Include Options```:
        - `xively-client-c/include`
        - `xively-client-c/include/bsp`
    - add the Xively C Client library: ```Project```->```Properties```->```Build```->```ARM Linker```->```File Search Path```:
        - `xively-client-c/bin/cc3200/libxively.a`
    - add two files to the project: ```Project```->```Add Files```: `ti/tirex-content/CC3200SDK_1.1.0/cc3200-sdk/example/common`
        - `timer_if.h`
        - `timer_if.c`

    - update wifi settings in main.c

        - update AP name and password defines according to your wifi settings:

                #define ENT_NAME    "AccessPointName"
                #define USER_NAME   "UsernameIfAny"
                #define PASSWORD    "Password"

        - select a security type in the `EntWlan()` function according to your wifi settings.  For example, in the case of WPA2 set

                g_SecParams.Type = SL_SEC_TYPE_WPA_WPA2;

            and delete the variable `eapParams`. Then pass NULL as the last attribute to the connect function:

                lRetVal = sl_WlanConnect(ENT_NAME,strlen(ENT_NAME),NULL,&g_SecParams,NULL);

    - All set. Now do this: `Project`->`Build` and `Run`->`Debug`

        This should result in a CC3200 connected to Xively Services!

### You (hopefully) did it!

If everything worked correctly, within a few seconds you should see a debug log that says

    Hello Xively World!, state: 0

If you do not see that, double check that you followed all the previous complicated steps accurately. If you see a `state` value other than `0` check within `xively_error.h` to see which error could be occuring (ex: `34` means bad credentials).

If you are just testing (or on a Mac) go ahead and skip the next step and go straight to [Congratulations!](#congratulations!)


## Step 7 of 7: Flash your client application onto the device. _(Optional, Windows Only)_

By default Code Composer uploads your application into RAM for execution. This is great for quick iterations, but it also means that your device will lose your changes when you uplug it.

To permanently make changes to the device you must flash the device using a Windows binary executable called Uniflash. This tool is external to Code Composer Studio™.

### Download and install the Code Composer Studio™ Uniflash software

* From [Code Composer Studio™ Uniflash download page](http://processors.wiki.ti.com/index.php/CCS_Uniflash_v3.4.1_Release_Notes) choose Windows Offline Version
* Begin the installation process
* On the "Select Components" window
    * Please leave only ```Simplelink WiFi CC31xx/CC32xx``` the selected and continue installation process

### Run the Code Composer Studio™ Uniflash Software

* Plug in your CC3200 device and make sure that the J15 Jumper is set to ON
* From ```File``` select ```New Configuration``` and select
    * Connection: ```CC3x Serial(UART) Interface```
    * Board or Device: ```SimpleLink WiFi CC3100/CC3200```
* On the left panel under the ```System Files``` please highlight the ```/sys/mcuimg.bin```file
* From the right panel press the ```Browse``` button right next to the ```Url``` field
* Pick the ```name_of_your_project.bin``` from your ```workspace_name/project_name/RELEASE/```
* From the left panel highlight ```CC31xx/CC32xx Flash Setup and Control```
* Press ```Program``` button
* Set the J15 jumper to OFF and restart your device it should now run the test program


## Congratulations!

You did it! You now have a CC3200 board connected and communicating with Xively.

You should be able to go back to your device page on Xively CPM and see that its status is now `Connected` and within the logs see its `Device connected` lifecycle log.

<img src="https://cloud.githubusercontent.com/assets/1428256/19814034/91b3b296-9d0a-11e6-813b-9eb7ca499350.png" width="600">

## What to do next?

### Stage 2: How to use custom wolfSSL during connecting to Xively with CC3200

For advanced security please take a look at the tutorial extension in the same directory as this document: [CC3200 Xively tutorial Stage 2](./tutorial_ti_cc3200_advanced.md)

### More coming soon

_For now [please visit our docs](http://developer.xively.com/docs) and view some other guides and please let us know any feedback or questions that you may have regarding this guide or Xively in general!_

## Common pitfalls or errors

_More coming soon_

##### Q. When I build the example application I get the "Xively Hello World" debug message, but with a state of 34.

    Hello Xively World!, state: 34

**A.** A state of `34` means that the device connected to the Xively system, but its credentials are invalid. This could occur if you copied the credentials incorrectly or if you have regenerated the device credentials and are using older ones. The easiest way to fix this issue is to regenerate the device credentials (see Step 5.3) and re-copy the new credentials within `main.c`. Once you've done this rebuild the image flash the hardware again.
