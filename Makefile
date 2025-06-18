##**************************************************************************
##*                                                                        *
##*                                                                        *
##*	       Multiple Classification   Problem (MCP)                     *
##*                                                                        *
##*	Author:   Miki Hermann                                             *
##*	e-mail:   hermann@lix.polytechnique.fr                             *
##*	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France         *
##*                                                                        *
##*	Author: Gernot Salzer                                              *
##*	e-mail: gernot.salzer@tuwien.ac.at                                 *
##*	Address: Technische Universitaet Wien, Vienna, Austria             *
##*                                                                        *
##*	Version: all                                                       *
##*     File:    solver/Makefile                                           *
##*                                                                        *
##*      Copyright (c) 2019 - 2025                                         *
##*                                                                        *
##* Compile and install the MCP system.                                    *
##*                                                                        *
##**************************************************************************

# variable holding the default version of the MCP system
DEFAULT ?= $(shell bash -c 'read -p "Set MCP default version (mekong | danube |  seine): " default; echo $$default')
GXX   := g++
GXX_VERSION := -std=c++23
# variable holding the path to executables, must be part of PATH
BIN := bin
EXECUTABLES := /usr/local/bin
MANPAGES := /usr/local/share/man
SUDO := sudo

.PHONY: all complete \
	prequel compile \
	compile-and-install switch-and-install generate-and-install \
	compile-danube compile-mekong compile-seine \
	compile-switch \
	seine danube mekong switch noarch \
	man install \
	clean scratch

all: prequel compile-and-install switch-and-install generate-and-install

complete: all man

prequel:
	mkdir -p $(BIN)

compile-and-install: compile
	$(SUDO) mkdir -p $(EXECUTABLES)
	$(SUDO) cp -f $(BIN)/mcp-* $(EXECUTABLES)

switch-and-install: switch
	$(SUDO) mkdir -p $(EXECUTABLES)
	$(SUDO) cp -f $(BIN)/mcp-switch $(EXECUTABLES)

generate-and-install: generate
	$(SUDO) mkdir -p $(EXECUTABLES)
	$(SUDO) cp -f $(BIN)/mcp-module $(EXECUTABLES)
	$(SUDO) ln -sf $(EXECUTABLES)/mcp-module $(EXECUTABLES)/mcp-seq
	$(SUDO) ln -sf $(EXECUTABLES)/mcp-module $(EXECUTABLES)/mcp-pthread
	$(SUDO) ln -sf $(EXECUTABLES)/mcp-module $(EXECUTABLES)/mcp-trans
	$(SUDO) ln -sf $(EXECUTABLES)/mcp-module $(EXECUTABLES)/mcp-check
	$(SUDO) ln -sf $(EXECUTABLES)/mcp-module $(EXECUTABLES)/mcp-predict

compile: noarch seine danube mekong

generate:
	@echo +++
	@echo +++ make sure that you compiled and installed
	@echo ... mcp-switch
	@echo +++
	$(MAKE) -C src-switch prepare-$(DEFAULT)
	$(MAKE) -C src-switch generate

switch:
	$(MAKE) -C src-switch compile

danube:
	$(MAKE) -C src-danube compile GXX=$(GXX) GXX_VERSION=$(GXX_VERSION)

mekong:
	$(MAKE) -C src-mekong compile GXX=$(GXX) GXX_VERSION=$(GXX_VERSION)

seine:
	$(MAKE) -C src-seine compile GXX=$(GXX) GXX_VERSION=$(GXX_VERSION)

noarch:
	$(MAKE) -C src-noarch compile GXX=$(GXX) GXX_VERSION=$(GXX_VERSION)

man:
	$(MAKE) -C man MANPAGES=$(MANPAGES) SUDO=$(SUDO)

install:
	$(SUDO) mkdir -p $(EXECUTABLES)
	$(SUDO) cp -f $(BIN)/mcp-* $(EXECUTABLES)

clean:
	rm -f $(BIN)/mcp-*
	rm -f src-*/*.o

scratch: clean
	$(MAKE) -C src-switch scratch
	rm -f *~
