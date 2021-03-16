all:
	mcp-trans   -i mushroom.data -m mushroom.txt
	mcp-split   -i mushroom.mat  --ratio 20
	mcp-seq     -i mushroom.lrn  -o mushroom.out           --formula mushroom --closure cnf
	mcp-check   -i mushroom.chk  -l mushroom_edible.log    -o mushroom_edible.out
	mcp-check   -i mushroom.chk  -l mushroom_poisonous.log -o mushroom_poisonous.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~

