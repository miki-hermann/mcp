all:
	mcp-overview -i bcw.data -c 10
	mcp-trans -i bcw.data -m bcw.txt
	mcp-uniq  -i bcw.mat
	mcp-split -i bcw.unq  --ratio 49
	mcp-seq   -i bcw.lrn -o bcw-bij.out --closure bij --formula bcw-bij
	mcp-check -i bcw.chk -l bcw-bij_benign.log \
			--print clause -o bcw-bij_benign.out
	mcp-check -i bcw.chk -l bcw-bij_malignant.log \
			--print clause -o bcw-bij_malignant.out

.PHONY: guess clean scratch

guess:
	mcp-guess -i bcw.data -n bcw.names -o dummy.txt

clean:
	rm -f *.mat *.hdr *.unq *.out *.lrn *.chk *.log
	rm -f *-overview.*

scratch: clean
	rm -f *~
	rm -f dummy.txt
