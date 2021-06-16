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

.PHONY: no-mpi compile compile-no-mpi man install clean scratch github

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

github:
	cp -f src/*.cpp src/*.hpp ~/github/mcp/src
	cp -f src/mcp-cnf ~/github/mcp/src
	cp -f src/Makefile src/changelog.txt ~/github/mcp/src
	cp -f man/Makefile ~/github/mcp/man
	cp -f man/mcp-check.1 man/mcp-guess.1 man/mcp-split.1 ~/github/mcp/man
	cp -f man/mcp-trans.1 man/mcp-trans.5 ~/github/mcp/man
	cp -f man/mcp-seq.1 ~/github/mcp/man
	cp -f Makefile ~/github/mcp/
	cp -f ../papers/2020_report/mcp-sat.pdf ~/github/mcp/paper/

clean:
	rm -f bin/mcp-*
	rm -f src/*.o

scratch: clean
	rm -f *~
