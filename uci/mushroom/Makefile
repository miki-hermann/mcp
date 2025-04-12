.PHONY: all horn dhorn cnf guess csv clean scratch

all: horn

horn:  mushroom.lrn mushroom.chk mushroom.hdr
	mcp-pthread -i mushroom.lrn  -o mushroom.out           --formula mushroom --setcover yes
#	mcp-seq     -i mushroom.lrn  -o mushroom.out           --formula mushroom --setcover yes
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out
	mcp-chk2tst -i mushroom.mat
	mcp-predict -i mushroom.tst  -l mushroom               -o mushroom.pdt
	mcp-compare --original mushroom.mat

dhorn:  mushroom.lrn mushroom.chk mushroom.hdr
	mcp-pthread -i mushroom.lrn  -o mushroom.out           --formula mushroom        --closure dhorn
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out
	mcp-chk2tst -i mushroom.mat
	mcp-predict -i mushroom.tst  -l mushroom               -o mushroom.pdt
	mcp-compare --original mushroom.mat

cnf: mushroom.lrn mushroom.chk mushroom.hdr
	mcp-pthread -i mushroom.lrn  -o mushroom.out           --formula mushroom        --closure cnf
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out    --print clause
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out --print clause
	mcp-chk2tst -i mushroom.mat
	mcp-predict -i mushroom.tst  -l mushroom               -o mushroom.pdt
	mcp-compare --original mushroom.mat

mushroom.lrn: mushroom.unq
	mcp-split   -i mushroom.unq  --ratio 20

mushroom.unq: mushroom.mat
	mcp-uniq    -i mushroom.mat

mushroom.mat: mushroom.data mushroom.txt
	mcp-overview -i mushroom.data -c 0
	mcp-trans    -i mushroom.data -m mushroom.txt

guess: mushroom.data mushroom.names
	mcp-guess   -i mushroom.data -n mushroom.names -o dummy.txt

csv: mushroom.mat
	mcp-mat2csv < mushroom.mat > mushroom.csv

clean:
	rm -f *.hdr *.prc *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt *.csv
	rm -f *-overview.* *-compare.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
