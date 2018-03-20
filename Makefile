# Configuration options.
cc         = gcc
openmp     = set
prefix     = ~/local
gdb        = not-set
assert     = not-set
assert2    = not-set
debug      = not-set
gprof      = not-set
shared     = not-set
gklib_path = not-set


# Basically proxies everything to the builddir cmake.

cputype = $(shell uname -m | sed "s/\\ /_/g")
systype = $(shell uname -s)

BUILDDIR = build/$(systype)-$(cputype)

# Process configuration options.
CONFIG_FLAGS = -DCMAKE_VERBOSE_MAKEFILE=1
ifneq ($(gklib_path), not-set)
    CONFIG_FLAGS += -DGKLIB_PATH=$(abspath $(gklib_path))
endif
ifneq ($(gdb), not-set)
    CONFIG_FLAGS += -DGDB=$(gdb)
endif
ifneq ($(assert), not-set)
    CONFIG_FLAGS += -DASSERT=$(assert)
endif
ifneq ($(assert2), not-set)
    CONFIG_FLAGS += -DASSERT2=$(assert2)
endif
ifneq ($(debug), not-set)
    CONFIG_FLAGS += -DDEBUG=$(debug)
endif
ifneq ($(gprof), not-set)
    CONFIG_FLAGS += -DGPROF=$(gprof)
endif
ifneq ($(openmp), not-set)
    CONFIG_FLAGS += -DOPENMP=$(openmp)
endif
ifneq ($(prefix), not-set)
    CONFIG_FLAGS += -DCMAKE_INSTALL_PREFIX=$(prefix)
endif
ifneq ($(shared), not-set)
    CONFIG_FLAGS += -DSHARED=1
endif
ifneq ($(cc), not-set)
    CONFIG_FLAGS += -DCMAKE_C_COMPILER=$(cc)
endif

CONFIG_FLAGS += -DPKGNAME=gktc

define run-config
mkdir -p $(BUILDDIR)
cd $(BUILDDIR) && cmake $(CURDIR) $(CONFIG_FLAGS)
endef

all clean install:
	@if [ ! -f $(BUILDDIR)/Makefile ]; then \
		more README.md; \
	else \
	  	make -C $(BUILDDIR) $@ $(MAKEFLAGS); \
	fi

uninstall:
	xargs rm < $(BUILDDIR)/install_manifest.txt

config: distclean
	$(run-config)

distclean:
	rm -rf $(BUILDDIR)

remake:
	find . -name CMakeLists.txt -exec touch {} ';'

.PHONY: config distclean all clean install uninstall remake
