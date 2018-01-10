# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

#
# DDB, NOTE: this was copied from the previous Makefile configuration's avr make target.  I'm not sure how this works
# nor how I should really be testing it so I'm putting it here for now.
#

AVRTEST_SRCDIR ?= ../import/avrtest/
AVRTEST_CFLAGS += -I$(AVRTEST_SRCDIR)
AVRTEST_CFLAGS += -W -Wall -Wno-unused-parameter -Wno-attributes

XI_INCLUDE_FLAGS += -I$(AVRTEST_SRCDIR)

XI_TEST_ELF := $(XI_BINDIR)/libxively_avr_unit_test.elf
XI_TEST_SIM := $(XI_BINDIR)/avrtest

XI_UTEST_SOURCES += $(XI_OBJDIR)/avrtest/exit.o
XI_TEST_DEPENDS += $(XI_TEST_SIM)

$(XI_BINDIR)/libxively_avr_unit_test: $(XI_TEST_ELF) $(XI_TEST_DEPENDS)
	@-mkdir -p $(dir $@)
	$(XI_TEST_SIM) -mmcu=avr6 $(XI_TEST_ELF)

$(XI_TEST_ELF): $(XI_UTEST_SOURCES) $(XI_TEST_DEPENDS)
	@-mkdir -p $(dir $@)
	$(CC) $(XI_CONFIG_FLAGS) $(XI_INCLUDE_FLAGS) $(XI_UTEST_SOURCES) -o $@

$(AVRTEST_SRCDIR)/gen-flag-tables: $(AVRTEST_SRCDIR)/gen-flag-tables.c
	gcc $(AVRTEST_CFLAGS) $^ -o $@

$(AVRTEST_SRCDIR)/flag-tables.c: $(AVRTEST_SRCDIR)/gen-flag-tables
	$^ > $@

$(XI_TEST_SIM): $(AVRTEST_SRCDIR)/flag-tables.c $(AVRTEST_SRCDIR)/avrtest.c
	@-mkdir -p $(dir $@)
	gcc $(AVRTEST_CFLAGS) $(AVRTEST_SRCDIR)/avrtest.c -o $@

$(XI_OBJDIR)/avrtest/exit.o: $(AVRTEST_SRCDIR)/dejagnuboards/exit.c
	@-mkdir -p $(dir $@)
	avr-gcc $(XI_CONFIG_FLAGS) -mmcu=atmega2560 -c $^ -o $@
