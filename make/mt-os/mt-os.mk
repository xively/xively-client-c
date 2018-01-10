# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_CONST_PLATFORM_LINUX := linux
XI_CONST_PLATFORM_OSX := osx
XI_CONST_PLATFORM_MBED := mbed
XI_CONST_PLATFORM_ARM := arm-linux
XI_CONST_PLATFORM_WMSDK := wmsdk
XI_CONST_PLATFORM_MICROCHIP := microchip
XI_CONST_PLATFORM_CC3200 := cc3200
XI_CONST_PLATFORM_CC3220SF := cc3220sf
XI_CONST_PLATFORM_STM32FX := stm32fx
XI_CONST_PLATFORM_STM32FX_NUCLEO_WIFI := stm32fx_nucleo_wifi
XI_CONST_PLATFORM_ESP32 := esp32

ifneq (,$(findstring $(XI_CONST_PLATFORM_ARM),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_ARM)
else
ifneq (,$(findstring $(XI_CONST_PLATFORM_LINUX),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_LINUX)
endif
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_OSX),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_OSX)
  XI_BUILD_OSX=1
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_MBED),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_MBED)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_MICROCHIP),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_MICROCHIP)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_WMSDK),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_WMSDK)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_CC3220SF),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_CC3220SF)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_CC3200),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_CC3200)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_STM32FX),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_STM32FX)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_STM32FX_NUCLEO_WIFI),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_STM32FX_NUCLEO_WIFI)
endif

ifneq (,$(findstring $(XI_CONST_PLATFORM_ESP32),$(TARGET)))
  XI_CONST_PLATFORM_CURRENT := $(XI_CONST_PLATFORM_ESP32)
endif

XI_BINDIR := $(XI_BINDIR_BASE)/$(XI_CONST_PLATFORM_CURRENT)
XI_OBJDIR := $(XI_OBJDIR_BASE)/$(XI_CONST_PLATFORM_CURRENT)
