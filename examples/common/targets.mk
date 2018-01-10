# Copyright (c) 2003-2018, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

all: $(XI_EXAMPLE_BIN)

-include $(XI_EXAMPLE_DEPS)

$(XI_EXAMPLE_OBJDIR)/common/%.o : $(CURDIR)/../common/src/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(XI_FLAGS_COMPILER) $(XI_FLAGS_INCLUDE) -c $< -o $@
	$(MD) $(CC) $(XI_FLAGS_COMPILER) $(XI_FLAGS_INCLUDE) -MM $< -MT $@ -MF $(@:.o=.d)

$(XI_EXAMPLE_OBJDIR)/%.o : $(XI_EXAMPLE_SRCDIR)/%.c
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	$(MD) $(CC) $(XI_FLAGS_COMPILER) $(XI_FLAGS_INCLUDE) -c $< -o $@
	$(MD) $(CC) $(XI_FLAGS_COMPILER) $(XI_FLAGS_INCLUDE) -MM $< -MT $@ -MF $(@:.o=.d)

$(XI_EXAMPLE_BIN) : $(XI_EXAMPLE_OBJS)
	$(info [$(CC)] $@)
	@-mkdir -p $(dir $@)
	@cp $(XI_CLIENT_ROOTCA_LIST) $(dir $@)
	$(MD) $(CC) $(XI_EXAMPLE_OBJS) $(XI_FLAGS_LINKER) -o $@

clean:
	$(info [clean] $(XI_EXAMPLE_NAME) )
	@rm -rf $(XI_EXAMPLE_OBJDIR)
	@rm -rf $(XI_EXAMPLE_BINDIR)
