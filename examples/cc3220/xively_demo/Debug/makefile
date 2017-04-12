################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./config_file.obj" \
"./main_tirtos.obj" \
"./xively_example.obj" \
"./platformDrivers/CC3220SF_LAUNCHXL.obj" \
"./platformDrivers/bma222drv.obj" \
"./platformDrivers/platform.obj" \
"./platformDrivers/tmp006drv.obj" \
"./platformDrivers/uart_term.obj" \
"./simplelinkWifi/provisioning_task.obj" \
"./simplelinkWifi/simplelink_callbacks.obj" \
"../tirtos/ccs/CC3220SF_LAUNCHXL_TIRTOS.cmd" \
$(GEN_CMDS__FLAG) \
-l"C:/ti/simplelink_cc3220_sdk_1_01_02_00/source/ti/drivers/net/wifi/ccs/tirtos/simplelink.a" \
-l"E:/dev/xively/xively-client-c/bin/cc3220/libxively.a" \
-l"C:/ti/simplelink_cc3220_sdk_1_01_02_00/source/ti/devices/cc32xx/driverlib/ccs/Release/driverlib.a" \
-llibc.a \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include platformDrivers/subdir_vars.mk
-include simplelinkWifi/subdir_vars.mk
-include tirtos/ccs/subdir_vars.mk
-include subdir_rules.mk
-include platformDrivers/subdir_rules.mk
-include simplelinkWifi/subdir_rules.mk
-include tirtos/ccs/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
XivelyExample.out \

EXE_OUTPUTS__QUOTED += \
"XivelyExample.out" \

BIN_OUTPUTS += \
XivelyExample.bin \

BIN_OUTPUTS__QUOTED += \
"XivelyExample.bin" \


# All Target
all: XivelyExample.out

# Tool invocations
XivelyExample.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM Linker'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib -me -g --define=CC32XXWARE --define=USER_TIME=1 --define=HAVE_CERTIFICATE_STATUS_REQUEST --define=cc3200 --define=HAVE_OCSP --define=HAVE_SNI --define=HAVE_TLS_EXTENSIONS --define=NO_DES3 --define=NO_DES --define=NO_DH --define=NO_DSA --define=NO_ERROR_STRINGS --define=NO_HC128 --define=NO_MD4 --define=NO_OLD_TLS --define=NO_PSK --define=NO_PWDBASED --define=NO_RABBIT --define=NO_RC4 --define=NO_SHA512 --define=NO_STDIO_FILESYSTEM --define=NO_WOLFSSL_DIR --define=NO_WOLFSSL_SERVER --define=SIZEOF_LONG_LONG=8 --define=SMALL_SESSION_CACHE --define=WOLFSSL_IAR_ARM --define=WOLFSSL_STATIC_RSA --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi -z -m"XivelyExample.map" --stack_size=0x2000 --heap_size=0x8000 -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/lib" -i"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/include" --reread_libs --display_error_number --warn_sections --diag_wrap=off --xml_link_info="XivelyExample_linkInfo.xml" --rom_model -o "XivelyExample.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

XivelyExample.bin: $(EXE_OUTPUTS)
	@echo 'Invoking: ARM Hex Utility'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armhex"  -o "XivelyExample.bin" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '
	@$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "config_file.obj" "main_tirtos.obj" "xively_example.obj" "platformDrivers\CC3220SF_LAUNCHXL.obj" "platformDrivers\bma222drv.obj" "platformDrivers\platform.obj" "platformDrivers\tmp006drv.obj" "platformDrivers\uart_term.obj" "simplelinkWifi\provisioning_task.obj" "simplelinkWifi\simplelink_callbacks.obj" 
	-$(RM) "config_file.d" "main_tirtos.d" "xively_example.d" "platformDrivers\CC3220SF_LAUNCHXL.d" "platformDrivers\bma222drv.d" "platformDrivers\platform.d" "platformDrivers\tmp006drv.d" "platformDrivers\uart_term.d" "simplelinkWifi\provisioning_task.d" "simplelinkWifi\simplelink_callbacks.d" 
	-@echo 'Finished clean'
	-@echo ' '

post-build:
	-"C:/ti/ccsv7/utils/tiobj2bin/tiobj2bin" "XivelyExample.out" "XivelyExample.bin" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armofd" "C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armhex" "C:/ti/ccsv7/utils/tiobj2bin/mkhex4bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

