all: adult+stretch adult-stretch yellow-small  yellow-small+adult-stretch

adult+stretch:
	mcp-trans -m balloons.txt -i adult+stretch.data -o adult+stretch.mat
	mcp-seq                   -i adult+stretch.mat  -o adult+stretch.out

adult-stretch:
	mcp-trans -m balloons.txt -i adult-stretch.data -o adult-stretch.mat
	mcp-seq                   -i adult-stretch.mat  -o adult-stretch.out

yellow-small:
	mcp-trans -m balloons.txt -i yellow-small.data -o yellow-small.mat
	mcp-seq                   -i yellow-small.mat  -o yellow-small.out --print clause

yellow-small+adult-stretch:
	mcp-trans -m balloons.txt -i y-s+a-s.data -o y-s+a-s.mat
	mcp-seq                   -i y-s+a-s.mat  -o y-s+a-s.out

.PHONY: clean scratch

clean:
	rm -f *.mat *.out *.lrn *.chk *.log

scratch: clean
	rm -f *~
