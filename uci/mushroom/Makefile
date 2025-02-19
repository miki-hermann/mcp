.PHONY: all horn guess csv clean scratch

all: mushroom.lrn mushroom.chk mushroom.hdr
	mcp-seq     -i mushroom.lrn  -o mushroom.out           --formula mushroom        --closure cnf
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out    --print clause
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out --print clause
	mcp-chk2tst -i mushroom.chk
	mcp-predict -i mushroom.tst  -l mushroom               -o mushroom.pdt

horn:  mushroom.lrn mushroom.chk mushroom.hdr
	mcp-pthread -i mushroom.lrn  -o mushroom.out           --formula mushroom        # --closure cnf
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out    # --print clause
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out # --print clause
	mcp-chk2tst -i mushroom.chk
	mcp-predict -i mushroom.tst  -l mushroom               -o mushroom.pdt

mushroom.lrn: mushroom.unq
	mcp-split   -i mushroom.unq  --ratio 20

mushroom.unq: mushroom.mat
	mcp-uniq    -i mushroom.mat

mushroom.mat: mushroom.data mushroom.txt
	mcp-overview -i mushroom.data -c 0
	mcp-trans    -i mushroom.data -m mushroom.txt --robust yes

guess: mushroom.data mushroom.names
	mcp-guess   -i mushroom.data -n mushroom.names -o dummy.txt

csv: mushroom.mat
	mcp-mat2csv < mushroom.mat > mushroom.csv

clean:
	rm -f *.hdr *.prc *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt *.csv
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
