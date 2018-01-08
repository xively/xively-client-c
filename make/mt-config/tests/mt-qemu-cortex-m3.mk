# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

XI_TEST_SIM := qemu-system-arm -machine integratorcp -cpu cortex-m3 -nographic -monitor null -serial null -semihosting -kernel

ifeq ($(XI_HOST_PLATFORM),Linux)
    XI_RUN_UTESTS := $(XI_TEST_SIM) $(XI_UTESTS) -append "-l0"
    XI_RUN_ITESTS := $(XI_TEST_SIM) $(XI_ITESTS)
else ifeq ($(XI_HOST_PLATFORM),Darwin)
    XI_RUN_UTESTS := @echo "WARNING: no qemu on OSX yet, utests are not executed"
    XI_RUN_ITESTS := @echo "WARNING: no qemu on OSX yet, itests are not executed"
endif
