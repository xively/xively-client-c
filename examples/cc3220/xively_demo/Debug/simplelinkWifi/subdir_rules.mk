################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
simplelinkWifi/provisioning_task.obj: ../simplelinkWifi/provisioning_task.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib -me --include_path="E:/dev/xively/xively-client-c/examples/cc3220/xively_demo" --include_path="E:/dev/xively/xively-client-c/src/import/tls/wolfssl" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/source/ti/devices/cc32xx/driverlib" --include_path="E:/dev/xively/xively-client-c/include" --include_path="E:/dev/xively/xively-client-c/include/bsp" --include_path="E:/dev/xively/xively-client-c/examples/cc3220/xively_demo" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/os/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/source" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/include" -g --define=CC32XXWARE --define=USER_TIME=1 --define=HAVE_CERTIFICATE_STATUS_REQUEST --define=cc3200 --define=HAVE_OCSP --define=HAVE_SNI --define=HAVE_TLS_EXTENSIONS --define=NO_DES3 --define=NO_DES --define=NO_DH --define=NO_DSA --define=NO_ERROR_STRINGS --define=NO_HC128 --define=NO_MD4 --define=NO_OLD_TLS --define=NO_PSK --define=NO_PWDBASED --define=NO_RABBIT --define=NO_RC4 --define=NO_SHA512 --define=NO_STDIO_FILESYSTEM --define=NO_WOLFSSL_DIR --define=NO_WOLFSSL_SERVER --define=SIZEOF_LONG_LONG=8 --define=SMALL_SESSION_CACHE --define=WOLFSSL_IAR_ARM --define=WOLFSSL_STATIC_RSA --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="simplelinkWifi/provisioning_task.d" --obj_directory="simplelinkWifi" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

simplelinkWifi/simplelink_callbacks.obj: ../simplelinkWifi/simplelink_callbacks.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=vfplib -me --include_path="E:/dev/xively/xively-client-c/examples/cc3220/xively_demo" --include_path="E:/dev/xively/xively-client-c/src/import/tls/wolfssl" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/source/ti/devices/cc32xx/driverlib" --include_path="E:/dev/xively/xively-client-c/include" --include_path="E:/dev/xively/xively-client-c/include/bsp" --include_path="E:/dev/xively/xively-client-c/examples/cc3220/xively_demo" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/os/tirtos/packages/ti/sysbios/posix" --include_path="C:/ti/simplelink_cc3220_sdk_1_01_02_00/source" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-arm_16.9.0.LTS/include" -g --define=CC32XXWARE --define=USER_TIME=1 --define=HAVE_CERTIFICATE_STATUS_REQUEST --define=cc3200 --define=HAVE_OCSP --define=HAVE_SNI --define=HAVE_TLS_EXTENSIONS --define=NO_DES3 --define=NO_DES --define=NO_DH --define=NO_DSA --define=NO_ERROR_STRINGS --define=NO_HC128 --define=NO_MD4 --define=NO_OLD_TLS --define=NO_PSK --define=NO_PWDBASED --define=NO_RABBIT --define=NO_RC4 --define=NO_SHA512 --define=NO_STDIO_FILESYSTEM --define=NO_WOLFSSL_DIR --define=NO_WOLFSSL_SERVER --define=SIZEOF_LONG_LONG=8 --define=SMALL_SESSION_CACHE --define=WOLFSSL_IAR_ARM --define=WOLFSSL_STATIC_RSA --diag_wrap=off --diag_warning=225 --diag_warning=255 --display_error_number --gen_func_subsections=on --abi=eabi --preproc_with_compile --preproc_dependency="simplelinkWifi/simplelink_callbacks.d" --obj_directory="simplelinkWifi" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


