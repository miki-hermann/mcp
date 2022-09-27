all:
	mcp-trans -i optdigits-tra.csv  -m optdigits.txt
	mcp-uniq  -i optdigits-tra.mat
	mcp-seq   -i optdigits-tra.unq  -o optdigits-tra.out --formula optdigits --closure bij --action select 8
	mcp-trans  -i optdigits-tes.csv -m optdigits.txt
	mcp-check  -i optdigits-tes.mat -l optdigits_8.log   -o optdigits_8.out  --print clause

.PHONY: guess clean scratch

guess:
	mcp-guess   -i optdigits-tra.csv -n optdigits.names -o dummy-tra.txt
	mcp-guess   -i optdigits-tes.csv -n optdigits.names -o dummy-tes.txt

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt

scratch: clean
	rm -f *~
	rm -f dummy.txt
