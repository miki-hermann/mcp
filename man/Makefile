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
##*      Copyright (c) 2019 - 2025                                         *
##*                                                                        *
##* Publish MCP man pages.                                                 *
##*                                                                        *
##*                                                                        *
##**************************************************************************

MANPAGES := /usr/local/share/man
SUDO := sudo

.PHONY: check guess seq pthread split trans sparse chk2tst \
	predict uniq sample clean overview compare

all: check guess seq pthread split trans sparse chk2tst \
	predict uniq sample clean overview compare

check: mcp-check.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-check.1 $(MANPAGES)/man1/

guess: mcp-guess.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-guess.1 $(MANPAGES)/man1/

seq: mcp-seq.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-seq.1 $(MANPAGES)/man1/

pthread: seq
	$(SUDO) ln -sf $(MANPAGES)/man1/mcp-seq.1 \
		$(MANPAGES)/man1/mcp-pthread.1

split: mcp-split.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-split.1 $(MANPAGES)/man1/

trans: mcp-trans.1 mcp-trans.5
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-trans.1 $(MANPAGES)/man1/
	$(SUDO) mkdir -p $(MANPAGES)/man5/
	$(SUDO) cp -f mcp-trans.5 $(MANPAGES)/man5/

clean: mcp-clean.1 mcp-clean.5
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-clean.1 $(MANPAGES)/man1/
	$(SUDO) mkdir -p $(MANPAGES)/man5/
	$(SUDO) cp -f mcp-clean.5 $(MANPAGES)/man5/

overview: mcp-overview.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-overview.1 $(MANPAGES)/man1/

sparse: mcp-sparse.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-sparse.1 $(MANPAGES)/man1/

chk2tst: mcp-chk2tst.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-chk2tst.1 $(MANPAGES)/man1/

predict: mcp-predict.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-predict.1 $(MANPAGES)/man1/

compare: mcp-compare.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-compare.1 $(MANPAGES)/man1/

uniq: mcp-uniq.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-uniq.1 $(MANPAGES)/man1/

sample: mcp-sample.1
	$(SUDO) mkdir -p $(MANPAGES)/man1/
	$(SUDO) cp -f mcp-sample.1 $(MANPAGES)/man1/
