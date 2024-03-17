all:
	mcp-trans  -i hv84.data -m hv84.txt
	mcp-uniq   -i hv84.mat
	mcp-split  -i hv84.unq --ratio 20
	mcp-seq    -i hv84.lrn -o hv84-bij.out --closure bij --formula hv84
	mcp-check  -i hv84.chk -l hv84_democrat.log \
			--print clause -o hv84_democrat.out
	mcp-check  -i hv84.chk -l hv84_republican.log \
			--print clause -o hv84_republican.out
	mcp-chk2tst -i hv84.chk
	mcp-predict -i hv84.tst -l hv84 -o hv84.pdt

.PHONY: guess clean scratch

guess:
	mcp-guess  -i hv84.data -n hv84.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log *.tst *.pvt *.pdx *.pdt

scratch: clean
	rm -f *~
	rm -f dummy.txt
