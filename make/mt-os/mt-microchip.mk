# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

CC = /Applications/microchip/xc32/v1.34/bin/xc32-gcc
#AR = /Applications/microchip/xc32/v1.34/bin/xc32-ar
AR = /Applications/microchip/xc32/v1.34/bin/xc32-gcc

include make/mt-os/mt-os-common.mk

XI_ARFLAGS += -mprocessor=32MX695F512H  -o
XI_ARFLAGS += $(XI)
#XI_EXTRA_ARFLAGS += -mdebugger -D__MPLAB_DEBUGGER_REAL_ICE=1 -mprocessor=$(MP_PROCESSOR_OPTION)
#XI_EXTRA_ARFLAGS += -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC02000:0x1FC02FEF
#XI_EXTRA_ARFLAGS += -mreserve=boot@0x1FC02000:0x1FC024FF
#XI_EXTRA_ARFLAGS += -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_REAL_ICE=1,--defsym=_min_heap_size=16000,--defsym=_min_stack_size=2048,-L"..",-Map="$(BINDIR_)$(TARGETBASE).map",--report-mem "-mperipheral-libs"

XI_EXTRA_ARFLAGS += -Wl,--defsym=__MPLAB_BUILD=1,--defsym=_min_heap_size=16000,--defsym=_min_stack_size=2048,-L"..",-Map=".map",--report-mem "-mperipheral-libs" -s

XI_COMPILER_FLAGS += -I/Users/ddellabitta/microchip_solutions_v2013-06-15/Microchip/Include
XI_COMPILER_FLAGS += -I"/Users/ddellabitta/microchip_solutions_v2013-06-15/Microchip/Include/TCPIP Stack"
XI_CONFIG_FLAGS += -DCREDENTIALS_NO_STORE
