.PHONY: all guess csv clean scratch

all: horn bij cnf

horn: rice.lrn
	mcp-pthread -i rice.lrn  -o rice_horn.out          --formula rice_horn       --closure horn
	mcp-check   -i rice.chk  -l rice_horn_cammeo.log   -o rice_horn_cammeo.out   --print clause
	mcp-check   -i rice.chk  -l rice_horn_osmancik.log -o rice_horn_osmancik.out --print clause

bij: rice.lrn
	mcp-pthread -i rice.lrn  -o rice_bij.out           --formula rice_bij        --closure bij
	mcp-check   -i rice.chk  -l rice_bij_cammeo.log    -o rice_bij_cammeo.out    --print clause
	mcp-check   -i rice.chk  -l rice_bij_osmancik.log  -o rice_bij_osmancik.out  --print clause

cnf: rice.lrn
	mcp-pthread -i rice.lrn  -o rice_cnf.out           --formula rice_cnf        --closure cnf
	mcp-check   -i rice.chk  -l rice_cnf_cammeo.log    -o rice_cnf_cammeo.out    --print clause
	mcp-check   -i rice.chk  -l rice_cnf_osmancik.log  -o rice_cnf_osmancik.out  --print clause

rice.lrn: rice.unq
	mcp-split   -i rice.unq  --ratio 20

rice.unq: rice.mat
	mcp-uniq    -i rice.mat

rice.mat: rice.data rice.txt
	mcp-overview -i rice.data -c 7
	mcp-trans    -i rice.data -m rice.txt

guess: rice.data
	mcp-guess   -i rice.data -n rice.nam -o dummy.txt

csv: rice.mat
	mcp-mat2csv < rice.mat > rice.csv

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt *.csv
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
