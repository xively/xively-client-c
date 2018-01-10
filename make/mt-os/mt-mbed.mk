# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

#
# NOTE: this was copied from the previous Makefile configuration's mbed make target.  I'm not sure how this works
# nor how I should really be testing it so I'm putting it here for now. This could be useful if we have new customers that use
# mbed as a platform.

MBED_HEAD ?= HEAD
MBED_TEMP ?= mbed_mercurial
MBED_USER ?= xively
MBED_REPO ?= libxively-develop

MBED_URL := https://$(MBED_USER)@mbed.org/users/$(MBED_USER)/code/$(MBED_REPO)/

MBED_REV := $(shell git rev-parse --short --no-symbolic $(MBED_HEAD))

MBED_MSG := "Updated from git revision $(MBED_REV)"

update_mbed_repo:
	hg clone $(MBED_URL) $(MBED_TEMP)
	-rm -rf $(MBED_TEMP)/src
	git archive $(MBED_REV) \
		"src/libxively/*.[ch]" \
		"src/libxively/io/posix/" \
		"src/libxively/datastructures/" \
		"src/libxively/event_dispatcher/" \
		"src/libxively/mqtt/" \
		| tar x -C $(MBED_TEMP)
	echo "#define XI_VERSION \"0.1.x-$(MBED_REV)\"" \
		> $(MBED_TEMP)/src/libxively/xi_version.h
	hg commit --repository $(MBED_TEMP) --addremove \
		--user $(MBED_USER) --message $(MBED_MSG)
	-hg push --repository $(MBED_TEMP)
	-rm -rf $(MBED_TEMP)