# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk

# todo: this is common, move it to outside
XI ?= $(XI_BINDIR)/libxively.a

include make/mt-utils/mt-get-gnu-arm-toolchain.mk

XI_STM32_PATH_SDK = $(HOME)/Downloads/STM32Cube_FW_F4_V1.14.0

#####################
# LWIP configurations
#####################
XI_STM32_PATH_SDK_LWIP = $(XI_STM32_PATH_SDK)/Middlewares/Third_Party/LwIP

XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK_LWIP)/src/include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK_LWIP)/system
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/FreeRTOS/Source/include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/STM32F4xx_HAL_Driver/Inc
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Include
XI_COMPILER_FLAGS += -I$(LIBXIVELY)/examples/stm32/xively_demo_mt/Inc

####################
# arm-eabi-specifics
####################
# create dummy fsctl and unistd methods for gnu newlib
#XI_COMPILER_FLAGS += -ffreestanding
# stm32f4 uses cortex m4
XI_COMPILER_FLAGS += -mcpu=cortex-m4
# enable Thumb instruction set ( reduces memory usage )
XI_COMPILER_FLAGS += -mthumb
XI_COMPILER_FLAGS += -mfloat-abi=hard
# diagnostic message length
XI_COMPILER_FLAGS += -fmessage-length=0
# set char type
XI_COMPILER_FLAGS += -fsigned-char
# linker can optimizie better if we use function
XI_COMPILER_FLAGS += -ffunction-sections
# loop optimization
XI_COMPILER_FLAGS += -fno-move-loop-invariants
# define the board type
XI_COMPILER_FLAGS += -DSTM32F407xx
# we need HAL for random & networking
XI_COMPILER_FLAGS += -DUSE_HAL_DRIVER
# HSE crystal fequency in Hz
XI_COMPILER_FLAGS += -DHSE_VALUE=8000000
#XI_COMPILER_FLAGS += -std=gnu11
#XI_COMPILER_FLAGS += -MMD
#XI_COMPILER_FLAGS += -MP

ifneq (,$(findstring release,$(TARGET)))
	XI_COMPILER_FLAGS += -O4
endif

ifneq (,$(findstring debug,$(TARGET)))
	XI_COMPILER_FLAGS += -O0 -g
endif

# Xively Client config flags
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

# wolfssl API
XI_CONFIG_FLAGS += -DNO_WRITEV
XI_COMPILER_FLAGS += -DSINGLE_THREADED

XI_ARFLAGS += -rs -c $(XI)

ifeq ($(XI_HOST_PLATFORM),Linux)
	# linux cross-compilation prerequisite downloads

ifdef XI_TRAVIS_BUILD
### TOOLCHAIN AUTODOWNLOAD SECTION --- BEGIN

	XI_BUILD_PRECONDITIONS += STM32F4_SDK
.PHONY : STM32F4_SDK
STM32F4_SDK:
	git clone -b st git@github.com:xively/xively-client-artifactory.git $(HOME)/Downloads/xively-client-artifactory

### TOOLCHAIN AUTODOWNLOAD SECTION --- END
endif

endif

XI_POST_COMPILE_ACTION =
