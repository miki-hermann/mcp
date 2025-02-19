all:
	mcp-overview   -i accent.csv -c 0
	mcp-trans      -i accent.csv -m accent.txt
	mcp-uniq       -i accent.mat
	mcp-split      -i accent.unq  --ratio 25
	mcp-pthread    -i accent.lrn  -o accent.out     --formula accent
	for LANG in ES FR GE IT UK US ; do \
		mcp-check      -i accent.chk -l accent_$$LANG.log  -o accent_$$LANG.out ; \
	done

.PHONY: guess clean scratch

guess:
	mcp-guess      -i accent.csv -n accent.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.prc *.unq *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
