.PHONY: all hor 2sat perl csv clean scratch

all: horn 2sat cnf # perl

ba.lrn: ba.data ba.txt
	mcp-overview -i ba.data -c 4
	mcp-trans    -i ba.data -m ba.txt
	mcp-uniq     -i ba.mat
	mcp-split    -i ba.unq  --ratio 10
	mcp-chk2tst  -i ba.mat

2sat: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-2sat.out          -l ba-2sat          --closure 2sat
	mcp-check   -i ba.chk  -l ba-2sat_genuine.log  -o ba-2sat_genuine.out
	mcp-check   -i ba.chk  -l ba-2sat_forged.log   -o ba-2sat_forged.out
	mcp-predict -i ba.tst  -l ba-2sat              -o ba-2sat.pdt --pdx ba-2sat.pdx
	mcp-compare --original ba.mat --pdx ba-2sat.pdx -o ba-2sat-compare.txt

cnf: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-cnf.out          -l ba-cnf          --closure cnf
	mcp-check   -i ba.chk  -l ba-cnf_genuine.log  -o ba-cnf_genuine.out
	mcp-check   -i ba.chk  -l ba-cnf_forged.log   -o ba-cnf_forged.out
	mcp-predict -i ba.tst  -l ba-cnf              -o ba-cnf.pdt --pdx ba-cnf.pdx
	mcp-compare --original ba.mat --pdx ba-cnf.pdx -o ba-cnf-compare.txt

horn: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-hrn.out          -l ba-hrn
	mcp-check   -i ba.chk  -l ba-hrn_genuine.log  -o ba-hrn_genuine.out
	mcp-check   -i ba.chk  -l ba-hrn_forged.log   -o ba-hrn_forged.out
	mcp-predict -i ba.tst  -l ba-hrn              -o ba-hrn.pdt --pdx ba-hrn.pdx
	mcp-compare --original ba.mat --pdx ba-hrn.pdx -o ba-hrn-compare.txt

perl: ba.data
	ba.pl < ba.data > ba.csv

csv: ba.mat
	mcp-mat2csv < ba.mat > ba.csv

clean:
	rm -f *.mat *.hdr *.prc *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt
	rm -f *-overview.* *-compare.*
	rm -f ba.csv

scratch: clean
	rm -f *~
	rm -f *_clean.*
