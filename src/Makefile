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
##*     File:    src-danube/Makefile                                       *
##*                                                                        *
##*      Copyright (c) 2019 - 2024                                         *
##*                                                                        *
##* Given several  Boolean matrices  representing the  presence/absence of *
##* attributes in  observations, this software generates  Horn, dual Horn, *
##* or bijunctive formulas from positive and negative examples represented *
##* by these matrices.                                                     *
##*                                                                        *
##*                                                                        *
##**************************************************************************

BIN = ../bin
CXX_VERSION := -std=c++20
# CXX_VERSION :=
MINIMAL_FLAGS := -O4
VANILLA_FLAGS := $(CXX_VERSION) -O4 -march=native -flto -fopenmp -Wfatal-errors
# VANILLA_FLAGS := -O4 -march=native -flto -fopenmp
RELEASE_FLAGS := $(CXX_VERSION) -O4 -march=native -flto -fopenmp -Wall -Werror -Wfatal-errors
DEBUG_FLAGS := $(CXX_VERSION) -g -Og -fopenmp -Wall -Werror -Wfatal-errors -fsanitize=undefined -fsanitize=address
GXX   := g++
CXX   := $(GXX) $(VANILLA_FLAGS)
# CXX   := g++ $(RELEASE_FLAGS)
# CXX   := g++ $(DEBUG_FLAGS)

.PHONY: compile compile-main compile-no-mpi seq mpi pthread hybrid trans guess cnf csv split sample check sparse predict chk2tst uniq overview compare export clean scratch

compile: seq mpi pthread hybrid \
	trans guess cnf csv split sample check sparse predict compare chk2tst uniq overview clean

compile-main: seq pthread trans check predict

compile-no-mpi: seq pthread \
	trans guess cnf csv split sample check sparse predict compare chk2tst uniq overview clean

#---------------------------------------------------------------------------------------------------

seq: $(BIN)/mcp-seq

mcp-matrix+formula-seq.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-common-seq.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-common.cpp

mcp-seq.o: mcp-seq.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-seq.cpp

$(BIN)/mcp-seq: mcp-matrix+formula-seq.o mcp-common-seq.o mcp-seq.o
	$(CXX) -o $(BIN)/mcp-seq \
		mcp-seq.o \
		mcp-common-seq.o \
		mcp-matrix+formula-seq.o

#---------------------------------------------------------------------------------------------------

mpi: $(BIN)/mcp-mpi

mcp-matrix+formula-mpi.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -c -o $@ mcp-matrix+formula.cpp

mcp-common-mpi.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -c -o $@ mcp-common.cpp

mcp-parallel-mpi.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -c -o $@ mcp-parallel.cpp

mcp-mpi.o: mcp-mpi.cpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -c -o $@ mcp-mpi.cpp

$(BIN)/mcp-mpi: mcp-matrix+formula-mpi.o mcp-common-mpi.o mcp-parallel-mpi.o mcp-mpi.o
	mpic++ $(VANILLA_FLAGS) -o $(BIN)/mcp-mpi \
		mcp-mpi.o \
		mcp-parallel-mpi.o \
		mcp-common-mpi.o \
		mcp-matrix+formula-mpi.o

#---------------------------------------------------------------------------------------------------

pthread: $(BIN)/mcp-pthread

mcp-matrix+formula-pthread.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-matrix+formula.cpp

mcp-common-pthread.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-common.cpp

mcp-parallel-pthread.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-parallel.cpp

mcp-posix-pthread.o: mcp-posix.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-posix.cpp

mcp-pthread.o: mcp-pthread.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	$(CXX) -pthread -c -o $@ mcp-pthread.cpp

$(BIN)/mcp-pthread: mcp-matrix+formula-pthread.o mcp-common-pthread.o mcp-parallel-pthread.o \
		mcp-posix-pthread.o mcp-pthread.o
	$(CXX) -pthread -o $(BIN)/mcp-pthread \
		mcp-pthread.o \
		mcp-posix-pthread.o \
		mcp-parallel-pthread.o \
		mcp-common-pthread.o \
		mcp-matrix+formula-pthread.o

#---------------------------------------------------------------------------------------------------

hybrid: $(BIN)/mcp-hybrid

mcp-matrix+formula-hybrid.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -pthread -c -o $@ mcp-matrix+formula.cpp

mcp-common-hybrid.o: mcp-common.cpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -pthread -c -o $@ mcp-common.cpp

mcp-parallel-hybrid.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -pthread -c -o $@ mcp-parallel.cpp

mcp-posix-hybrid.o: mcp-posix.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -pthread -c -o $@ mcp-posix.cpp

mcp-hybrid.o: mcp-hybrid.cpp mcp-common.hpp mcp-parallel.hpp mcp-posix.hpp mcp-matrix+formula.hpp
	mpic++ $(VANILLA_FLAGS) -pthread -c -o $@ mcp-hybrid.cpp

$(BIN)/mcp-hybrid: mcp-matrix+formula-hybrid.o mcp-common-hybrid.o mcp-parallel-hybrid.o \
		mcp-posix-hybrid.o mcp-hybrid.o
	mpic++ $(VANILLA_FLAGS) -pthread -o $(BIN)/mcp-hybrid \
		mcp-hybrid.o \
		mcp-posix-hybrid.o \
		mcp-parallel-hybrid.o \
		mcp-common-hybrid.o \
		mcp-matrix+formula-hybrid.o

#---------------------------------------------------------------------------------------------------

trans: $(BIN)/mcp-trans

$(BIN)/mcp-trans: mcp-matrix+formula-trans.o mcp-trans.o
	$(CXX) -o $(BIN)/mcp-trans mcp-trans.o mcp-matrix+formula-trans.o

mcp-trans.o: mcp-trans.cpp
	$(CXX) -c -o $@ mcp-trans.cpp

mcp-matrix+formula-trans.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

guess: $(BIN)/mcp-guess

$(BIN)/mcp-guess: mcp-matrix+formula-guess.o mcp-guess.o
	$(GXX) $(MINIMAL_FLAGS) -o $(BIN)/mcp-guess mcp-guess.o mcp-matrix+formula-guess.o

mcp-guess.o: mcp-guess.cpp
	$(GXX) $(MINIMAL_FLAGS) -c -o $@ mcp-guess.cpp

mcp-matrix+formula-guess.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(GXX) $(MINIMAL_FLAGS) -c -o $@ mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

cnf: $(BIN)/mcp-cnf

$(BIN)/mcp-cnf: mcp-cnf
	cp -f mcp-cnf $(BIN)/mcp-cnf

#---------------------------------------------------------------------------------------------------

clean: $(BIN)/mcp-clean

$(BIN)/mcp-clean: mcp-clean
	cp -f mcp-clean $(BIN)/mcp-clean

#---------------------------------------------------------------------------------------------------

csv: $(BIN)/mcp-csv

$(BIN)/mcp-csv: mcp-mat2csv
	cp -f mcp-mat2csv $(BIN)/mcp-mat2csv

#---------------------------------------------------------------------------------------------------

split: $(BIN)/mcp-split

mcp-matrix+formula-split.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-split.o: mcp-split.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-split.cpp

$(BIN)/mcp-split: mcp-split.o  mcp-matrix+formula-split.o
	$(CXX) -o $(BIN)/mcp-split \
		mcp-split.o \
		mcp-matrix+formula-split.o

#---------------------------------------------------------------------------------------------------

sample: $(BIN)/mcp-sample

mcp-matrix+formula-sample.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-tally-sample.o: mcp-tally.cpp mcp-tally.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-tally.cpp

mcp-sample.o: mcp-sample.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-sample.cpp

$(BIN)/mcp-sample: mcp-matrix+formula-sample.o mcp-tally-sample.o mcp-sample.o
	$(CXX) -o $(BIN)/mcp-sample \
		mcp-sample.o \
		mcp-tally-sample.o \
		mcp-matrix+formula-sample.o

#---------------------------------------------------------------------------------------------------

overview: $(BIN)/mcp-overview

mcp-matrix+formula-overview.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-tally-overview.o: mcp-tally.cpp mcp-tally.hpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-tally.cpp

mcp-overview.o: mcp-overview.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-overview.cpp

$(BIN)/mcp-overview: mcp-matrix+formula-overview.o mcp-tally-overview.o mcp-overview.o
	$(CXX) -o $(BIN)/mcp-overview \
		mcp-overview.o \
		mcp-tally-overview.o \
		mcp-matrix+formula-overview.o

#---------------------------------------------------------------------------------------------------

check: $(BIN)/mcp-check

mcp-matrix+formula-check.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-check.o: mcp-check.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o mcp-check.o mcp-check.cpp

$(BIN)/mcp-check:  mcp-matrix+formula-check.o mcp-check.o
	$(CXX) -o $(BIN)/mcp-check \
		mcp-check.o \
		mcp-matrix+formula-check.o

#---------------------------------------------------------------------------------------------------

sparse: $(BIN)/mcp-sparse

mcp-matrix+formula-sparse.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-sparse.o: mcp-sparse.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-sparse.cpp

$(BIN)/mcp-sparse: mcp-matrix+formula-sparse.o mcp-sparse.o
	$(CXX) -o $(BIN)/mcp-sparse \
		mcp-sparse.o \
		mcp-matrix+formula-sparse.o

#---------------------------------------------------------------------------------------------------

predict: $(BIN)/mcp-predict

mcp-matrix+formula-predict.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-predict.o: mcp-predict.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-predict.cpp

$(BIN)/mcp-predict: mcp-matrix+formula-predict.o mcp-predict.o
	$(CXX) -o $(BIN)/mcp-predict \
		mcp-predict.o \
		mcp-matrix+formula-predict.o

#---------------------------------------------------------------------------------------------------

compare: $(BIN)/mcp-compare

mcp-matrix+formula-compare.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-compare.o: mcp-compare.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-compare.cpp

$(BIN)/mcp-compare: mcp-matrix+formula-compare.o mcp-compare.o
	$(CXX) -o $(BIN)/mcp-compare \
		mcp-compare.o \
		mcp-matrix+formula-compare.o

#---------------------------------------------------------------------------------------------------

chk2tst: $(BIN)/mcp-chk2tst

mcp-matrix+formula-chk2tst.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-chk2tst.o: mcp-chk2tst.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-chk2tst.cpp

$(BIN)/mcp-chk2tst: mcp-matrix+formula-chk2tst.o mcp-chk2tst.o
	$(CXX) -o $(BIN)/mcp-chk2tst \
		mcp-chk2tst.o \
		mcp-matrix+formula-chk2tst.o

#---------------------------------------------------------------------------------------------------

uniq: $(BIN)/mcp-uniq

mcp-matrix+formula-uniq.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-matrix+formula.cpp

mcp-uniq.o: mcp-uniq.cpp mcp-matrix+formula.hpp
	$(CXX) -c -o $@ mcp-uniq.cpp

$(BIN)/mcp-uniq: mcp-matrix+formula-uniq.o mcp-uniq.o
	$(CXX) -o $(BIN)/mcp-uniq \
		mcp-uniq.o \
		mcp-matrix+formula-uniq.o

#---------------------------------------------------------------------------------------------------

export:
	cp -f $(BIN)/mcp-* ..

scratch:
	rm -f *.o
	rm -f $(BIN)/mcp-*
	rm -f *~

#---------------------------------------------------------------------------------------------------
# EOF
