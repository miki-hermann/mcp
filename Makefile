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

.PHONY: no-mpi compile compile-no-mpi man install clean scratch github ex

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

github:
	mkdir -p ~/github/mcp/
	mkdir -p ~/github/mcp/src
	cp -f src/*.cpp         ~/github/mcp/src
	cp -f src/*.hpp         ~/github/mcp/src
	cp -f src/mcp-cnf       ~/github/mcp/src
	cp -f src/Makefile      ~/github/mcp/src
	cp -f src/changelog.txt ~/github/mcp/src
	mkdir -p ~/github/mcp/man
	cp -f man/Makefile      ~/github/mcp/man
	cp -f man/mcp-check.1   ~/github/mcp/man
	cp -f man/mcp-chk2tst.1 ~/github/mcp/man
	cp -f man/mcp-guess.1   ~/github/mcp/man
	cp -f man/mcp-predict.1 ~/github/mcp/man
	cp -f man/mcp-seq.1     ~/github/mcp/man
	cp -f man/mcp-split.1   ~/github/mcp/man
	cp -f man/mcp-trans.1   ~/github/mcp/man
	cp -f man/mcp-trans.5   ~/github/mcp/man
	cp -f man/mcp-uniq.1    ~/github/mcp/man
	cp -f Makefile          ~/github/mcp/
	mkdir -p ~/github/mcp/paper
	cp -f ../papers/github/mcp.pdf ~/github/mcp/paper/

clean:
	rm -f bin/mcp-*
	rm -f src/*.o

ex:
	rm -f mcp-*-ex
	sudo rm -f /usr/local/bin/mcp-*-ex

scratch: clean ex
	rm -f *~
