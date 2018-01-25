# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

include make/mt-os/mt-os-common.mk
include make/mt-utils/mt-get-gnu-arm-toolchain.mk

XI_STM32_PATH_SDK = $(HOME)/Downloads/xively-client-artifactory/st/STM32CubeExpansion_WIFI1_V3.0.2

$(info .    CC:                  [$(CC)] )
$(info .    AR:                  [$(AR)] )
$(info .    XI_STM32_PATH_SDK:   [$(XI_STM32_PATH_SDK)] )

# This BSP folder MUST be included before the SDK's X-NUCLEO-IDW0xx1/.
# More info available in src/bsp/platform/stm32fx_nucleo_wifi/stm32_spwf_wifi.h
XI_COMPILER_FLAGS += -I$(XI_BSP_DIR)/platform/$(XI_BSP_PLATFORM)
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/ST/STM32_SPWF0xSy/Utils
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Middlewares/ST/STM32_SPWF0xSy/Inc
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/BSP/X-NUCLEO-IDW0xx1
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/CMSIS/Device/ST/STM32F4xx/Include
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/STM32F4xx_HAL_Driver/Inc
XI_COMPILER_FLAGS += -I$(XI_STM32_PATH_SDK)/Drivers/BSP/STM32F4xx-Nucleo


####################
# arm-eabi-specifics
####################
# create dummy fsctl and unistd methods for gnu newlib
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
XI_COMPILER_FLAGS += -DSTM32F401xE
XI_COMPILER_FLAGS += -DUSE_STM32F4XX_NUCLEO
# we need HAL for random & networking
XI_COMPILER_FLAGS += -DUSE_HAL_DRIVER
# HSE crystal fequency in Hz
XI_COMPILER_FLAGS += -DHSE_VALUE=8000000

ifneq (,$(findstring release,$(TARGET)))
	XI_COMPILER_FLAGS += -O4
endif

ifneq (,$(findstring debug,$(TARGET)))
	XI_COMPILER_FLAGS += -O0 -g
endif

# Xively Client config flags
XI_CONFIG_FLAGS += -DXI_CROSS_TARGET
XI_CONFIG_FLAGS += -DXI_EMBEDDED_TESTS

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
