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
##*      Copyright (c) 2019 - 2025                                         *
##*                                                                        *
##* Given several (Boolean) matrices  representing the presence/absence of *
##* attributes in  observations, this software generates  Horn, dual Horn, *
##* or 2SAT  formulas from  positive and  negative samples  represented by *
##* these matrices.                                                        *
##*                                                                        *
##*                                                                        *
##**************************************************************************

BIN = ../bin
GXX_VERSION := -std=c++23
# GXX_VERSION :=
MINIMAL_FLAGS := -O4
VANILLA_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -Wfatal-errors
RELEASE_FLAGS := $(GXX_VERSION) -O4 -march=native -flto -fopenmp -Wall -Werror -Wfatal-errors
DEBUG_FLAGS := $(GXX_VERSION) -g -Og -fopenmp -Wall -Werror -Wfatal-errors -fsanitize=undefined -fsanitize=address
GXX   := g++
CXX   := $(GXX) $(VANILLA_FLAGS)
# CXX   := $(GXX) $(RELEASE_FLAGS)
# CXX   := $(GXX) $(DEBUG_FLAGS)

.PHONY: compile guess split sample chk2tst uniq overview compare \
	csv cnf \
	clean scratch

compile: guess overview clean sample uniq split compare chk2tst cnf csv

#---------------------------------------------------------------------------------------------------

guess: $(BIN)/mcp-guess

mcp-basics-guess.o: mcp-defs.hpp mcp-basics.cpp mcp-basics.hpp
	$(GXX) $(MINIMAL_FLAGS) -c -o $@ mcp-basics.cpp

mcp-guess.o: mcp-guess.cpp mcp-basics.hpp mcp-defs.hpp
	$(GXX) $(MINIMAL_FLAGS) -c -o $@ mcp-guess.cpp

$(BIN)/mcp-guess: mcp-defs.hpp mcp-basics-guess.o mcp-guess.o
	$(GXX) $(MINIMAL_FLAGS) -o $(BIN)/mcp-guess \
		mcp-guess.o \
		mcp-basics-guess.o

#---------------------------------------------------------------------------------------------------

clean: $(BIN)/mcp-clean

$(BIN)/mcp-clean: mcp-clean
	cp -f mcp-clean $(BIN)/mcp-clean

#---------------------------------------------------------------------------------------------------

split: $(BIN)/mcp-split

mcp-split.o: mcp-split.cpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-split.cpp

$(BIN)/mcp-split: mcp-split.o
	$(CXX) -o $(BIN)/mcp-split \
		mcp-split.o

#---------------------------------------------------------------------------------------------------

sample: $(BIN)/mcp-sample

mcp-basics-sample.o: mcp-basics.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-basics.cpp

mcp-tally-sample.o: mcp-tally.cpp mcp-tally.hpp mcp-basics.hpp
	$(CXX) -c -o $@ mcp-tally.cpp

mcp-sample.o: mcp-sample.cpp mcp-basics.hpp
	$(CXX) -c -o $@ mcp-sample.cpp

$(BIN)/mcp-sample: mcp-basics-sample.o mcp-tally-sample.o mcp-sample.o mcp-defs.hpp
	$(CXX) -o $(BIN)/mcp-sample \
		mcp-sample.o \
		mcp-tally-sample.o \
		mcp-basics-sample.o

#---------------------------------------------------------------------------------------------------

overview: $(BIN)/mcp-overview

mcp-basics-overview.o: mcp-basics.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-basics.cpp

mcp-tally-overview.o: mcp-tally.cpp mcp-tally.hpp mcp-basics.hpp
	$(CXX) -c -o $@ mcp-tally.cpp

mcp-overview.o: mcp-overview.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-overview.cpp

$(BIN)/mcp-overview: mcp-basics-overview.o mcp-tally-overview.o mcp-overview.o
	$(CXX) -o $(BIN)/mcp-overview \
		mcp-overview.o \
		mcp-tally-overview.o \
		mcp-basics-overview.o

#---------------------------------------------------------------------------------------------------

compare: $(BIN)/mcp-compare

mcp-basics-compare.o: mcp-basics.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-basics.cpp

mcp-compare.o: mcp-compare.cpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-compare.cpp

$(BIN)/mcp-compare: mcp-basics-compare.o mcp-compare.o
	$(CXX) -o $(BIN)/mcp-compare \
		mcp-compare.o \
		mcp-basics-compare.o

#---------------------------------------------------------------------------------------------------

chk2tst: $(BIN)/mcp-chk2tst

mcp-basics-chk2tst.o: mcp-basics.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-basics.cpp

mcp-chk2tst.o: mcp-chk2tst.cpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-chk2tst.cpp

$(BIN)/mcp-chk2tst: mcp-basics-chk2tst.o mcp-chk2tst.o
	$(CXX) -o $(BIN)/mcp-chk2tst \
		mcp-chk2tst.o \
		mcp-basics-chk2tst.o

#---------------------------------------------------------------------------------------------------

uniq: $(BIN)/mcp-uniq

mcp-basics-uniq.o: mcp-basics.cpp mcp-basics.hpp mcp-defs.hpp
	$(CXX) -c -o $@ mcp-basics.cpp

mcp-uniq.o: mcp-uniq.cpp mcp-basics.hpp
	$(CXX) -c -o $@ mcp-uniq.cpp

$(BIN)/mcp-uniq: mcp-basics-uniq.o mcp-uniq.o
	$(CXX) -o $(BIN)/mcp-uniq \
		mcp-uniq.o \
		mcp-basics-uniq.o

#---------------------------------------------------------------------------------------------------

cnf: $(BIN)/mcp-cnf

$(BIN)/mcp-cnf: mcp-cnf
	cp -f mcp-cnf $(BIN)/mcp-cnf

#---------------------------------------------------------------------------------------------------

csv: $(BIN)/mcp-csv

$(BIN)/mcp-csv: mcp-mat2csv
	cp -f mcp-mat2csv $(BIN)/mcp-mat2csv

#---------------------------------------------------------------------------------------------------

scratch:
	rm -f *.o
	rm -f *~

#---------------------------------------------------------------------------------------------------
# EOF
