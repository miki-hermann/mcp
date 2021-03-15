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

all: mcp-check.1 mcp-guess.1 mcp-seq.1 mcp-split.1 mcp-trans.1 mcp-trans.5
	sudo cp -f mcp-check.1 \
		mcp-guess.1 \
		mcp-seq.1 \
		mcp-split.1 \
		mcp-trans.1 \
		/usr/local/share/man/man1/
	sudo cp -f mcp-trans.5 /usr/local/share/man/man5/
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-mpi.1
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-pthread.1
	sudo ln -sf /usr/local/share/man/man1/mcp-seq.1 \
		/usr/local/share/man/man1/mcp-hybrid.1
