.PHONY: all clean scratch

all: bij horn cnf perl

ba.mat: ba.data ba.txt
	mcp-overview -i ba.data -c 4
	mcp-trans    -i ba.data -m ba.txt

ba.lrn: ba.mat
	mcp-uniq    -i ba.mat
	mcp-split   -i ba.unq  --ratio 10
	mcp-chk2tst -i ba.chk

bij: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-bij.out          -l ba-bij          --closure bij
	mcp-check   -i ba.chk  -l ba-bij_genuine.log  -o ba-bij_genuine.out
	mcp-check   -i ba.chk  -l ba-bij_forged.log   -o ba-bij_forged.out
	mcp-predict -i ba.tst  -l ba-bij              -o ba-bij.pdt --pdx ba-bij.pdx

cnf: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-cnf.out          -l ba-cnf          --closure cnf
	mcp-check   -i ba.chk  -l ba-cnf_genuine.log  -o ba-cnf_genuine.out
	mcp-check   -i ba.chk  -l ba-cnf_forged.log   -o ba-cnf_forged.out
	mcp-predict -i ba.tst  -l ba-cnf              -o ba-cnf.pdt --pdx ba-cnf.pdx

horn: ba.lrn
	mcp-pthread -i ba.lrn  -o ba-hrn.out          -l ba-hrn
	mcp-check   -i ba.chk  -l ba-hrn_genuine.log  -o ba-hrn_genuine.out
	mcp-check   -i ba.chk  -l ba-hrn_forged.log   -o ba-hrn_forged.out
	mcp-predict -i ba.tst  -l ba-hrn              -o ba-hrn.pdt --pdx ba-hrn.pdx

perl: ba.data
	ba.pl < ba.data > ba.csv

csv: ba.mat
	mcp-mat2csv < ba.mat > ba.csv

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f *_clean.*
