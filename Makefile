# Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
#
# This is part of the Xively C Client library,
# it is licensed under the BSD 3-Clause license.

LIBXIVELY := $(CURDIR)

export LIBXIVELY

# Reserve 'all' as the default build target
all:

XI_SRCDIRS ?=
XI_CONFIG_FLAGS ?=
XI_ARFLAGS ?=
BSP_FLAGS ?=
XI_INCLUDE_FLAGS ?= -I.
XI_PROTO_COMPILER ?=
XI_BSP_DIR ?= $(LIBXIVELY)/src/bsp
MD ?= @

include make/mt-config/mt-presets.mk

# TLS related configuration
XI_BSP_TLS ?= wolfssl

include make/mt-config/mt-config
include make/mt-os/mt-os
include make/mt-os/mt-$(XI_CONST_PLATFORM_CURRENT)
include make/mt-config/mt-examples
include make/mt-config/tests/mt-tests-tools
include make/mt-config/tests/mt-tests-unit
include make/mt-config/tests/mt-tests-integration
include make/mt-config/mt-help

ifdef MAKEFILE_DEBUG
$(info ----- )
$(info -TOOLCHAIN- $$CC is [${CC}])
$(info -TOOLCHAIN- $$AR is [${AR}])
$(info ----- )
$(info -TESTS- $$XI_UTESTS is [${XI_UTESTS}])
$(info -TESTS- $$XI_TEST_BINDIR is [${XI_TEST_BINDIR}])
$(info -TESTS- $$XI_TEST_DIR is [${XI_TEST_DIR}])
$(info ----- )
$(info -EXAMPLES- $$XI_EXAMPLES is [${XI_EXAMPLES}])
$(info ----- )
endif

#gather all binary directories
XI_BIN_DIRS := $(XI_BIN_DIR) $(XI_EXAMPLE_BINDIR) $(XI_EXAMPLE_BINDIR)/internal $(XI_TEST_BINDIR) $(XI_TEST_TOOLS_BINDIR)

#default test target always present cause tiny test cross-compiles
XI_TESTS_TARGETS := $(XI_UTESTS) $(XI_TEST_TOOLS_OBJS) $(XI_TEST_TOOLS)

ifneq ($(XI_CONST_PLATFORM_CURRENT),$(XI_CONST_PLATFORM_ARM))
#the integration tests does not cross-compile atm
	XI_TESTS_TARGETS += $(XI_ITESTS)
endif

build_output: help_disclaimer preset_output
	$(info .    CONFIG:          [${CONFIG}])
	$(info .    TARGET:          [${TARGET}])
	$(info .    COMPILER:        [$(CC)] )
	$(info )

all: build_output $(XI)

tests: $(XI) $(XI_TESTS_TARGETS)

internal_examples: $(XI) $(XI_INTERNAL_EXAMPLES)

linux:
	make CONFIG=$(CONFIG) TARGET=$(subst osx,linux,$(TARGET))

buildtime:
	time bash -c make

clean:
	$(RM) -rf \
		$(XI_BINDIR) \
		$(XI_OBJDIR) \
		$(XI_TEST_OBJDIR) \
		$(filter-out %.o %.a %import,$(wildcard $(XI_OBJDIR)/*))

clean_all: clean
	$(RM) -rf \
		$(XI_BINDIR_BASE) \
		$(XI_OBJDIR_BASE)
	@rm -rf $(CMOCKA_BUILD_DIR)
	@rm -f $(XI_TEST_OBJDIR)

libxively: $(XI)

$(XI): $(TLS_LIB_PATH) $(XI_PROTOFILES_C) $(XI_OBJS) | $(XI_BIN_DIRS)
	$(info [$(AR)] $@ )
	$(MD) $(AR) $(XI_ARFLAGS) $(XI_OBJS) $(XI_EXTRA_ARFLAGS)

# protobuf compilation
$(XI_PROTOBUF_GENERATED)/%.pb-c.c : $(XI_PROTO_DIR)/%.proto
	@-mkdir -p $(dir $@)
	$(XI_PROTO_COMPILER) --c_out=$(XI_PROTOBUF_GENERATED) --proto_path=$(XI_PROTO_DIR) $<

# defining dependencies for object files generated by gcc -MM
# Autodependencies with GNU make: http://scottmcpeak.com/autodepend/autodepend.html
-include $(XI_TEST_TOOLS_OBJS:.o=.d)

# specific compiler flags for utest objects
$(XI_UTEST_OBJDIR)/%.o : $(XI_UTEST_SOURCE_DIR)/%.c $(XI_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_UTEST_CONFIG_FLAGS) $(XI_UTEST_INCLUDE_FLAGS) -c $<
	@$(CC) $(XI_UTEST_CONFIG_FLAGS) $(XI_UTEST_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)

# specific compiler flags for libxively_driver
$(XI_OBJDIR)/tests/tools/xi_libxively_driver/%.o : $(LIBXIVELY)/src/tests/tools/xi_libxively_driver/%.c $(XI_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) $(XI_TEST_TOOLS_INCLUDE_FLAGS) -c $<
	@$(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) $(XI_TEST_TOOLS_INCLUDE_FLAGS) -MM $< -MT $@ -MF $(@:.o=.d)

-include $(XI_OBJS:.o=.d)

$(XI_OBJDIR)/%.o : $(LIBXIVELY)/src/%.c $(XI_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -c $<
	$(XI_POST_COMPILE_ACTION)

$(XI_OBJDIR)/bsp/platform/$(BSP_FOUND)/%.o : $(XI_BSP_DIR)/platform/$(BSP_FOUND)/%.c $(XI_BUILD_PRECONDITIONS)
	@-mkdir -p $(dir $@)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -c $<
	$(XI_POST_COMPILE_ACTION)

# gather all of the binary directories
XI_RESOURCE_FILES := $(LIBXIVELY)/res/trusted_RootCA_certs/xi_RootCA_list.pem

ifneq (,$(findstring posix_fs,$(CONFIG)))
XI_PROVIDE_RESOURCE_FILES = ON
endif

###
#### EXAMPLES
###
-include $(XI_EXAMPLE_OBJDIR)/*.d

$(XI_EXAMPLE_BINDIR)/internal/%: $(XI)
	$(info [$(CC)] $@)
	@-mkdir -p $(XI_EXAMPLE_OBJDIR)/$(subst $(XI_EXAMPLE_BINDIR)/,,$(dir $@))
	$(MD) $(CC) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -L$(XI_BINDIR) $(XI) $(LIBXIVELY)/examples/common/src/commandline.c $(XI_EXAMPLE_DIR)/$(subst $(XI_EXAMPLE_BINDIR),,$@).c $(XI_LIB_FLAGS) -o $@
	@$(CC) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -MM $(XI_EXAMPLE_DIR)/$(subst $(XI_EXAMPLE_BINDIR),,$@).c -MT $@ -MF $(XI_EXAMPLE_OBJDIR)/$(subst $(XI_EXAMPLE_BINDIR)/,,$@).d

###
#### TEST TOOLS
###
-include $(XI_TEST_TOOLS_OBJDIR)/*.d

$(XI_TEST_TOOLS_BINDIR)/%: $(XI) $(XI_TEST_TOOLS_OBJS)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -L$(XI_BINDIR) $(XI_TEST_TOOLS_OBJS) $(XI_TEST_TOOLS_SRCDIR)/$(notdir $@)/$(notdir $@).c $(XI_LIB_FLAGS) -o $@
	@-mkdir -p $(XI_TEST_TOOLS_OBJDIR)
	@$(CC) $(XI_CONFIG_FLAGS) $(XI_COMPILER_FLAGS) $(XI_INCLUDE_FLAGS) -MM $(XI_TEST_TOOLS_SRCDIR)/$(notdir $@)/$(notdir $@).c -MT $@ -MF $(XI_TEST_TOOLS_OBJDIR)/$(notdir $@).d
	@#$@

###
#### TESTS
###
# dependencies for unit test binary
XI_UTESTS_DEPENDENCIES_FILE = $(XI_UTEST_OBJDIR)/$(notdir $(XI_UTESTS)).d
-include $(XI_UTESTS_DEPENDENCIES_FILE)

$(XI_UTESTS): $(XI) $(XI_UTEST_OBJS) $(TINY_TEST_OBJ)
	$(info [$(CC)] $@)
	$(MD) $(CC)  $(XI_UTEST_CONFIG_FLAGS) $(XI_UTEST_INCLUDE_FLAGS) -L$(XI_BINDIR) $(XI_UTEST_SUITE_SOURCE) $(XI_UTEST_OBJS) $(TINY_TEST_OBJ) $(XI_LIB_FLAGS) -o $@
	@-mkdir -p $(XI_UTEST_OBJDIR)
	@$(CC) $(XI_UTEST_CONFIG_FLAGS) $(XI_UTEST_INCLUDE_FLAGS) -MM $(XI_UTEST_SUITE_SOURCE) -MT $@ -MF $(XI_UTESTS_DEPENDENCIES_FILE)
	$(XI_RUN_UTESTS)

# dependencies for integration test binary
ifneq ($(XI_CONST_PLATFORM_CURRENT),$(XI_CONST_PLATFORM_ARM))

-include $(XI_ITEST_OBJS:.o=.d)

$(XI_ITESTS): $(XI) $(CMOCKA_LIBRARY_DEPS) $(XI_ITEST_OBJS)
	$(info [$(CC)] $@)
	$(MD) $(CC) $(XI_ITEST_OBJS) -L$(XI_BINDIR) $(XI_LIB_FLAGS) $(CMOCKA_LIBRARY) -o $@
	$(XI_RUN_ITESTS)

endif

$(XI_BIN_DIRS):
	@mkdir -p $@
ifdef XI_PROVIDE_RESOURCE_FILES
	@cp $(XI_RESOURCE_FILES) $@
endif

libxively: $(XI)

# new rule added in order not to change too much in this file
# without a vision of how we are going to handle cross platform
# compilation of examples and tests
wmsdk_examples: libxively
	@# export required by wmsdk_example rule which is
	@# using the external makefile located in wmsdk examples
	@# until further decisions made of how we should maintain
	@# cross platform compilation of tests and examples I will
	@# use this extra rule in order to build examples binaries that's wmsdk specific
	@export EXTRALIBS=$(XI_BINDIR)/libxively.a; \
	export XI_OBJDIR_BASE=$(XI_OBJDIR_BASE)/$(XI_CONST_PLATFORM_CURRENT)/; \
	export XI_BINDIR=$(XI_BINDIR); \
	export "XI_INCLUDE_FLAGS=$(XI_INCLUDE_FLAGS)"; \
	export "XI_CONFIG_FLAGS=$(XI_CONFIG_FLAGS)"; \
	export "XI_COMPILER_FLAGS=$(XI_COMPILER_FLAGS)"; \
	$(MAKE) -s -C $(LIBXIVELY)/src/examples/wmsdk/ all;

update_docs_branch:
	-rm -rf doc/html
	doxygen && cd doc/html \
		&& git init \
		&& git remote add github git@github.com:xively/libxively \
		&& git add . \
		&& git commit -m "[docs] Regerated documentation for $(REV)" \
		&& git push github master:gh-pages -f

CRT_URL := https://secure.globalsign.net/cacert/Root-R1.crt

ca.pem:
	curl -s $(CRT_URL) \
		| openssl x509 \
			-out $@ \
			-outform pem

update_builtin_cert: ca.pem
	./tools/create_buffer.py \
		--file_name ./ca.pem \
		--array_name xi_ca \
		--out_path ./src/libxively/tls/ \
		--no-pretend
	-rm -rf ./ca.pem
