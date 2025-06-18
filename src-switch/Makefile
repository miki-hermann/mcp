##**************************************************************************
##*                                                                        *
##*                                                                        *
##*	       Multiple Characterization Problem (MCP)                     *
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
##*     File:    src-switch/Makefile                                       *
##*                                                                        *
##*      Copyright (c) 2019 - 2025                                         *
##*                                                                        *
##*                                                                        *
##*                                                                        *
##**************************************************************************

BIN = ../bin
GXX_VERSION := -std=c++23
# GXX_VERSION :=
MINIMAL_FLAGS := -O4
VANILLA_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -Wfatal-errors
SSL_CRYPTO :=  -lssl -lcrypto
RELEASE_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -lssl -lcrypto -Wall -Werror -Wfatal-errors
DEBUG_FLAGS := $(GXX_VERSION) -g -Og -fopenmp -Wall -Werror -Wfatal-errors -fsanitize=undefined -fsanitize=address  -lssl -lcrypto
GXX   := g++
CXX   := $(GXX) $(VANILLA_FLAGS)
# CXX   := $(GXX) $(RELEASE_FLAGS)
# CXX   := $(GXX) $(DEBUG_FLAGS)
.PHONY: compile generate switch module scratch \
	prepare-seine prepare-danube prepare-mekong

compile: switch

generate: module

#------------------------------------------------------------------------------

prepare-seine:
	mcp-switch --default seine

prepare-danube:
	mcp-switch --default danube

prepare-mekong:
	mcp-switch --default mekong

#------------------------------------------------------------------------------

mcp-sha3.o: mcp-sha3.cpp mcp-sha3.hpp
	$(CXX) -c -o $@ mcp-sha3.cpp $(SSL_CRYPTO)

mcp-basic.o: mcp-basic.cpp mcp-basic.hpp
	$(CXX) -c -o $@ mcp-basic.cpp

#------------------------------------------------------------------------------

switch: $(BIN)/mcp-switch

mcp-switch.o: mcp-switch.cpp mcp-sha3.hpp mcp-version.hpp mcp-basic.hpp
	$(CXX) -c -o $@ mcp-switch.cpp $(SSL_CRYPTO)

$(BIN)/mcp-switch: mcp-basic.o mcp-sha3.o mcp-switch.o
	$(CXX) -fuse-linker-plugin  -o $(BIN)/mcp-switch \
		mcp-switch.o \
		mcp-sha3.o \
		mcp-basic.o \
		$(SSL_CRYPTO)

#------------------------------------------------------------------------------

module: $(BIN)/mcp-module

mcp-version.o: mcp-version.cpp mcp-version.hpp mcp-basic.hpp
	$(CXX) -c -o $@ mcp-version.cpp $(SSL_CRYPTO)

mcp-module.o: mcp-module.cpp mcp-sha3.hpp mcp-version.hpp mcp-basic.hpp
	$(CXX) -c -o $@ mcp-module.cpp $(SSL_CRYPTO)

$(BIN)/mcp-module: mcp-basic.o mcp-sha3.o mcp-version.o mcp-module.o
	$(CXX) -fuse-linker-plugin  -o $(BIN)/mcp-module \
		mcp-module.o \
		mcp-version.o \
		mcp-sha3.o \
		mcp-basic.o \
		$(SSL_CRYPTO)

#------------------------------------------------------------------------------

clean:
	rm -f mcp-version.cpp


scratch: clean
	rm -f *.o
	rm -f *~

#------------------------------------------------------------------------------
# EOF
