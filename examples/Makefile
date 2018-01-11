# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

# Batch Build Tool for Xively C Client Examples
#
# These Examples are built against a pre-built Xively C Client library.
#
# The pre-built Xively C Client library paths are in XI_CLIENT_INC_PATH
# and XI_CLIENT_LIB_PATH in rules.mk. Their default values reflect the
# original relative location of examples to Xively C Client. If this
# relative location is altered the varables shall be adjusted either
# directly in rules.mk or in commandline like
# 'make XI_CLIENT_INC_PATH=new/path/include XI_CLIENT_LIB_PATH=new/path/obj/osx'.
#
# Examples can be batch-built with 'make' in this directory or one-by-one
# 'make' in each example directory.
#
# 	Targets:
#
# 		all: 	builds all examples
#		clean: 	cleans all examples
#
#		[EXAMPLE_NAME]: builds only the specified example
#						'make mqtt_logic_example'
#
# Result binaries are under [EXAMPLE_NAME]/bin
#
# If you wish to create your own application based on a Xively example source and
# makefile structure, then the following steps are suggested:
#		- create a new subdirectory under examples: my_new_example
#		- copy mqtt_logic_example/Makefile to my_new_example/Makefile and initialize
#		  variable XI_EXAMPLE_NAME with 'my_new_example'
#		- put your code into my_new_exapmle/src/my_new_example.c
#		- add the new example to the examples list: XI_EXAMPLES_ALL += my_new_example
#		- execute 'make my_new_example'
#
# It should be 'easy' to alter these makefiles to cross-compile. This requires the
# following modification in rules.mk:
# 		- available cross-compiled Xively C Client library
#		- changing the compiler and linker variables: CC and AR
# 		- adjusting linked libraries

MD=@

XI_EXAMPLES_ALL := mqtt_logic_example
XI_EXAMPLES_ALL += mqtt_logic_producer
XI_EXAMPLES_ALL += mqtt_logic_consumer
XI_EXAMPLES_ALL += senml_serializer
XI_EXAMPLES_ALL += mqtt_last_will
XI_EXAMPLES_ALL += mqtt_logic_example_two_contexts
XI_EXAMPLES_ALL += firmware_update
XI_EXAMPLES_ALL += gateway

all: $(XI_EXAMPLES_ALL)

.PHONY : $(XI_EXAMPLES_ALL)

$(XI_EXAMPLES_ALL) :
	$(MD) $(MAKE) -C $(CURDIR)/$@ all

clean:
	@for M in $(XI_EXAMPLES_ALL); do \
		$(MAKE) -C $(CURDIR)/$$M clean; \
	done;
