# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk
include make/mt-utils/mt-get-gnu-arm-toolchain.mk

XI_STM32_PATH_SDK ?= $(HOME)/Downloads/xively-client-artifactory/st/STM32Cube_FW_F4_V1.16.0

$(info .    CC:                  [$(CC)] )
$(info .    AR:                  [$(AR)] )
$(info .    XI_STM32_PATH_SDK:   [$(XI_STM32_PATH_SDK)] )

#####################
# LWIP configurations
#####################
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/LwIP/src/include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/LwIP/system

XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/FreeRTOS/Source/include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F

XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Include
####################
# STM324xG_EVAL
####################
# XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Projects/STM324xG_EVAL/Templates/Inc

####################
# STM32F429ZI-Nucleo
####################
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Projects/STM32F429ZI-Nucleo/Templates/Inc
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/STM32F4xx_HAL_Driver/Inc
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Device/ST/STM32F4xx/Include

####################
# STM32F207ZG-Nucleo
####################
# XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Projects/STM32F207ZG-Nucleo/Templates/Inc
# XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/STM32F2xx_HAL_Driver/Inc
# XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Device/ST/STM32F2xx/Include

XI_COMPILER_FLAGS += -I$(LIBXIVELY)/src/bsp/platform/stm32fx/include

####################
# arm-eabi-specifics
####################
# create dummy fsctl and unistd methods for gnu newlib
#XI_COMPILER_FLAGS += -ffreestanding
# stm32f4 uses cortex m4
# enable Thumb instruction set ( reduces memory usage )
XI_COMPILER_FLAGS += -mthumb
# diagnostic message length
XI_COMPILER_FLAGS += -fmessage-length=0
# set char type
XI_COMPILER_FLAGS += -fsigned-char
# linker can optimizie better if we use function
XI_COMPILER_FLAGS += -ffunction-sections
# loop optimization
XI_COMPILER_FLAGS += -fno-move-loop-invariants

# define the board type
# XI_COMPILER_FLAGS += -DSTM32F207xx
# XI_COMPILER_FLAGS += -DUSE_STM32F2XX_NUCLEO_144
# XI_COMPILER_FLAGS += -mcpu=cortex-m3
# XI_COMPILER_FLAGS += -mfloat-abi=soft

# XI_COMPILER_FLAGS += -DSTM32F407xx
# XI_COMPILER_FLAGS += -mcpu=cortex-m4
# XI_COMPILER_FLAGS += -mfloat-abi=hard

XI_COMPILER_FLAGS += -DSTM32F429xx
XI_COMPILER_FLAGS += -mcpu=cortex-m4
XI_COMPILER_FLAGS += -mfloat-abi=hard

# we need HAL for random & networking
XI_COMPILER_FLAGS += -DUSE_HAL_DRIVER
# HSE crystal fequency in Hz
XI_COMPILER_FLAGS += -DHSE_VALUE=8000000
#XI_COMPILER_FLAGS += -std=gnu11
#XI_COMPILER_FLAGS += -MMD
#XI_COMPILER_FLAGS += -MP

ifneq (,$(findstring release,$(TARGET)))
	XI_COMPILER_FLAGS += -Os
endif

ifneq (,$(findstring debug,$(TARGET)))
	XI_COMPILER_FLAGS += -O0 -g
endif

# Xively Client config flags
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

XI_COMPILER_FLAGS += -DLWIP_TIMEVAL_PRIVATE=0
# wolfssl API
XI_CONFIG_FLAGS += -DNO_WRITEV
XI_COMPILER_FLAGS += -DSINGLE_THREADED

XI_ARFLAGS += -rs -c $(XI)

ifeq ($(XI_HOST_PLATFORM),Linux)
	# linux cross-compilation prerequisite downloads

ifdef XI_TRAVIS_BUILD
### TOOLCHAIN AUTODOWNLOAD SECTION --- BEGIN

	XI_BUILD_PRECONDITIONS += STM32FX_SDK
.PHONY : STM32FX_SDK
STM32FX_SDK:
	git clone -b st git@github.com:xively/xively-client-artifactory.git $(HOME)/Downloads/xively-client-artifactory

### TOOLCHAIN AUTODOWNLOAD SECTION --- END
endif

endif

XI_POST_COMPILE_ACTION =
