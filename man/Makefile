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
##*     File:    man/Makefile                                              *
##*                                                                        *
##*      Copyright (c) 2019 - 2021                                         *
##*                                                                        *
##* Publish MCP man pages.                                                 *
##*                                                                        *
##* This software has been created within the ACCA Project.                *
##*                                                                        *
##*                                                                        *
##**************************************************************************

.PHONY: check guess seq mpi pthread hybrid split trans sparse

all: check guess seq mpi pthread hybrid split trans sparse

check: mcp-check.1
	sudo cp -f mcp-check.1 /usr/local/share/man/man1/

guess: mcp-guess.1
	sudo cp -f mcp-guess.1 /usr/local/share/man/man1/

seq: mcp-seq.1
	sudo cp -f mcp-seq.1 /usr/local/share/man/man1/

mpi: seq
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-mpi.1

pthread: seq
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-pthread.1

hybrid: seq
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-hybrid.1

split: mcp-split.1
	sudo cp -f mcp-split.1 /usr/local/share/man/man1/

trans: mcp-trans.1 mcp-trans.5
	sudo cp -f mcp-trans.1 /usr/local/share/man/man1/
	sudo cp -f mcp-trans.5 /usr/local/share/man/man5/

sparse: mcp-sparse.1
	sudo cp -f mcp-sparse.1 /usr/local/share/man/man1/
