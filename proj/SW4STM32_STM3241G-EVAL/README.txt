These projects are for building libxively and cyassl, via the 
AC6 System Workbench for STM32 (as provided by OpenSTM32.org) 
toolchain, for use with LwIP and FreeRTOS, as provided by STCube for 
the STM3241G-EVAL Cortex-m4 demo board. 


The projects are configured with relative paths utilizing three variables. These
are: XI_TOPDIR = $PROJ_DIR$\..\..\..\.. which should be the top libxively
directory, CUBE_TOPDIR = $PROJ_DIR$\..\..\..\..\..\xi_target_proj which 
is expected to be the top STCube examples directory (appropriately 
pruned if desired), and APP_PROJ_INC which needs to be set to the configuration-
defining include path for the applicable application project (the directory 
with lwipopts.h and FreeRTOSConfig.h, e.g. $PROJ_DIR$\..\..\..\..\..\
xi_target_proj\Projects\STM324xG_EVAL\Applications\LwIP\
XivelyExampleLwipFreeRtosBringup\Inc.  The CUBE_TOPDIR directory should contain 
the subdirectories: 
Drivers, Middleswares, Projects and Utilities 
(as does STM32Cube_FW_F4_V1.6.0). Note that APP_PROJ_INC must be redefined,
as needed, to point to the correct xively application directory when
these projects are cloned for a new target.

The library builds are dependent upon the include files for LwIP and
FreeRTOS contained within xi_target_proj.  The directory structure 
within xi_target_proj is expected to be as supplied by 
STM32Cube_FW_F4_V1.6.0.

A header file of the build configuration macro definitions that are to be 
shared among the builds of the libs and the xively target projects are 
contained in xi_target_..._config.h.  A specific instances of this file
is to be included in both the lib builds and the target applications build 
via the gcc command line option -include.

These projects are structured utilizing the assumption that that one or more 
projects in the xi_target_proj directory 
in Projects\STM3241xG_EVAL\Applications\LwIP\<proj name>\SW4STM32 
will be the target applications that utilizes the build of libxivly and 
libcyassl created by the projects associated with this README.

It is intended that a retargetting for a different target can be 
efficiently initiated by copying the $XI_TOPDIR$\
proj\SW4STM32_STM3241G-EVAL directory to a new name, creating a new
target directory structure matching STCube's supplied examples and all the 
resulting paths etc. should be correct. If relative positions of these two 
directories are changed or the top level directories are renamed, the 
custom environment variables for each project will have to be
adjusted accordingly.

The build configurations feature is utilized to provide library
build configurations for: dummy_io-dummy_platform and 
lwip_io-newlib_platform.
