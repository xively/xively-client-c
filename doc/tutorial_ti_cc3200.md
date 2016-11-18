# How to connect your CC3200 to Xively

Please scan this whole tutorial to get familiar with it. Then complete each step in a sequential manner; each step builds upon the previous one.

This tutorial supports mainly macOS and Windows, though the Linux flow should be somewhat similar to macOS.


## What you will learn.

This tutorial will teach you how to build, link and deploy a Xively C Client Application onto the CC3200 using Code Composer Studio™. Then you will learn how to connect your device to Xively.


## Hardware you will need.

Texas Instruments [SimpleLink™ Wi-Fi® CC3200 LaunchPad™](http://www.ti.com/tool/cc3200-launchxl) development kit.


## Software you will install during the tutorial.

- Code Composer Studio™
- CC3200 Simplelink™ WiFi SDK
- Xively C Client library
- CC3200 Uniflash _(optional)_

## Step 1 of 7: Install Code Composer Studio™.

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
	1. ```CC32xx Device Support```
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

### Configuration of the Xively C Client build enviornment

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

1. Create a CC3200 device template using the Product Launcher
_This operation will create a device template and a device instance inside the Xively Service to represent your CC3200 board._
 - Click on `Product Launcher` > `Add another device`

 <img src="https://cloud.githubusercontent.com/assets/1899893/20390204/0a1a1068-acce-11e6-90c4-e95aa878913a.png" width="600"/>

 - From the popup window select `Choose from our device library` and click `Next`

 <img src="https://cloud.githubusercontent.com/assets/1899893/20390205/0a1c11ba-acce-11e6-9a80-02802309ac96.png" width="600"/>

 - From the sections tabs at the top of the window go to `Quickstart Kits` and select `TI CC3200` and click `Next`

 <img src="https://cloud.githubusercontent.com/assets/1899893/20390206/0a1f07da-acce-11e6-8fb6-bab8243a314d.png" width="600"/>

2. Get credentials for this device.
 _In order for your device to securely talk to Xively it needs credentials that it will use to authenticate itself as a valid device within your account._
 - Go to `Devices` > `All devices` and look for your sample CC3200 device then click on it's name
 - Click on `Get password`
 - When the modal window pops-up, click the `Download` button.

	A file named `MQTTCredentials.txt` gets downloaded. It contains the device credentials that will be used in the next step. The file contains two data items:
 		- the first line is the _Xively Device Secret_
 		- the second line is the _Xively Device Id_.

 <img src="https://cloud.githubusercontent.com/assets/1899893/20426734/c725974c-ad80-11e6-812f-ef5043b10a2c.png" width="600">

 You now have a provisioned device in Xively that your CC3200 will be able to connect as!

3. Get account id.
_To allow your device to publish and subscribe to MQTT topics you will need your account id_
 - In order to get your account id click on your account name on the very top right side of the page
 - The small window should appear with the name of your organization and with turquoise rectangle underneath
 - Click that rectangle and your account id will be copied to the clipboard
 - Save your account id for later it will be needed for ```xively_demo``` application configuration in the next step

<img src="https://cloud.githubusercontent.com/assets/1899893/20405267/a1b265bc-ad08-11e6-8a9f-08fc64e1ebaf.png" width="600">

 Now you are ready to build and run Xively Demo application!

## Step 6 of 7: Build your client application.

We've prepared a CC3200 demo application that uses the Xively C Library to connect to the Xively Service.  It will allow you to control the on-board LEDs remotely from the Xively Product Launcher, and send temperature and button state data from the device to the Xively Service.

We will first import the example into Code Composer Studio™, and then configure your IoT Client parameters to connect to the Xively service.

### Build the _xively_demo_ example

#### Import _xively_demo_

1. In Code Composer Studio™, select ```File```->```Import```.
2. Select ```Code Composer Studio™```->```CCS Projects``` and click ```Next >```
3. To the right of ```Select search-directory``` click Browse.
4. From this directory, browse to ```PATH_TO_XIVELY_LIBRARY/xively-client-c/examples/cc3200``` and highlight the ```xively_demo``` folder.  Click ```Open```.
5. Click ```Finish```.

#### Code Composer Studio project configuration

In order to adapt the imported project to your environment you have to modify project variables that describes two very important locations on your hard drive.

1. In Code Composer Studio™, make sure the ```xively_demo``` project is highlighted
2. Select ```Project```->```Properties```
3. Highlight```Resource```->```Linked Resources```
4. Double click on ```CC3200_SDK_ROOT``` variable name and using the ```Folder``` button navigate to where your ```CC3200_SDK``` is installed, mark the subfolder ```cc3200-sdk``` in SDK's main directory and hit ```OK``` twice.
5. Double click on ```XIVELY_LIBRARY_C_ROOT``` variable name and again using the ```Folder``` make the variable to point to the ```xively-client-c``` folder.

#### xively_demo configuration

Before you build your application you will have to set your WiFi credentials and Xively device credentials.  

- set your WiFi credentials in main.c

    - update AP name and password defines according to your wifi settings:

            #define ENT_NAME    "AccessPointName"
            #define USER_NAME   "UsernameIfAny"
            #define PASSWORD    "Password"

    - select a security type in the `MainLogic()` function according to your wifi settings.  For example, in the case of WPA2 set

            g_SecParams.Type = SL_SEC_TYPE_WPA_WPA2;

        - for other WLAN security setting flags please refer to `CC3200_SDK_ROOT/simplelink/include/wlan.h` or TI Simplelink documentation

- set your Xively Credentials In main.c

    - locate and update these three values using information you got from Step 5 (_Create your Xively (digital) device_).

            #define XIVELY_DEVICE_ID "PASTE_YOUR_XIVELY_DEVICE_ID"
            #define XIVELY_DEVICE_SECRET "PASTE_YOUR_XIVELY_DEVICE_SECRET"
            #define XIVELY_ACCOUNT_ID "PASTE_YOUR_XIVELY_ACCOUNT_ID"

#### Build and run the example

1. Select ```Project```-> ```Build Project```
	1. When complete, you should see in the ```Console```:

			<Linking>
			Finished building target: xively_demo.out
			...
			**** Build Finished ****

2. 	Execute the example on the CC3200 device
	1. Connect the device to your PC or Mac with USB cable
	2. In Code Composer Studio™, hit the green bug button on the top or select ```Run``` ->```Debug```

Reaching this point means you are able to produce and execute CC3200 compatible binary on the device itself. Congratulations!

**NOTE**: As per Texas Instruments instructions, keep the J15 Jumper set to ON and push Reset button on the board before each debug session. In case of trouble review the [TI's CC3200 help doc](http://www.ti.com/lit/ds/symlink/cc3200.pdf)

### You (hopefully) did it!

If everything worked correctly, within a few seconds you should see a debug log that says

    connected to broker.xively.com:8883

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

When you enter the Product Launcher you will be able to see the graphical representation of data received from your CC3200 device such as temperature and button states. You can also turn on and off CC3200's LEDs' using this web page interface.

<img src="https://cloud.githubusercontent.com/assets/1899893/20430715/39b6de32-ad96-11e6-8945-79442b3f77a9.png" width="600">

## What to do next?

### How to use a more secure configuration of wolfSSL with OCSP Stapling?

For advanced security with OCSP support please take a look at the next tutorial: [CC3200 Xively tutorial Stage 2](./tutorial_ti_cc3200_advanced.md)

### More coming soon

_For now [please visit our docs](http://developer.xively.com/docs) and view some other guides and please let us know any feedback or questions that you may have regarding this guide or Xively in general!_

## Common pitfalls or errors

_More coming soon_

##### Q. When I build the example application I get the "Xively Hello World" debug message, but with a state of 34.

    connection to broker.xively.com:8883 has failed reason 34

**A.** A state of `34` means that the device connected to the Xively system, but its credentials are invalid. This could occur if you copied the credentials incorrectly or if you have regenerated the device credentials and are using older ones. The easiest way to fix this issue is to regenerate the device credentials (see Step 5.3) and re-copy the new credentials within `main.c`. Once you've done this rebuild the image flash the hardware again.
