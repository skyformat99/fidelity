## environment

SHELL = /bin/bash -o pipefail

## toolchain

CCACHE := $(shell command -v ccache)
export CC  = $(CCACHE) $(XPATH)-gcc
export CXX = $(CCACHE) $(XPATH)-g++
export AR  = $(XPATH)-ar

## builddirs

export OBJDIR = obj_$(XARCH)
export BUILD  = $(CURDIR)/build_$(XARCH)
export STAGE  = $(BUILD)/stage
export STAGE_APP  = $(STAGE)_app
export STAGE_LIB  = $(STAGE)_lib
export STAGE_TEST = $(STAGE)/opt/test

## build options

export CXXFLAGS = -std=c++17 -O2 -g \
                  -Werror -Wall -Wextra -pedantic \
                  -pthread \
                  -I$(CURDIR) \
                  -isystem $(CURDIR)/extern

export COVFLAGS = -g -O0 -fprofile-arcs -ftest-coverage

export LDFLAGS  = -pthread -L$(STAGE_LIB) \
                  -lcontract

export LDGTEST  = -lgtest-tap -lgtest -lgmock -lcontract -lgcov

## targets

all: fidelity test demo extern

fidelity: extern
	$(MAKE) -C fidelity

test: fidelity extern
	$(MAKE) -C fidelity test
	$(MAKE) -C test scenario

demo: fidelity extern
	$(MAKE) -C test demo

extern:
	$(MAKE) -C extern

# run unit tests in minimal x86 environment

check: TEST_OPTS = --gtest_output="xml:/test/report/" --gtest_filter=*
check: BIND_OPTS = -b $(BUILD)/test:/test -b $(XROOT)/usr/lib:/usr/lib
check: fidelity extern
ifeq ($(XARCH),x86)
	$(MAKE) -C fidelity test
	rm -rf $(BUILD)/test
	cp -a $(STAGE_TEST) $(BUILD)/test
	PROOT_NO_SECCOMP=1 \
	  xchroot $(BIND_OPTS) -r $$(xroot-init) /test/fidelity/fidelity.unit $(TEST_OPTS)
	test/test-report.sh $(BUILD)/test/report/fidelity.unit
endif

# run unit test coverage in minimal x86 environment

cov: BIND_OPTS = -b $(BUILD)/cov:/cov -b $(XROOT)/usr/lib:/usr/lib
cov: export GCOV_PREFIX = /cov/
cov: export GCOV_PREFIX_STRIP = 5
cov: extern
ifeq ($(XARCH),x86)
	$(MAKE) -C fidelity cov
	PROOT_NO_SECCOMP=1 xchroot $(BIND_OPTS) -r $$(xroot-init) /cov/fidelity.unit
	lcov -q -c -d $(BUILD)/cov \
	  -o $(BUILD)/cov/fidelity.unit.total.cov --gcov-tool $(XPATH)-gcov --rc lcov_branch_coverage=1
	lcov -q -r $(BUILD)/cov/fidelity.unit.total.cov '/opt/*' '*/test/*' '*/extern/*' \
	  -o $(BUILD)/cov/fidelity.unit.cov --rc lcov_branch_coverage=1
	genhtml -q --branch-coverage --demangle-cpp --legend \
	  -o $(BUILD)/cov/fidelity.unit.html $(BUILD)/cov/fidelity.unit.cov
endif

# create doxygen documentation

doc:
	(cat doc/doxygen.conf; echo "PROJECT_NUMBER = $(VERSION)") | doxygen -

# run clang formater

define format_files
  $(foreach type,$(1),$(shell find fidelity -name "*.$(type)"))
endef

format:
	for file in $(call format_files,cpp hpp ipp c h); do \
		echo "formatting $$file"; \
		clang-format -i $$file; \
	done

# run clang-tidy

tidy: SRCS = $(shell find fidelity test/demo -name "*.cpp" -not -path "*/test/*")
tidy:
	clang-tidy $(SRCS) -- $(CXXFLAGS) -isystem $(XROOT)/usr/include

iwyu: SRCS = $(shell find fidelity test -name "*.cpp")
iwyu:
	for src in $(SRCS); do \
	  include-what-you-use -Xiwyu --mapping_file=fidelity.imp \
	    $$src $(CXXFLAGS) -isystem $(XROOT)/usr/include; \
	done

# install staging folder

install: DESTDIR ?= install_$(XARCH)
install: all
	cp -r $(STAGE)/. $(DESTDIR)

install-lib: DESTDIR ?= install-lib_$(XARCH)
install-lib: all
	cp -r $(STAGE_LIB)/. $(DESTDIR)

$(BUILD)/all.done: all

$(DESTDIR):
	mkdir -p $(DESTDIR)

# cleanup

CLEANDIRS = fidelity test extern

clean: XARCH = {arm,x86}
clean: $(CLEANDIRS:%=%.clean)
	rm -rf $(BUILD) doc/html
	find . -name .DS_Store -exec rm {} \;

%.clean:
	$(MAKE) -C $* clean

.PHONY: fidelity test extern doc
