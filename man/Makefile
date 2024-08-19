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
##*      Copyright (c) 2019 - 2024                                         *
##*                                                                        *
##* Publish MCP man pages.                                                 *
##*                                                                        *
##* This software has been created within the ACCA Project.                *
##*                                                                        *
##*                                                                        *
##**************************************************************************

.PHONY: check guess seq mpi pthread hybrid split trans sparse chk2tst \
	predict uniq sample clean over

all: check guess seq mpi pthread hybrid split trans sparse chk2tst \
	predict uniq sample clean over

check: mcp-check.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-check.1 /usr/local/share/man/man1/

guess: mcp-guess.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-guess.1 /usr/local/share/man/man1/

seq: mcp-seq.1
	sudo mkdir -p /usr/local/share/man/man1/
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
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-split.1 /usr/local/share/man/man1/

trans: mcp-trans.1 mcp-trans.5
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-trans.1 /usr/local/share/man/man1/
	sudo mkdir -p /usr/local/share/man/man5/
	sudo cp -f mcp-trans.5 /usr/local/share/man/man5/

clean: mcp-clean.1 # mcp-clean.5
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-clean.1 /usr/local/share/man/man1/
	# sudo mkdir -p /usr/local/share/man/man5/
	# sudo cp -f mcp-clean.5 /usr/local/share/man/man5/

over: mcp-overview.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-overview.1 /usr/local/share/man/man1/

sparse: mcp-sparse.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-sparse.1 /usr/local/share/man/man1/

chk2tst: mcp-chk2tst.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-chk2tst.1 /usr/local/share/man/man1/

predict: mcp-predict.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-predict.1 /usr/local/share/man/man1/

uniq: mcp-uniq.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-uniq.1 /usr/local/share/man/man1/

sample: mcp-sample.1
	sudo mkdir -p /usr/local/share/man/man1/
	sudo cp -f mcp-sample.1 /usr/local/share/man/man1/
