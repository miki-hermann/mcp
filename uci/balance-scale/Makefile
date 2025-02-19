all:
	mcp-overview -i bsc.data -c 0
	mcp-trans    -m bsc.txt -i bsc.data
	mcp-split    -i bsc.mat --ratio 5
	mcp-pthread  -i bsc.lrn -o bsc.out -l bsc
	mcp-check    -i bsc.chk -l bsc_B.log -o bsc_B.out
	mcp-check    -i bsc.chk -l bsc_L.log -o bsc_L.out
	mcp-check    -i bsc.chk -l bsc_R.log -o bsc_R.out

.PHONY: guess clean scratch

guess:
	mcp-guess -i bsc.data -n bsc.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.prc *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
