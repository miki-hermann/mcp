all:
	mcp-trans   -i mushroom.data -m mushroom.txt
	mcp-uniq    -i mushroom.mat
	mcp-split   -i mushroom.unq  --ratio 20
	mcp-seq     -i mushroom.lrn  -o mushroom.out           --formula mushroom --closure cnf
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out
	mcp-chk2tst -i mushroom.chk
	mcp-predict -i mushroom.tst -l mushroom -o mushroom.pdt

.PHONY: guess clean scratch

guess:
	mcp-guess   -i mushroom.data -n mushroom.names -o dummy.txt

clean:
	rm -f *.mat *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt

scratch: clean
	rm -f *~
	rm -f dummy.txt
