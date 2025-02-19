all:
	mcp-overview -i abalone.data -c 8
	mcp-trans    -i abalone.data -m abalone.txt
	mcp-uniq     -i abalone.mat
	mcp-split    -i abalone.unq  --ratio 25
	mcp-pthread  -i abalone.lrn -o abalone.out     --formula abalone
	mcp-pthread  -i abalone.lrn -o abalone-bij.out --formula abalone-bij \
				--closure bij
	mcp-check    -i abalone.chk -l abalone_24.log     -o abalone_24.out
	mcp-check    -i abalone.chk -l abalone-bij_25.log -o abalone-bij_25.out

.PHONY: guess clean scratch

guess:
	mcp-guess      -i abalone.data -n abalone.nam -o dummy.txt

clean:
	rm -f *.mat *.hdr *.prc *.unq *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
