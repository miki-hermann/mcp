##**************************************************************************
##*                                                                        *
##*                                                                        *
##*	       Multiple Characterization Problem (MCP)                     *
##*                                                                        *
##*	Author:   Miki Hermann                                             *
##*	e-mail:   hermann@lix.polytechnique.fr                             *
##*	Address:  LIX (CNRS UMR 7161), Ecole Polytechnique, France          *
##*                                                                        *
##*	Author: Gernot Salzer                                              *
##*	e-mail: gernot.salzer@tuwien.ac.at                                 *
##*	Address: Technische Universitaet Wien, Vienna, Austria             *
##*                                                                        *
##*	Version: all                                                       *
##*     File:    src/Makefile                                              *
##*                                                                        *
##*      Copyright (c) 2019 - 2020                                         *
##*                                                                        *
##* Given several  Boolean matrices  representing the  presence/absence of *
##* attributes in  observations, this software generates  Horn, dual Horn, *
##* or bijunctive formulas from positive and negative examples represented *
##* by these matrices.                                                     *
##*                                                                        *
##* This software has been created within the ACCA Project.                *
##*                                                                        *
##*                                                                        *
##**************************************************************************

OBJ = ../obj
BIN = ../bin
DIST = ../dist

all: seq mpi pthread hybrid trans guess cnf split check

#---------------------------------------------------------------------------------------------------

seq: matrix+formula-seq.o common-seq.o seq.o
	g++ -O4 -o $(BIN)/mcp-seq $(OBJ)/mcp-seq.o \
				  $(OBJ)/mcp-common-seq.o \
				  $(OBJ)/mcp-matrix+formula-seq.o
	\cp $(BIN)/mcp-seq ..

seq.o: mcp-seq.cpp mcp-common.hpp
	g++ -O4 -c -o $(OBJ)/mcp-seq.o mcp-seq.cpp

common-seq.o: mcp-common.cpp mcp-common.hpp
	g++ -O4 -c -o $(OBJ)/mcp-common-seq.o mcp-common.cpp

matrix+formula-seq.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	g++ -O4 -c -o $(OBJ)/mcp-matrix+formula-seq.o mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

mpi: matrix+formula-mpi.o common-mpi.o parallel-mpi.o mpi.o
	mpic++ -O4 -o $(BIN)/mcp-mpi $(OBJ)/mcp-mpi.o \
				     $(OBJ)/mcp-common-mpi.o \
				     $(OBJ)/mcp-matrix+formula-mpi.o \
				     $(OBJ)/mcp-parallel-mpi.o
	\cp $(BIN)/mcp-mpi ..

mpi.o: mcp-mpi.cpp mcp-common.hpp mcp-parallel.hpp
	mpic++ -O4 -c -o $(OBJ)/mcp-mpi.o mcp-mpi.cpp

parallel-mpi.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp
	mpic++ -O4 -c -o $(OBJ)/mcp-parallel-mpi.o mcp-parallel.cpp

common-mpi.o: mcp-common.cpp mcp-common.hpp
	mpic++ -O4 -c -o $(OBJ)/mcp-common-mpi.o mcp-common.cpp

matrix+formula-mpi.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	mpic++ -O4 -c -o $(OBJ)/mcp-matrix+formula-mpi.o mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

pthread: matrix+formula-pthread.o common-pthread.o parallel-pthread.o posix-pthread.o pthread.o
	g++ -pthread -O4 -o $(BIN)/mcp-pthread $(OBJ)/mcp-pthread.o \
					       $(OBJ)/mcp-common-pthread.o \
					       $(OBJ)/mcp-matrix+formula-pthread.o \
					       $(OBJ)/mcp-parallel-pthread.o \
					       $(OBJ)/mcp-posix-pthread.o
	\cp $(BIN)/mcp-pthread ..

pthread.o: mcp-pthread.cpp mcp-common.hpp mcp-parallel.hpp mcp-posix.hpp
	g++ -pthread -c -O4 -o $(OBJ)/mcp-pthread.o mcp-pthread.cpp

posix-pthread.o: mcp-posix.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp
	g++ -pthread -c -O4 -o $(OBJ)/mcp-posix-pthread.o mcp-posix.cpp

parallel-pthread.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp
	g++ -pthread -c -O4 -o $(OBJ)/mcp-parallel-pthread.o mcp-parallel.cpp

common-pthread.o: mcp-common.cpp mcp-common.hpp
	g++ -pthread -c -O4 -o $(OBJ)/mcp-common-pthread.o mcp-common.cpp

matrix+formula-pthread.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	g++ -pthread -c -O4 -o $(OBJ)/mcp-matrix+formula-pthread.o mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

hybrid: matrix+formula-hybrid.o common-hybrid.o parallel-hybrid.o posix-hybrid.o hybrid.o
	mpic++ -pthread -O4 -o $(BIN)/mcp-hybrid $(OBJ)/mcp-hybrid.o \
						 $(OBJ)/mcp-common-hybrid.o \
					         $(OBJ)/mcp-matrix+formula-hybrid.o \
						 $(OBJ)/mcp-parallel-hybrid.o \
						 $(OBJ)/mcp-posix-hybrid.o
	\cp $(BIN)/mcp-hybrid ..


hybrid.o: mcp-hybrid.cpp mcp-common.hpp mcp-parallel.hpp mcp-posix.hpp
	mpic++ -pthread -O4 -c -o $(OBJ)/mcp-hybrid.o mcp-hybrid.cpp


posix-hybrid.o: mcp-posix.cpp mcp-posix.hpp mcp-parallel.hpp mcp-common.hpp
	mpic++ -pthread -c -O4 -o $(OBJ)/mcp-posix-hybrid.o mcp-posix.cpp

parallel-hybrid.o: mcp-parallel.cpp mcp-parallel.hpp mcp-common.hpp
	mpic++ -pthread -c -O4 -o $(OBJ)/mcp-parallel-hybrid.o mcp-parallel.cpp

common-hybrid.o: mcp-common.cpp mcp-common.hpp
	mpic++ -pthread -c -O4 -o $(OBJ)/mcp-common-hybrid.o mcp-common.cpp

matrix+formula-hybrid.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	mpic++ -pthread -c -O4 -o $(OBJ)/mcp-matrix+formula-hybrid.o mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

trans: mcp-trans.cpp mcp-trans.pl
	g++ -O4 -o $(BIN)/mcp-trans mcp-trans.cpp
	\cp mcp-trans.pl $(BIN)/mcp-trans.pl
	\cp $(BIN)/mcp-trans ..
	\cp $(BIN)/mcp-trans.pl ..

#---------------------------------------------------------------------------------------------------

guess: mcp-guess.cpp mcp-guess.pl
	g++ -O4 -o $(BIN)/mcp-guess mcp-guess.cpp
	\cp mcp-guess.pl $(BIN)/mcp-guess.pl
	\cp $(BIN)/mcp-guess ..
	\cp $(BIN)/mcp-guess.pl ..

#---------------------------------------------------------------------------------------------------

cnf: mcp-cnf
	\cp mcp-cnf $(BIN)/mcp-cnf
	\cp $(BIN)/mcp-cnf ..

#---------------------------------------------------------------------------------------------------

split: mcp-split.cpp
	g++ -O4 -o $(BIN)/mcp-split mcp-split.cpp
	\cp $(BIN)/mcp-split ..

#---------------------------------------------------------------------------------------------------

check:  matrix+formula-check.o mcp-check.o
	g++ -O4 -o $(BIN)/mcp-check $(OBJ)/mcp-check.o \
				    $(OBJ)/mcp-matrix+formula-check.o
	\cp $(BIN)/mcp-check ..

mcp-check.o: mcp-check.cpp mcp-matrix+formula.hpp
	g++ -O4 -c -o $(OBJ)/mcp-check.o mcp-check.cpp

matrix+formula-check.o: mcp-matrix+formula.cpp mcp-matrix+formula.hpp
	g++ -O4 -c -o $(OBJ)/mcp-matrix+formula-check.o mcp-matrix+formula.cpp

#---------------------------------------------------------------------------------------------------

install:
	\cp $(BIN)/mcp-seq ..
	\cp $(BIN)/mcp-mpi ..
	\cp $(BIN)/mcp-pthread ..
	\cp $(BIN)/mcp-hybrid ..
	\cp $(BIN)/mcp-trans ..
	\cp $(BIN)/mcp-trans.pl ..
	\cp $(BIN)/mcp-guess ..
	\cp $(BIN)/mcp-guess.pl ..
	\cp $(BIN)/mcp-cnf ..
	\cp $(BIN)/mcp-split ..
	\cp $(BIN)/mcp-check ..

.PHONY: clean scratch

clean:
	rm -f $(OBJ)/*.o $(BIN)/mcp-*

scratch: clean
	rm -f *~

#---------------------------------------------------------------------------------------------------
# EOF
