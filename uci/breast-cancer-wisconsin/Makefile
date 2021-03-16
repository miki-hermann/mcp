all:
	mcp-trans -i bcw.data -m bcw.txt
	mcp-split -i bcw.mat  --ratio 49
	mcp-seq   -i bcw.lrn -o bcw-bij.out --closure bij --formula bcw-bij
	mcp-check -i bcw.chk -l bcw-bij_benign.log \
			--print clause -o bcw-bij_benign.out
	mcp-check -i bcw.chk -l bcw-bij_malignant.log \
			--print clause -o bcw-bij_malignant.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
