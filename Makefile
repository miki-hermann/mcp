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
##*     File:    solver/Makefile                                           *
##*                                                                        *
##*      Copyright (c) 2019 - 2023                                         *
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
	sudo mkdir -p /usr/local/bin
	sudo cp -f mcp-* /usr/local/bin/

clean:
	rm -f bin/mcp-*
	rm -f src/*.o

scratch: clean ex
	rm -f *~
