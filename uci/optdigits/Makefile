all: perl uniq
	mcp-pthread -i optdigits-tra.unq  -o optdigits-tra.out --formula optdigits --direction lowcard
	for number in 0 1 2 3 4 5 6 7 8 9 ; do \
		mcp-check -i optdigits-wdep.unq   -l optdigits_$$number.log   -o optdigits_$$number.wdep.out ; \
		mcp-check -i optdigits-windep.unq -l optdigits_$$number.log   -o optdigits_$$number.windep.out ; \
		mcp-check -i optdigits-cv.unq     -l optdigits_$$number.log   -o optdigits_$$number.cv.out ; \
	done
	rm -f *.mat *.unq *.log

.PHONY: perl uniq clean scratch

perl:
	optdigits.pl optdigits-tra.orig
	optdigits.pl optdigits-wdep.orig
	optdigits.pl optdigits-windep.orig
	optdigits.pl optdigits-cv.orig

uniq:
	mcp-uniq    -i optdigits-tra.mat
	mcp-uniq    -i optdigits-wdep.mat
	mcp-uniq    -i optdigits-windep.mat
	mcp-uniq    -i optdigits-cv.mat

clean:
	rm -f *.mat *.unq *.out *.log *.tst *.pvt *.pdx *.pdt

scratch: clean
	rm -f *~
