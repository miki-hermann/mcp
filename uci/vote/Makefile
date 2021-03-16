all:
	mcp-trans  -i hv84.data -m hv84.txt
	mcp-split  -i hv84.mat --ratio 20
	mcp-seq    -i hv84.lrn -o hv84-bij.out --closure bij --formula hv84
	mcp-check  -i hv84.chk -l hv84_democrat.log \
			--print clause -o hv84_democrat.out
	mcp-check  -i hv84.chk -l hv84_republican.log \
			--print clause -o hv84_republican.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~

