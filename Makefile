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
##*     File:    man/Makefile                                              *
##*                                                                        *
##*      Copyright (c) 2019 - 2021                                         *
##*                                                                        *
##* Compile and install the MCP system.                                    *
##*                                                                        *
##* This software has been created within the ACCA Project.                *
##*                                                                        *
##*                                                                        *
##**************************************************************************

.PHONY: no-mpi compile compile-no-mpi man install clean scratch

all: compile man install

no-mpi: compile-no-mpi man install

compile:
	$(MAKE) -C src
	$(MAKE) -C src export
	$(MAKE) -C src clean

compile-no-mpi:
	$(MAKE) -C src compile-no-mpi
	$(MAKE) -C src export
	$(MAKE) -C src clean

man:
	$(MAKE) -C man

install: 
	sudo cp -f mcp-* /usr/local/bin/

clean:
	rm -f $(OBJ)/*.o $(BIN)/mcp-*

scratch: clean
	rm -f *~
