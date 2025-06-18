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
##*     File:    src-mekong/Makefile                                       *
##*                                                                        *
##*      Copyright (c) 2019 - 2025                                         *
##*                                                                        *
##* Given  several   matrices  over   natural  numbers   representing  the *
##* presence/absence   of  attributes   in  observations,   this  software *
##* generates Horn,  dual Horn, or  bijunctive formulas from  positive and *
##* negative examples represented by these matrices.                       *
##*                                                                        *
##**************************************************************************

BIN = ../bin
GXX_VERSION := -std=c++23
MINIMAL_FLAGS := -O4
VANILLA_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -Wfatal-errors
RELEASE_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -Wall -Werror -Wfatal-errors
DEBUG_FLAGS := $(GXX_VERSION) -g -Og -fopenmp -Wall -Werror -Wfatal-errors -fsanitize=undefined -fsanitize=address
GXX   := g++
CXX   := $(GXX) $(VANILLA_FLAGS)
# CXX   := $(GXX) $(RELEASE_FLAGS)
# CXX   := $(GXX) $(DEBUG_FLAGS)
VERSION := mekong

.PHONY: compile seq pthread trans check predict scratch

compile: seq pthread trans check predict

#---------------------------------------------------------------------------------------------------

seq: $(BIN)/mcp-seq

mcp-matrix+formula-seq.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-common-seq.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-common.cpp

mcp-bucket-seq.o: mcp-bucket.cpp mcp-bucket.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-bucket.cpp

mcp-mesh-seq.o: mcp-mesh.cpp mcp-mesh.hpp mcp-bucket.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-mesh.cpp

mcp-seq.o: mcp-seq.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-seq.cpp

$(BIN)/mcp-seq: mcp-matrix+formula-seq.o mcp-common-seq.o mcp-bucket-seq.o mcp-mesh-seq.o mcp-seq.o
	$(CXX) -fuse-linker-plugin  -o $(BIN)/mcp-seq-$(VERSION) \
		mcp-seq.o \
		mcp-common-seq.o \
		mcp-matrix+formula-seq.o \
		mcp-bucket-seq.o \
		mcp-mesh-seq.o

#---------------------------------------------------------------------------------------------------

pthread: $(BIN)/mcp-pthread

mcp-matrix+formula-pthread.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-matrix+formula.cpp

mcp-common-pthread.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-common.cpp

mcp-bucket-pthread.o: mcp-bucket.cpp mcp-bucket.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-bucket.cpp

mcp-mesh-pthread.o: mcp-mesh.cpp mcp-mesh.hpp mcp-bucket.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-mesh.cpp

mcp-parallel-pthread.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-parallel.cpp

mcp-posix-pthread.o: mcp-posix.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-posix.cpp

mcp-pthread.o: mcp-pthread.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-pthread.cpp

$(BIN)/mcp-pthread: mcp-matrix+formula-pthread.o mcp-common-pthread.o \
		mcp-bucket-pthread.o mcp-mesh-pthread.o \
		mcp-parallel-pthread.o \
		mcp-posix-pthread.o mcp-pthread.o
	$(CXX) -pthread -o $(BIN)/mcp-pthread-$(VERSION) \
		mcp-pthread.o \
		mcp-posix-pthread.o \
		mcp-bucket-pthread.o \
		mcp-mesh-pthread.o \
		mcp-parallel-pthread.o \
		mcp-common-pthread.o \
		mcp-matrix+formula-pthread.o

#---------------------------------------------------------------------------------------------------

trans: $(BIN)/mcp-trans

$(BIN)/mcp-trans: mcp-matrix+formula-trans.o mcp-trans.o
	$(CXX) -o $(BIN)/mcp-trans-$(VERSION) mcp-trans.o mcp-matrix+formula-trans.o

mcp-trans.o: mcp-trans.cpp
	$(CXX) -c -o $@ mcp-trans.cpp

mcp-matrix+formula-trans.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

check: $(BIN)/mcp-check

mcp-matrix+formula-check.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-check.o: mcp-check.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-check.cpp

$(BIN)/mcp-check:  mcp-matrix+formula-check.o mcp-check.o
	$(CXX) -o $(BIN)/mcp-check-$(VERSION) \
		mcp-check.o \
		mcp-matrix+formula-check.o

#---------------------------------------------------------------------------------------------------

predict: $(BIN)/mcp-predict

mcp-matrix+formula-predict.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-predict.o: mcp-predict.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-predict.cpp

$(BIN)/mcp-predict: mcp-matrix+formula-predict.o mcp-predict.o
	$(CXX) -o $(BIN)/mcp-predict-$(VERSION) \
		mcp-predict.o \
		mcp-matrix+formula-predict.o

#---------------------------------------------------------------------------------------------------

scratch:
	rm -f *.o
	rm -f *~

#---------------------------------------------------------------------------------------------------
# EOF
