ifeq ($(XI_HOST_PLATFORM),Linux)
	# linux cross-compilation assumes tools downloaded and are on PATH

	XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2
	XI_GCC_ARM_NONE_EABI_PATH ?= ~/Downloads/gcc-arm-none-eabi-5_4-2016q2

	CC = $(XI_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-gcc
	AR = $(XI_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-ar

	XI_GCC_ARM_TOOLCHAIN_URL := https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-linux.tar.bz2

	#export PATH=$PATH:$(XI_GCC_ARM_NONE_EABI_PATH)/bin

	XI_BUILD_PRECONDITIONS := $(CC)

else ifeq ($(XI_HOST_PLATFORM),Darwin)
	# osx cross-compilation downloads arm-gcc

	XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE = ~/Downloads/gcc-arm-none-eabi-5_4-2016q2-20160622-mac.tar.bz2
	XI_GCC_ARM_NONE_EABI_PATH ?= ~/Downloads/gcc-arm-none-eabi-5_4-2016q2

	CC = $(XI_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-gcc
	AR = $(XI_GCC_ARM_NONE_EABI_PATH)/bin/arm-none-eabi-ar

	XI_GCC_ARM_TOOLCHAIN_URL := https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-mac.tar.bz2

	XI_BUILD_PRECONDITIONS := $(CC)

else ifeq ($(XI_HOST_PLATFORM),Windows_NT)
	CC = arm-none-eabi-gcc
	AR = arm-none-eabi-ar
endif

$(XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE):
	@echo "XI ARM-GCC BUILD: downloading arm-gcc toolchain to file $(XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE)"
	@-mkdir -p $(dir $@)
	@curl -L -o $(XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE) $(XI_GCC_ARM_TOOLCHAIN_URL)

$(CC): $(XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE)
	@echo "XI ARM-GCC BUILD: extracting arm-gcc toolchain"
	@tar -xf $(XI_GCC_ARM_NONE_EABI_DOWNLOAD_FILE) -C ~/Downloads
	@touch $@
	$@ --version
